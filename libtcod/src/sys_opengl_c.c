/*
* libtcod 1.5.2
* Copyright (c) 2008,2009,2010,2012 Jice & Mingos
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of Jice or Mingos may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY JICE AND MINGOS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL JICE OR MINGOS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * This renderer is mostly copied and pasted from Antagonist's SkyFire GLSL roguelike engine
 */ 

#include "libtcod.h"
#include "libtcod_int.h"
#ifndef NO_OPENGL
#include <SDL.h>
#include <SDL_opengl.h>

#define CHECKGL( GLcall )                               		\
    GLcall;                                             		\
    if(!_CheckGL_Error( #GLcall, __FILE__, __LINE__))     		\
    return false;

#ifdef NDEBUG
#define DBGCHECKGL(GLcall) GLcall
#else
#define DBGCHECKGL CHECKGL
#endif

typedef  enum 
{
	Character,
	ForeCol,
	BackCol,
	ConsoleDataEnumSize
} ConsoleDataEnum;
/* JBR04152012 - Made Character a 4 byte value here to support extended characters like other renderers.
   Seems like it should be possible to make it a two byte value using GL_UNSIGNED_SHORT_5_6_5_REV in updateTex, 
   but I can't seem to get the math right in the shader code, it always loses precision somewhere,
   resulting in incorrect characters. */
const int ConsoleDataAlignment[3] = {4, 3, 3 };

static const char *TCOD_con_vertex_shader =
#ifndef NDEBUG
"#version 110\n"
#endif
"uniform vec2 termsize; "

"void main(void) "
"{ "

"   gl_Position = gl_Vertex; "

"   gl_TexCoord[0] = gl_MultiTexCoord0; "
"   gl_TexCoord[0].x = gl_TexCoord[0].x*termsize.x; "
"   gl_TexCoord[0].y = gl_TexCoord[0].y*termsize.y; "
"} "
;

static const char *TCOD_con_pixel_shader =
#ifndef NDEBUG
"#version 110\n"
#endif
"uniform sampler2D font; "
"uniform sampler2D term; "
"uniform sampler2D termfcol; "
"uniform sampler2D termbcol; "

"uniform float fontw; "
"uniform vec2 fontcoef; "
"uniform vec2 termsize; "
"uniform vec2 termcoef; "

"void main(void) "
"{ "
"   vec2 rawCoord = gl_TexCoord[0].xy; "                           /* varying from [0, termsize) in x and y */
"   vec2 conPos = floor(rawCoord); "                               /* console integer position */
"   vec2 pixPos = fract(rawCoord); "                               /* pixel offset within console position */
"   pixPos = vec2(pixPos.x*fontcoef.x,pixPos.y*fontcoef.y); "      /* Correct pixel offset for font tex location */

"   vec2 address = vec2(conPos.x*termcoef.x,conPos.y*termcoef.y); "
"	address=address+vec2(0.001, 0.001); "
"   vec4 charvec = texture2D(term,address);"
"   float inchar = (charvec.r * 256.0) + (charvec.g * 256.0 * 256.0);"          /* character */
"   vec4 tcharfcol = texture2D(termfcol, address); "           /* front color */
"   vec4 tcharbcol = texture2D(termbcol, address); "           /* back color */

"   vec4 tchar = vec4(mod(floor(inchar),floor(fontw)),floor(inchar/fontw), 0.0, 0.0); "  /* 1D index to 2D index map for character */

"   gl_FragColor = texture2D(font, vec2((tchar.x*fontcoef.x),(tchar.y*fontcoef.y))+pixPos.xy); "   /* magic func: finds pixel value in font file */
"   gl_FragColor=gl_FragColor.a*tcharfcol+(1.0-gl_FragColor.a)*tcharbcol; "      /* Coloring stage */
"} "
;

bool _CheckGL_Error(const char* GLcall, const char* file, const int line)
{
    GLenum errCode;
    if((errCode = glGetError())!=GL_NO_ERROR)
    {
		printf("OPENGL ERROR #%i: in file %s on line %i\n",errCode,file, line);
        printf("OPENGL Call: %s\n",GLcall);
        return false;
    }
    return true;
}

/* called before creating window */
void TCOD_opengl_init_attributes() {
	static bool first=true;
	if ( first ) {
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
		SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32 );
		/* ATI driver bug : enabling this might result in red screen */
		/* SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1); */
		first=false;
	}
}

/* console size (power of 2 and cells) */
static int POTconwidth, POTconheight, conwidth, conheight;
/* programs and shaders handles */
static GLhandleARB conProgram, conVertShader, conFragShader;
/* font texture handle */
static GLuint font_tex;
/* font power of 2 size and pixels */
static int POTfontwidth,POTfontheight, fontwidth,fontheight;
/* console data */
static GLuint Tex[ConsoleDataEnumSize];
static unsigned char *data[ConsoleDataEnumSize];
static bool dirty[ConsoleDataEnumSize];

/* extension function pointers */
static PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB=0;
static PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB=0;
static PFNGLSHADERSOURCEARBPROC glShaderSourceARB=0;
static PFNGLCOMPILESHADERARBPROC glCompileShaderARB=0;
static PFNGLGETINFOLOGARBPROC glGetInfoLogARB=0;
static PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB=0;
static PFNGLATTACHOBJECTARBPROC glAttachObjectARB=0;
static PFNGLLINKPROGRAMARBPROC glLinkProgramARB=0;
static PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB=0;
static PFNGLUNIFORM2FARBPROC glUniform2fARB=0;
static PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB=0;
static PFNGLUNIFORM1FARBPROC glUniform1fARB=0;
static PFNGLUNIFORM1IARBPROC glUniform1iARB=0;
#ifdef TCOD_WINDOWS
static PFNGLACTIVETEXTUREPROC glActiveTexture=0;
#endif
                                        
/* call after creating window */
bool TCOD_opengl_init_state(int conw, int conh, void *font) {
	SDL_Surface *font_surf=(SDL_Surface *)font;
	
	/* convert font for opengl */
	Uint32 rmask, gmask, bmask, amask;
	SDL_Surface *temp;
	SDL_Surface *temp_alpha;

	/* check opengl extensions */
	if ( TCOD_ctx.renderer == TCOD_RENDERER_GLSL ) {
		bool hasShader = false;
		const char *glexts=(const char *)glGetString(GL_EXTENSIONS);
		if (glexts ) {
			hasShader = (strstr(glexts,"GL_ARB_shader_objects") != NULL);
		}
		if (! hasShader ) {
			TCOD_LOG(("Missing GL_ARB_shader_objects extension. Falling back to fixed pipeline...\n"));
			TCOD_ctx.renderer = TCOD_RENDERER_OPENGL;		
		}
	}

	/* set extensions functions pointers */
   	glCreateShaderObjectARB=(PFNGLCREATESHADEROBJECTARBPROC)SDL_GL_GetProcAddress("glCreateShaderObjectARB");
	glGetObjectParameterivARB=(PFNGLGETOBJECTPARAMETERIVARBPROC)SDL_GL_GetProcAddress("glGetObjectParameterivARB");
	glShaderSourceARB=(PFNGLSHADERSOURCEARBPROC)SDL_GL_GetProcAddress("glShaderSourceARB");
	glCompileShaderARB=(PFNGLCOMPILESHADERARBPROC)SDL_GL_GetProcAddress("glCompileShaderARB");
	glGetInfoLogARB=(PFNGLGETINFOLOGARBPROC)SDL_GL_GetProcAddress("glGetInfoLogARB");
	glCreateProgramObjectARB=(PFNGLCREATEPROGRAMOBJECTARBPROC)SDL_GL_GetProcAddress("glCreateProgramObjectARB");
	glAttachObjectARB=(PFNGLATTACHOBJECTARBPROC)SDL_GL_GetProcAddress("glAttachObjectARB");
	glLinkProgramARB=(PFNGLLINKPROGRAMARBPROC)SDL_GL_GetProcAddress("glLinkProgramARB");
	glUseProgramObjectARB=(PFNGLUSEPROGRAMOBJECTARBPROC)SDL_GL_GetProcAddress("glUseProgramObjectARB");
	glUniform2fARB=(PFNGLUNIFORM2FARBPROC)SDL_GL_GetProcAddress("glUniform2fARB");
	glGetUniformLocationARB=(PFNGLGETUNIFORMLOCATIONARBPROC)SDL_GL_GetProcAddress("glGetUniformLocationARB");
	glUniform1fARB=(PFNGLUNIFORM1FARBPROC)SDL_GL_GetProcAddress("glUniform1fARB");
	glUniform1iARB=(PFNGLUNIFORM1IARBPROC)SDL_GL_GetProcAddress("glUniform1iARB");
#ifdef TCOD_WINDOWS	
	glActiveTexture=(PFNGLACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glActiveTexture");
#endif
	
	/* set opengl state */
	glEnable(GL_TEXTURE_2D);
	glClearColor(1.0f, 1.0f, 0.0f, 0.0f);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glClear( GL_COLOR_BUFFER_BIT );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	if ( TCOD_ctx.renderer == TCOD_RENDERER_GLSL ) {
		glOrtho(0, conw, 0, conh, -1.0f, 1.0f);
		glDisable (GL_BLEND); 
	} else {
		glOrtho(0, conw, conh, 0.0f, -1.0f, 1.0f);
		glEnable (GL_BLEND); 
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
/*#ifdef TCOD_WINDOWS  */
	if ( ! TCOD_ctx.fullscreen ) {
		/* turn vsync off in windowed mode */
		typedef bool (APIENTRY *PFNWGLSWAPINTERVALFARPROC)(int);
		PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;

		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)SDL_GL_GetProcAddress("wglSwapIntervalEXT");

		if (wglSwapIntervalEXT) wglSwapIntervalEXT(0);
	}
/*#endif */

	/* compute pot size */
	conwidth=conw;
	conheight=conh;
	POTconwidth=POTconheight=1;
	while ( POTconwidth < conw ) POTconwidth *= 2;
	while ( POTconheight < conh ) POTconheight *= 2;


	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
	#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
	#endif

	fontwidth=font_surf->w;
	fontheight=font_surf->h;
	POTfontwidth=POTfontheight=1;
	while ( POTfontwidth < fontwidth ) POTfontwidth *= 2;
	while ( POTfontheight < fontheight ) POTfontheight *= 2;

	SDL_SetColorKey(font_surf, SDL_SRCCOLORKEY, SDL_MapRGB(font_surf->format, 0, 0, 0));
	temp_alpha = SDL_DisplayFormatAlpha(font_surf);
	SDL_SetAlpha(temp_alpha, 0, SDL_ALPHA_TRANSPARENT);

	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, POTfontwidth, POTfontheight, 32, bmask, gmask, rmask, amask); /*BGRA */

	SDL_BlitSurface(temp_alpha, NULL, temp, NULL);
	SDL_FreeSurface(temp_alpha);

	CHECKGL(glGenTextures(1, &font_tex));
	CHECKGL(glBindTexture(GL_TEXTURE_2D, font_tex));
	SDL_LockSurface(temp);

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	CHECKGL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, temp->w, temp->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, temp->pixels));
	SDL_UnlockSurface(temp);
	SDL_FreeSurface(temp);
	return true;
}

static GLhandleARB loadShader(const char *txt, GLuint type) {
	int success;
	int infologLength = 0;
	int charsWritten = 0;
    char *infoLog;
	GLhandleARB v = glCreateShaderObjectARB(type);
	glShaderSourceARB(v, 1, &txt, 0);
	glCompileShaderARB(v);

	glGetObjectParameterivARB(v, GL_COMPILE_STATUS, &success);
	if(success!=GL_TRUE)
	{
	    /* something went wrong */
		glGetObjectParameterivARB(v, GL_INFO_LOG_LENGTH,&infologLength);
		if(infologLength>0)
		{
			infoLog = (char *)malloc(infologLength);
			glGetInfoLogARB(v, infologLength, &charsWritten, infoLog);
			printf("GLSL ERROR : %s\n",infoLog);
			free(infoLog);
		}
		return 0;
	}

	return v;
}

static bool loadProgram(const char *vertShaderCode, const char *fragShaderCode,
	GLhandleARB *vertShader, GLhandleARB *fragShader, GLhandleARB *prog) {
	/* Create and load Program and Shaders */
	int success;
	*prog = DBGCHECKGL(glCreateProgramObjectARB());

	*vertShader = loadShader(vertShaderCode, GL_VERTEX_SHADER);
	if ( *vertShader == 0 ) return false;
	glAttachObjectARB(*prog, *vertShader);

	*fragShader = loadShader(fragShaderCode, GL_FRAGMENT_SHADER);
	if ( *fragShader == 0 ) return false;
	glAttachObjectARB(*prog, *fragShader);

	glLinkProgramARB(*prog);

	glGetObjectParameterivARB(*prog, GL_LINK_STATUS, &success);
	if(success!=GL_TRUE)
	{
		/* something went wrong */
		int infologLength = 0;
		int charsWritten = 0;
		char *infoLog;
		glGetObjectParameterivARB(*prog, GL_INFO_LOG_LENGTH,&infologLength);
		if (infologLength > 0)
	    {
	        infoLog = (char *)malloc(infologLength);
	        glGetInfoLogARB(*prog, infologLength, &charsWritten, infoLog);
			printf("OPENGL ERROR: Program link Error");
			printf("%s\n",infoLog);
	        free(infoLog);
	    }
		return false;
	}
	return true;
}

bool TCOD_opengl_init_shaders() {
	int i;
	TCOD_color_t *fCol;
	if ( TCOD_ctx.renderer == TCOD_RENDERER_GLSL ) {
		if (! loadProgram(TCOD_con_vertex_shader, TCOD_con_pixel_shader, &conVertShader, &conFragShader, &conProgram ) ) return false;
	}
	/* Host side data init */
	for(i = 0; i< ConsoleDataEnumSize; i++)
	{
		data[i] = (unsigned char *)calloc(conwidth*conheight,ConsoleDataAlignment[i]);
		dirty[i]=true;
	}
	/* Initialize ForeCol to 255, 255, 255, 255 */
	fCol = (TCOD_color_t *)data[ForeCol];
	for( i = 0; i < conwidth*conheight; i++)
	{
	    fCol[i].r=255;
	    fCol[i].g=255;
	    fCol[i].b=255;
	}

    /* Generate Textures */
	glGenTextures(3, Tex);

	/* Character Texture */
	CHECKGL(glBindTexture(GL_TEXTURE_2D, Tex[Character]));

    CHECKGL(glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ));
    CHECKGL(glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ));

    CHECKGL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, POTconwidth, POTconheight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0));


    /* ForeCol Texture */
	CHECKGL(glBindTexture(GL_TEXTURE_2D, Tex[ForeCol]));

    CHECKGL(glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ));
    CHECKGL(glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ));

	CHECKGL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, POTconwidth, POTconheight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0));

    /* BackCol Texture */
	CHECKGL(glBindTexture(GL_TEXTURE_2D, Tex[BackCol]));

    CHECKGL(glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ));
    CHECKGL(glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ));

	CHECKGL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, POTconwidth, POTconheight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0));

	CHECKGL(glBindTexture(GL_TEXTURE_2D, 0));
	
	return true;
}

static bool updateTex(ConsoleDataEnum dataType) {
	GLenum Type=0;
	DBGCHECKGL(glBindTexture(GL_TEXTURE_2D, Tex[dataType]));

	switch(ConsoleDataAlignment[dataType])
	{
	case 1:
		Type = GL_RED;
		break;
	/*case 2:
		Type = GL_RG;
		break; */
	case 3:
		Type = GL_RGB;
		break;
	case 4:
		Type = GL_RGBA;
		break;
	}
    /*glPixelStorei(GL_UNPACK_ALIGNMENT, 1); */
	DBGCHECKGL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, conwidth, conheight, Type, GL_UNSIGNED_BYTE, data[dataType]));

	DBGCHECKGL(glBindTexture(GL_TEXTURE_2D,0));
	return true;
}

static void updateChar(ConsoleDataEnum dataType, int BufferPos, unsigned char *c, int length, int offset) {
	int i;
	dirty[dataType] = true;		/* Set dirty so Texture gets updated next frame */

	for(i = 0; i<length; i++) {
		data[dataType][BufferPos*ConsoleDataAlignment[dataType] + i + offset] = c[i];
	}

}

void TCOD_opengl_putchar_ex(int x, int y, int c, TCOD_color_t fore, TCOD_color_t back) {
	int loc = x+y*conwidth;

	if ( TCOD_ctx.renderer == TCOD_RENDERER_GLSL ) {
		updateChar(Character, loc, (unsigned char *)&c, ConsoleDataAlignment[Character], 0);
		updateChar(ForeCol, loc, &fore.r, ConsoleDataAlignment[ForeCol], 0);
	}
	updateChar(BackCol, loc, &back.r, ConsoleDataAlignment[BackCol], 0);

}

bool TCOD_opengl_render( int oldFade, bool *ascii_updated, char_t *console_buffer, char_t *prev_console_buffer) {
	int x,y,i;
	int fade = (int)TCOD_console_get_fade();
	bool track_changes=(oldFade == fade && prev_console_buffer);
	char_t *c=console_buffer;
	char_t *oc=prev_console_buffer;
	int ascii;
	/* update opengl data */
	/* TODO use function pointers so that libtcod's putchar directly updates opengl data */
	for (y=0;y<conheight;y++) {
		for (x=0; x<conwidth; x++) {
			bool changed=true;
			if ( c->cf == -1 ) c->cf = TCOD_ctx.ascii_to_tcod[c->c];
			if ( track_changes ) {
				changed=false;
				if ( c->dirt || ascii_updated[ c->c ] || c->back.r != oc->back.r || c->back.g != oc->back.g
					|| c->back.b != oc->back.b || c->fore.r != oc->fore.r
					|| c->fore.g != oc->fore.g || c->fore.b != oc->fore.b
					|| c->c != oc->c || c->cf != oc->cf) {
					changed=true;
				}
			}
			c->dirt=0;
			if ( changed ) {
				TCOD_opengl_putchar_ex(x,y,c->cf,c->fore,c->back);
			}
			c++;oc++;
		}
	}

	/* check if any of the textures have changed since they were last uploaded */
	for( i = 0; i< ConsoleDataEnumSize; i++)
	{
		if(dirty[i])
		{
			updateTex((ConsoleDataEnum)i);
			dirty[i] = false;
		}
	}
	if ( TCOD_ctx.renderer == TCOD_RENDERER_OPENGL ) {
		/* fixed pipeline for video cards without pixel shader support */
		/* draw the background as a single quad */
		float texw=(float)conwidth/POTconwidth;
		float texh=(float)conheight/POTconheight;
		float fonw=(float)fontwidth/(TCOD_ctx.fontNbCharHoriz*POTfontwidth);
		float fonh=(float)fontheight/(TCOD_ctx.fontNbCharVertic*POTfontheight);
		char_t *c;
	    DBGCHECKGL(glBindTexture(GL_TEXTURE_2D, Tex[BackCol]));
		DBGCHECKGL(glBegin(GL_QUADS);
			glColor3f(1.0,1.0,1.0);
			glTexCoord2f( 0.0, 0.0 );
			glVertex2i( 0, 0);
			glTexCoord2f( 0.0, texh);
			glVertex2i( 0, conheight );
			glTexCoord2f( texw, texh );
			glVertex2i( conwidth, conheight);
			glTexCoord2f( texw, 0.0 );
			glVertex2i( conwidth, 0 );
		glEnd());
		/* draw the characters (one quad per cell) */
	    DBGCHECKGL(glBindTexture(GL_TEXTURE_2D, font_tex));
	    
	    c=console_buffer;
		for (y=0;y<conheight;y++) {
			for (x=0; x<conwidth; x++) {
				if ( c->c != ' ' ) {
					TCOD_color_t f=c->fore;
					TCOD_color_t b=c->back;
					/* only draw character if foreground color != background color */
					if ( f.r != b.r || f.g != b.g || f.b != b.b ) {
						int srcx,srcy,destx,desty;
						destx=x;/* *TCOD_font_width; */
						desty=y;/* *TCOD_font_height; */
						if ( TCOD_ctx.fullscreen ) {
							destx+=TCOD_ctx.fullscreen_offsetx/TCOD_ctx.font_width;
							desty+=TCOD_ctx.fullscreen_offsety/TCOD_ctx.font_height;
						}
						/* draw foreground */
						ascii=c->cf;
						srcx = (ascii%TCOD_ctx.fontNbCharHoriz);
						srcy = (ascii/TCOD_ctx.fontNbCharHoriz);
						glBegin( GL_QUADS );
						glColor3f((GLfloat)(f.r/255.0), (GLfloat)(f.g/255.0), (GLfloat)(f.b/255.0));
						glTexCoord2f( srcx*fonw, srcy*fonh );
						glVertex2i( destx, desty);
						glTexCoord2f( srcx*fonw, (srcy+1)*fonh );
						glVertex2i( destx, desty+1 );
						glTexCoord2f( (srcx+1)*fonw, (srcy+1)*fonh );
						glVertex2i( destx+1, desty+1 );
						glTexCoord2f( (srcx+1)*fonw, srcy*fonh );
						glVertex2i( destx+1, desty );
						glEnd();
					}
				}
				c++;
			}
		}
	    DBGCHECKGL(glBindTexture(GL_TEXTURE_2D, 0));
	} else {
		/* actual rendering */
	    DBGCHECKGL(glUseProgramObjectARB(conProgram));
	
		/* Technically all these glUniform calls can be moved to SFConsole() when the shader is loaded */
		/* None of these change */
		/* The Textures still need to bind to the same # Activetexture throughout though */
	    DBGCHECKGL(glUniform2fARB(glGetUniformLocationARB(conProgram,"termsize"), (float) conwidth, (float) conheight));
		DBGCHECKGL(glUniform2fARB(glGetUniformLocationARB(conProgram,"termcoef"), 1.0f/POTconwidth, 1.0f/POTconheight));
	    DBGCHECKGL(glUniform1fARB(glGetUniformLocationARB(conProgram,"fontw"), (float)TCOD_ctx.fontNbCharHoriz));
	    DBGCHECKGL(glUniform2fARB(glGetUniformLocationARB(conProgram,"fontcoef"), (float)(fontwidth)/(POTfontwidth*TCOD_ctx.fontNbCharHoriz), (float)(fontheight)/(POTfontheight*TCOD_ctx.fontNbCharVertic)));

	
	    DBGCHECKGL(glActiveTexture(GL_TEXTURE0));
	    DBGCHECKGL(glBindTexture(GL_TEXTURE_2D, font_tex));
	    DBGCHECKGL(glUniform1iARB(glGetUniformLocationARB(conProgram,"font"),0));
	
	    DBGCHECKGL(glActiveTexture(GL_TEXTURE1));
	    DBGCHECKGL(glBindTexture(GL_TEXTURE_2D, Tex[Character]));
	    DBGCHECKGL(glUniform1iARB(glGetUniformLocationARB(conProgram,"term"),1));
	
	    DBGCHECKGL(glActiveTexture(GL_TEXTURE2));
	    DBGCHECKGL(glBindTexture(GL_TEXTURE_2D, Tex[ForeCol]));
	    DBGCHECKGL(glUniform1iARB(glGetUniformLocationARB(conProgram,"termfcol"),2));
	
	    DBGCHECKGL(glActiveTexture(GL_TEXTURE3));
	    DBGCHECKGL(glBindTexture(GL_TEXTURE_2D, Tex[BackCol]));
	    DBGCHECKGL(glUniform1iARB(glGetUniformLocationARB(conProgram,"termbcol"),3));
	
	/*    DBGCHECKGL(shader->Validate()); */
	
		DBGCHECKGL(glBegin(GL_QUADS);
	        glTexCoord2f(0.0f, 1.0f);
			glVertex3f(-1.0f,-1.0f,0.0f);
	        glTexCoord2f(1.0f, 1.0f);
			glVertex3f(1.0f,-1.0f,0.0f);
	        glTexCoord2f(1.0f, 0.0f);
			glVertex3f(1.0f,1.0f, 0.0f);
	        glTexCoord2f(0.0f, 0.0f);
			glVertex3f(-1.0f,1.0f,0.0f);
		glEnd());
	
	    DBGCHECKGL(glBindTexture(GL_TEXTURE_2D, 0));
	
	    DBGCHECKGL(glUseProgramObjectARB(0));
	}
	/* fading overlay */
	if ( fade != 255 ) {
		int x=0,y=0;
		if ( TCOD_ctx.fullscreen ) {
			x=TCOD_ctx.fullscreen_offsetx/TCOD_ctx.font_width;
			y=TCOD_ctx.fullscreen_offsety/TCOD_ctx.font_height;
		}
		glBegin( GL_QUADS );
		glColor4f(TCOD_ctx.fading_color.r/255.0f,TCOD_ctx.fading_color.g/255.0f,TCOD_ctx.fading_color.b/255.0f,1.0f-fade/255.0f);
		glVertex2i( x, y);
		glVertex2i( x, y+conheight );
		glVertex2i( x+conwidth, y+conheight );
		glVertex2i( x+conwidth, y);
		glEnd();
	}
	return true;
}

void TCOD_opengl_swap() {
	SDL_GL_SwapBuffers();
}

void * TCOD_opengl_get_screen() {
	SDL_Surface *surf;
	int pixw,pixh,offx=0,offy=0,x,y;
	Uint32 mask,nmask;

	/* allocate a pixel buffer */
	pixw=TCOD_ctx.root->w * TCOD_ctx.font_width;
	pixh=TCOD_ctx.root->h * TCOD_ctx.font_height;
	surf=TCOD_sys_get_surface(pixw,pixh,false);
	if ( TCOD_ctx.fullscreen ) {
		offx=TCOD_ctx.fullscreen_offsetx;
		offy=TCOD_ctx.fullscreen_offsety;
	}

	/* get pixel data from opengl */
	glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT );
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(offx,offy,pixw,pixh, GL_RGB, GL_UNSIGNED_BYTE, surf->pixels);
	glPopClientAttrib();
	
	/* vertical flip (opengl has lower-left origin, SDL upper left) */
	mask=surf->format->Rmask|surf->format->Gmask|surf->format->Bmask;
	nmask=~mask;
	for (x=0; x < surf->w; x++) {
		for (y=0; y < surf->h/2; y++) {
			int offsrc=x*3+y*surf->pitch;
			int offdst=x*3+(surf->h-1-y)*surf->pitch;
			Uint32 *pixsrc = (Uint32 *)(((Uint8 *)surf->pixels)+offsrc);
			Uint32 *pixdst = (Uint32 *)(((Uint8 *)surf->pixels)+offdst);
			Uint32 tmp = *pixsrc;
			*pixsrc = ((*pixsrc) & nmask) | ((*pixdst) & mask);
			*pixdst = ((*pixdst) & nmask) | (tmp & mask);
		}
	}

	return (void *)surf;
}

#endif

