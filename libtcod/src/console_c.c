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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "libtcod.h"
#include "libtcod_int.h"
#ifndef NO_UNICODE
#include <wchar.h>
#include <wctype.h>
#endif

#if defined( TCOD_VISUAL_STUDIO )
static const char *version_string ="libtcod "TCOD_STRVERSION;
#else
static const char *version_string __attribute__((unused)) ="libtcod "TCOD_STRVERSION;
#endif

TCOD_internal_context_t TCOD_ctx={
	/* number of characters in the bitmap font */
	16,16,
	/* font type and layout */
	false,false,false,
	/* character size in font */
	8,8,
	"terminal.png","",
	NULL,NULL,NULL,0,false,0,0,0,0,0,0,
	/* default renderer to use */
	TCOD_RENDERER_GLSL,
	NULL,
	/* fading data */
	{0,0,0},255,
	/* window closed ? */
	false,
	/* mouse focus ? */ 
	false,
	/* application active ? */
	true,
};

static TCOD_color_t color_control_fore[TCOD_COLCTRL_NUMBER];
static TCOD_color_t color_control_back[TCOD_COLCTRL_NUMBER];

void TCOD_console_set_color_control(TCOD_colctrl_t con, TCOD_color_t fore, TCOD_color_t back) {
	TCOD_IFNOT(con >= TCOD_COLCTRL_1 && con <= TCOD_COLCTRL_NUMBER ) return;
	color_control_fore[con-1]=fore;
	color_control_back[con-1]=back;
}

void TCOD_fatal(const char *fmt, ...) {
	va_list ap;
	TCOD_sys_term();
	printf("%s\n",version_string);
	va_start(ap,fmt);
	vprintf(fmt,ap);
	va_end(ap);
	printf ("\n");
	exit (1);
}

void TCOD_fatal_nopar(const char *msg) {
	TCOD_sys_term();
	printf("%s\n%s\n",version_string,msg);
	exit (1);
}

TCOD_console_t TCOD_console_new(int w, int h)  {
	TCOD_IFNOT(w > 0 && h > 0 ) {
		return NULL;
	} else {
		TCOD_console_data_t *con=(TCOD_console_data_t *)calloc(sizeof(TCOD_console_data_t),1);
		con->w=w;
		con->h=h;
		TCOD_console_init(con,NULL,false);
		if(TCOD_ctx.root) {
			con->alignment=TCOD_ctx.root->alignment;
			con->bkgnd_flag=TCOD_ctx.root->bkgnd_flag;
		}
		return (TCOD_console_t)con;
	}
}

TCOD_key_t TCOD_console_check_for_keypress(int flags) {
	return TCOD_sys_check_for_keypress(flags);
}

TCOD_key_t TCOD_console_wait_for_keypress(bool flush) {
	return TCOD_sys_wait_for_keypress(flush);
}

bool TCOD_console_is_window_closed() {
	return TCOD_ctx.is_window_closed;
}

bool TCOD_console_has_mouse_focus() {
	return TCOD_ctx.app_has_mouse_focus;
}

bool TCOD_console_is_active() {
	return TCOD_ctx.app_is_active;
}

void TCOD_console_set_window_title(const char *title) {
	TCOD_sys_set_window_title(title);
}

void TCOD_console_set_fullscreen(bool fullscreen) {
	TCOD_IFNOT(TCOD_ctx.root != NULL) return;
	TCOD_sys_set_fullscreen(fullscreen);
	TCOD_ctx.fullscreen=fullscreen;
}

bool TCOD_console_is_fullscreen() {
	return TCOD_ctx.fullscreen;
}

void TCOD_console_set_background_flag(TCOD_console_t con,TCOD_bkgnd_flag_t flag) {
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT ( dat != NULL ) return;
	dat->bkgnd_flag=flag;
}

TCOD_bkgnd_flag_t TCOD_console_get_background_flag(TCOD_console_t con) {
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT ( dat != NULL ) return TCOD_BKGND_NONE;
	return dat->bkgnd_flag;
}

void TCOD_console_set_alignment(TCOD_console_t con,TCOD_alignment_t alignment) {
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT ( dat != NULL ) return;
	dat->alignment=alignment;
}

TCOD_alignment_t TCOD_console_get_alignment(TCOD_console_t con) {
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT ( dat != NULL ) return TCOD_LEFT;
	return dat->alignment;
}


void TCOD_console_delete(TCOD_console_t con) {
    TCOD_console_data_t *dat=(TCOD_console_data_t *)(con);
	if (! dat ) {
		dat=TCOD_ctx.root;
		TCOD_sys_term();
		TCOD_ctx.root=NULL;
	}
	free(dat->buf);
	free(dat->oldbuf);
	free(dat);
}

void TCOD_console_blit(TCOD_console_t srcCon,int xSrc, int ySrc, int wSrc, int hSrc,
	TCOD_console_t dstCon, int xDst, int yDst, float foreground_alpha, float background_alpha) {
    TCOD_console_data_t *src=srcCon ? (TCOD_console_data_t *)srcCon : TCOD_ctx.root;
    TCOD_console_data_t *dst=dstCon ? (TCOD_console_data_t *)dstCon : TCOD_ctx.root;
	int cx,cy;
	if ( wSrc == 0 ) wSrc=src->w;
	if ( hSrc == 0 ) hSrc=src->h;
	TCOD_IFNOT(wSrc > 0 && hSrc > 0 ) return;
	TCOD_IFNOT(xDst+wSrc >= 0 && yDst+hSrc >= 0 && xDst < dst->w && yDst < dst->h) return;
	for (cx = xSrc; cx < xSrc+wSrc; cx++) {
		for (cy = ySrc; cy < ySrc+hSrc; cy++) {
			/* check if we're outside the dest console */
			int dx=cx-xSrc+xDst;
			int dy=cy-ySrc+yDst;
			uint8 dirt;
			char_t srcChar,dstChar;
			if ( (unsigned) cx >= (unsigned) src->w || (unsigned)cy >= (unsigned) src->h ) continue;
			if ( (unsigned) dx >= (unsigned) dst->w || (unsigned)dy >= (unsigned) dst->h ) continue;
			/* check if source pixel is transparent */
			srcChar=src->buf[cy * src->w+cx];
			if ( src->haskey && srcChar.back.r == src->key.r
				&& srcChar.back.g == src->key.g && srcChar.back.b == src->key.b ) continue;
			if ( foreground_alpha == 1.0f && background_alpha == 1.0f ) {
				dstChar=srcChar;
			} else {
				dstChar=dst->buf[dy * dst->w + dx];

				dstChar.back = TCOD_color_lerp(dstChar.back,srcChar.back,background_alpha);
				if ( srcChar.c == ' ' ) {
					dstChar.fore = TCOD_color_lerp(dstChar.fore,srcChar.back,background_alpha);
				} else if (dstChar.c == ' ') {
					dstChar.c=srcChar.c;
					dstChar.cf=srcChar.cf;
					dstChar.fore = TCOD_color_lerp(dstChar.back,srcChar.fore,foreground_alpha);
				} else if (dstChar.c == srcChar.c) {
					dstChar.fore = TCOD_color_lerp(dstChar.fore,srcChar.fore,foreground_alpha);
				} else {
					if ( foreground_alpha < 0.5f ) {
						dstChar.fore=TCOD_color_lerp(dstChar.fore,dstChar.back,
							foreground_alpha*2);
					} else {
						dstChar.c=srcChar.c;
						dstChar.cf=srcChar.cf;
						dstChar.fore=TCOD_color_lerp(dstChar.back,srcChar.fore,
							(foreground_alpha-0.5f)*2);
					}
				}
			}
			dirt=dst->buf[dy * dst->w + dx].dirt;
			dst->buf[dy * dst->w + dx] = dstChar;
			dst->buf[dy * dst->w + dx].dirt=dirt;
		}
	}
}


void TCOD_console_flush() {
	TCOD_console_data_t *dat=TCOD_ctx.root;
	TCOD_IFNOT(TCOD_ctx.root != NULL) return;
	TCOD_sys_flush(true);
	memcpy(dat->oldbuf,dat->buf,sizeof(char_t)*
		dat->w*dat->h);
}

void TCOD_console_set_fade(uint8 val, TCOD_color_t fadecol) {
	TCOD_ctx.fade=val;
	TCOD_ctx.fading_color=fadecol;
}

uint8 TCOD_console_get_fade() {
	return TCOD_ctx.fade;
}

TCOD_color_t TCOD_console_get_fading_color() {
	return TCOD_ctx.fading_color;
}


void TCOD_console_put_char(TCOD_console_t con,int x, int y, int c, TCOD_bkgnd_flag_t flag) {
	int offset;
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT ( dat != NULL && (unsigned)(x) < (unsigned)dat->w && (unsigned)(y) < (unsigned)dat->h ) return;
	TCOD_IFNOT (c >= 0 && c < TCOD_ctx.max_font_chars ) return;
	offset = y * dat->w + x;
	dat->buf[ offset ].c = c;
	dat->buf[ offset ].cf = TCOD_ctx.ascii_to_tcod[c];
	dat->buf[ offset ].fore = dat->fore;
	TCOD_console_set_char_background(con,x,y,dat->back,(TCOD_bkgnd_flag_t)flag);
}

void TCOD_console_put_char_ex(TCOD_console_t con,int x, int y, int c, TCOD_color_t fore, TCOD_color_t back) {
	int offset;
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT ( dat != NULL && (unsigned)(x) < (unsigned)dat->w && (unsigned)(y) < (unsigned)dat->h ) return;
	TCOD_IFNOT (c >= 0 && c < TCOD_ctx.max_font_chars ) return;
	offset = y * dat->w + x;
	dat->buf[ offset ].c = c;
	dat->buf[ offset ].cf = TCOD_ctx.ascii_to_tcod[c];
	dat->buf[ offset ].fore = fore;
	dat->buf[ offset ].back = back;
}

void TCOD_console_set_dirty(int dx, int dy, int dw, int dh) {
	int x,y;
	TCOD_console_data_t *dat=TCOD_ctx.root;
	TCOD_IFNOT(dat != NULL) return;
	TCOD_IFNOT(dx < dat->w && dy < dat->h && dx+dw >= 0 && dy+dh >= 0 ) return;
	TCOD_IFNOT( dx >= 0 ) {
		dw += dx;
		dx = 0;
	}
	TCOD_IFNOT( dy >= 0 ) {
		dh += dy;
		dy = 0;
	}
	TCOD_IFNOT( dx+dw <= dat->w ) dw = dat->w-dx;
	TCOD_IFNOT( dy+dh <= dat->h ) dh = dat->h-dy;

	for (x=dx; x < dx+dw;x++) {
		for (y=dy; y < dy+dh; y++) {
			int off=x+dat->w*y;
			dat->buf[off].dirt=1;
		}
	}
}

void TCOD_console_clear(TCOD_console_t con) {
	int x,y;
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT(dat != NULL) return;
	for (x=0; x < dat->w;x++) {
		for (y=0; y < dat->h; y++) {
			int off=x+dat->w*y;
			dat->buf[off].dirt=0;
			dat->buf[off].c=' ';
			dat->buf[off].cf=TCOD_ctx.ascii_to_tcod?TCOD_ctx.ascii_to_tcod[' ']:0;
			dat->buf[off].fore=dat->fore;
			dat->buf[off].back=dat->back;
		}
	}
}

TCOD_color_t TCOD_console_get_char_background(TCOD_console_t con,int x, int y) {
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT ( dat != NULL
		&& (unsigned)(x) < (unsigned)dat->w && (unsigned)(y) < (unsigned)dat->h )
		return TCOD_black;
	return dat->buf[ y * dat->w + x ].back;
}

void TCOD_console_set_char_foreground(TCOD_console_t con,int x,int y, TCOD_color_t col) {
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	if ( (unsigned)(x) >= (unsigned)dat->w || (unsigned)(y) >= (unsigned)dat->h ) return;
	TCOD_IFNOT ( dat != NULL
		&& (unsigned)(x) < (unsigned)dat->w && (unsigned)(y) < (unsigned)dat->h )
		return;
	dat->buf[ y * dat->w + x ].fore=col;
}

TCOD_color_t TCOD_console_get_char_foreground(TCOD_console_t con,int x, int y) {
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT ( dat != NULL
		&& (unsigned)(x) < (unsigned)dat->w && (unsigned)(y) < (unsigned)dat->h )
		return TCOD_white;
	return dat->buf[ y * dat->w + x ].fore;
}

int TCOD_console_get_char(TCOD_console_t con,int x, int y) {
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT ( dat != NULL
		&& (unsigned)(x) < (unsigned)dat->w && (unsigned)(y) < (unsigned)dat->h )
		return 0;
	return dat->buf[ y * dat->w + x ].c;
}

void TCOD_console_set_char_background(TCOD_console_t con,int x, int y, TCOD_color_t col, TCOD_bkgnd_flag_t flag) {
	TCOD_color_t *back;
	int newr,newg,newb;
	int alpha;
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT ( dat != NULL
		&& (unsigned)(x) < (unsigned)dat->w && (unsigned)(y) < (unsigned)dat->h )
		return;
	back=&dat->buf[y*dat->w+x].back;
	if ( flag == TCOD_BKGND_DEFAULT ) flag = dat->bkgnd_flag;
	switch ( flag & 0xff ) {
		case TCOD_BKGND_SET : *back = col; break;
		case TCOD_BKGND_MULTIPLY  : *back= TCOD_color_multiply(*back, col); break;
		case TCOD_BKGND_LIGHTEN :
			back->r = MAX(back->r,col.r);
			back->g = MAX(back->g,col.g);
			back->b = MAX(back->b,col.b);
		break;
		case TCOD_BKGND_DARKEN :
			back->r = MIN(back->r,col.r);
			back->g = MIN(back->g,col.g);
			back->b = MIN(back->b,col.b);
		break;
		case TCOD_BKGND_SCREEN :
			/* newbk = white - (white - oldbk) * (white - curbk) */
			back->r = (uint8)(255 - (int)(255 - back->r)*(255 - col.r)/255);
			back->g = (uint8)(255 - (int)(255 - back->g)*(255 - col.g)/255);
			back->b = (uint8)(255 - (int)(255 - back->b)*(255 - col.b)/255);
		break;
		case TCOD_BKGND_COLOR_DODGE :
			/* newbk = curbk / (white - oldbk) */
			if ( back->r != 255 ) newr = (int)(255 * col.r) / (255 - back->r);
			else newr=255;
			if ( back->g != 255 ) newg = (int)(255 * col.g) / (255 - back->g);
			else newg=255;
			if ( back->b != 255 ) newb = (int)(255 * col.b) / (255 - back->b);
			else newb=255;
			back->r=(uint8)CLAMP(0,255,newr);
			back->g=(uint8)CLAMP(0,255,newg);
			back->b=(uint8)CLAMP(0,255,newb);
		break;
		case TCOD_BKGND_COLOR_BURN :
			/* newbk = white - (white - oldbk) / curbk */
			if ( col.r > 0 ) newr = 255 - (int)(255 * (255 - back->r) )/col.r;
			else newr = 0;
			if ( col.g > 0 ) newg = 255 - (int)(255 * (255 - back->g)) /col.g;
			else newg = 0;
			if ( col.b > 0 ) newb = 255 - (int)(255 * (255 - back->b)) /col.b;
			else newb = 0;
			back->r=(uint8)CLAMP(0,255,newr);
			back->g=(uint8)CLAMP(0,255,newg);
			back->b=(uint8)CLAMP(0,255,newb);
		break;
		case TCOD_BKGND_ADD :
			/* newbk = oldbk + curbk */
			newr=(int)(back->r)+col.r;
			newg=(int)(back->g)+col.g;
			newb=(int)(back->b)+col.b;
			back->r=(uint8)CLAMP(0,255,newr);
			back->g=(uint8)CLAMP(0,255,newg);
			back->b=(uint8)CLAMP(0,255,newb);
		break;
		case TCOD_BKGND_ADDA :
			alpha=(flag >> 8);
			/* newbk = oldbk + alpha * curbk */
			newr=(int)(back->r)+alpha * col.r / 255;
			newg=(int)(back->g)+alpha * col.g / 255;
			newb=(int)(back->b)+alpha * col.b / 255;
			back->r=(uint8)CLAMP(0,255,newr);
			back->g=(uint8)CLAMP(0,255,newg);
			back->b=(uint8)CLAMP(0,255,newb);
		break;
		case TCOD_BKGND_BURN :
			/* newbk = oldbk + curbk - white */
			newr=(int)(back->r)+col.r-255;
			newg=(int)(back->g)+col.g-255;
			newb=(int)(back->b)+col.b-255;
			back->r=(uint8)CLAMP(0,255,newr);
			back->g=(uint8)CLAMP(0,255,newg);
			back->b=(uint8)CLAMP(0,255,newb);
		break;
		case TCOD_BKGND_OVERLAY :
			/* newbk = curbk.x <= 0.5 ? 2*curbk*oldbk : white - 2*(white-curbk)*(white-oldbk) */
			newr = col.r <= 128 ? 2 * (int)(col.r) * back->r / 255 : 255 - 2*(int)(255 - col.r)*(255-back->r)/255;
			newg = col.g <= 128 ? 2 * (int)(col.g) * back->g / 255 : 255 - 2*(int)(255 - col.g)*(255-back->g)/255;
			newb = col.b <= 128 ? 2 * (int)(col.b) * back->b / 255 : 255 - 2*(int)(255 - col.b)*(255-back->b)/255;
			back->r=(uint8)CLAMP(0,255,newr);
			back->g=(uint8)CLAMP(0,255,newg);
			back->b=(uint8)CLAMP(0,255,newb);
		break;
		case TCOD_BKGND_ALPH :
			/* newbk = (1.0f-alpha)*oldbk + alpha*(curbk-oldbk) */
			alpha=(flag >> 8);
			*back = TCOD_color_lerp(*back,col,(float)(alpha/255.0f));
		break;
		default : break;
	}
}

void TCOD_console_set_char(TCOD_console_t con,int x, int y, int c) {
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	if ( (unsigned)(x) >= (unsigned)dat->w || (unsigned)(y) >= (unsigned)dat->h ) return;
	dat->buf[ y * dat->w + x ].c=c;
	dat->buf[ y * dat->w + x ].cf = TCOD_ctx.ascii_to_tcod[c];
}

static void TCOD_console_clamp(int cx, int cy, int cw, int ch, int *x, int *y, int *w, int *h) {
	if ( *x + *w > cw ) *w = cw - *x;
	if ( *y + *h > ch ) *h = ch - *y;
	if (*x < cx ) {
		*w -= cx - *x;
		*x = cx;
	}
	if (*y < cy ) {
		*h -= cy - *y;
		*y = cy;
	}
}

void TCOD_console_rect(TCOD_console_t con,int x,int y, int rw, int rh, bool clear, TCOD_bkgnd_flag_t flag) {
	int cx,cy;
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT ( dat != NULL ) return;
	TCOD_ASSERT((unsigned)(x) < (unsigned)dat->w && (unsigned)(y) < (unsigned)dat->h );
	TCOD_ASSERT(x+rw <= dat->w && y+rh <= dat->h );

	TCOD_console_clamp(0,0,dat->w,dat->h,&x,&y,&rw,&rh);
	TCOD_IFNOT(rw > 0 && rh > 0) return;
	for (cx=x;cx < x+rw; cx++) {
		for (cy=y;cy<y+rh;cy++) {
			TCOD_console_set_char_background(con,cx,cy,dat->back,flag);
			if ( clear ) {
				dat->buf[cx + cy*dat->w].c=' ';
				dat->buf[cx + cy*dat->w].cf=TCOD_ctx.ascii_to_tcod[' '];
			}
		}
	}
}

void TCOD_console_hline(TCOD_console_t con,int x,int y, int l, TCOD_bkgnd_flag_t flag) {
	int i;
	for (i=x; i< x+l; i++) TCOD_console_put_char(con,i,y,TCOD_CHAR_HLINE,flag);
}

void TCOD_console_vline(TCOD_console_t con,int x,int y, int l, TCOD_bkgnd_flag_t flag) {
	int i;
	for (i=y; i< y+l; i++) TCOD_console_put_char(con,x,i,TCOD_CHAR_VLINE,flag);
}

char *TCOD_console_vsprint(const char *fmt, va_list ap) {
	#define NB_BUFFERS 10
	#define INITIAL_SIZE 512
	/* several static buffers in case the function is used more than once in a single function call */
	static char *msg[NB_BUFFERS]={NULL};
	static int buflen[NB_BUFFERS]={0};
	static int curbuf=0;
	char *ret;
	bool ok=false;
	if (!msg[0]) {
		int i;
		for (i=0; i < NB_BUFFERS; i++) {
			buflen[i]=INITIAL_SIZE;
			msg[i]=(char *)calloc(sizeof(char),INITIAL_SIZE);
		}
	}
	do {
		/* warning ! depending on the compiler, vsnprintf return -1 or
		 the expected string length if the buffer is not big enough */
		int len = vsnprintf(msg[curbuf],buflen[curbuf],fmt,ap);
		ok=true;
		if (len < 0 || len >= buflen[curbuf]) {
			/* buffer too small. */
			if ( len > 0 ) {
				while ( buflen[curbuf] < len+1 ) buflen[curbuf]*=2;
			} else {
				buflen[curbuf]*=2;
			}
			free( msg[curbuf] );
			msg[curbuf]=(char *)calloc(sizeof(char),buflen[curbuf]);
			ok=false;
		}
	} while (! ok);
	ret=msg[curbuf];
	curbuf = (curbuf+1)%NB_BUFFERS;
	return ret;
}

void TCOD_console_print_frame(TCOD_console_t con,int x,int y,int w,int h, bool empty, TCOD_bkgnd_flag_t flag, const char *fmt, ...) {
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_console_put_char(con,x,y,TCOD_CHAR_NW,flag);
	TCOD_console_put_char(con,x+w-1,y,TCOD_CHAR_NE,flag);
	TCOD_console_put_char(con,x,y+h-1,TCOD_CHAR_SW,flag);
	TCOD_console_put_char(con,x+w-1,y+h-1,TCOD_CHAR_SE,flag);
	TCOD_console_hline(con,x+1,y,w-2,flag);
	TCOD_console_hline(con,x+1,y+h-1,w-2,flag);
	if ( h > 2 ) {
		TCOD_console_vline(con,x,y+1,h-2,flag);
		TCOD_console_vline(con,x+w-1,y+1,h-2,flag);
		if ( empty ) {
			TCOD_console_rect(con,x+1,y+1,w-2,h-2,true,flag);
		}
	}
	if (fmt) {
		va_list ap;
		int xs;
		TCOD_color_t tmp;
		char *title;
		va_start(ap,fmt);
		title = TCOD_console_vsprint(fmt,ap);
		va_end(ap);
		title[w-3]=0; /* truncate if needed */
		xs = x + (w-strlen(title)-2)/2;
		tmp=dat->back; /* swap colors */
		dat->back=dat->fore;
		dat->fore=tmp;
		TCOD_console_print_ex(con,xs,y,TCOD_BKGND_SET,TCOD_LEFT," %s ",title);
		tmp=dat->back; /* swap colors */
		dat->back=dat->fore;
		dat->fore=tmp;
	}
}

void TCOD_console_print(TCOD_console_t con,int x, int y, const char *fmt, ...) {
	va_list ap;
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT ( dat != NULL ) return;
	va_start(ap,fmt);
	TCOD_console_print_internal(con,x,y,0,0,dat->bkgnd_flag,
		dat->alignment,TCOD_console_vsprint(fmt,ap), false, false);
	va_end(ap);
}

void TCOD_console_print_ex(TCOD_console_t con,int x, int y,
	TCOD_bkgnd_flag_t flag, TCOD_alignment_t alignment, const char *fmt, ...) {
	va_list ap;
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT ( dat != NULL ) return;
	va_start(ap,fmt);
	TCOD_console_print_internal(con,x,y,0,0,flag,alignment,
		TCOD_console_vsprint(fmt,ap), false, false);
	va_end(ap);
}

int TCOD_console_print_rect(TCOD_console_t con,int x, int y, int w, int h, const char *fmt, ...) {
	int ret;
	va_list ap;
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT ( dat != NULL ) return 0;
	va_start(ap,fmt);
	ret = TCOD_console_print_internal(con,x,y,w,h,dat->bkgnd_flag,dat->alignment,
		TCOD_console_vsprint(fmt,ap), true, false);
	va_end(ap);
	return ret;
}

int TCOD_console_print_rect_ex(TCOD_console_t con,int x, int y, int w, int h,
	TCOD_bkgnd_flag_t flag, TCOD_alignment_t alignment,const char *fmt, ...) {
	int ret;
	va_list ap;
	va_start(ap,fmt);
	ret = TCOD_console_print_internal(con,x,y,w,h,flag,alignment,TCOD_console_vsprint(fmt,ap), true, false);
	va_end(ap);
	return ret;
}

int TCOD_console_get_height_rect(TCOD_console_t con,int x, int y, int w, int h, const char *fmt, ...) {
	int ret;
	va_list ap;
	va_start(ap,fmt);
	ret = TCOD_console_print_internal(con,x,y,w,h,TCOD_BKGND_NONE,TCOD_LEFT,TCOD_console_vsprint(fmt,ap), true, true);
	va_end(ap);
	return ret;
}

/* non public methods */
int TCOD_console_stringLength(const unsigned char *s) {
	int l=0;
	while (*s) {
		if ( *s == (int)TCOD_COLCTRL_FORE_RGB || *s == (int)TCOD_COLCTRL_BACK_RGB ) s+=3;
		else if ( (unsigned)(*s) > (unsigned)TCOD_COLCTRL_STOP ) l++;
		s++;
	}
	return l;
}

unsigned char * TCOD_console_forward(unsigned char *s,int l) {
	while ( *s && l > 0 ) {
		if ( *s == (int)TCOD_COLCTRL_FORE_RGB || *s == (int)TCOD_COLCTRL_BACK_RGB ) s+=3;
		else if ( *s > (int)TCOD_COLCTRL_STOP ) l--;
		s++;
	}
	return s;
}

unsigned char *TCOD_console_strchr(unsigned char *s, unsigned char c) {
	while ( *s && *s != c ) {
		if ( *s == (int)TCOD_COLCTRL_FORE_RGB || *s == (int)TCOD_COLCTRL_BACK_RGB ) s+=3;
		s++;
	}
	return (*s ? s : NULL);
}

int TCOD_console_print_internal(TCOD_console_t con,int x,int y, int rw, int rh, TCOD_bkgnd_flag_t flag,
	TCOD_alignment_t align, char *msg, bool can_split, bool count_only) {
	unsigned char *c=(unsigned char *)msg;
	int cx=0,cy=y;
	int minx,maxx,miny,maxy;
	TCOD_color_t oldFore;
	TCOD_color_t oldBack;
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT ( dat != NULL
		&& (unsigned)(x) < (unsigned)dat->w && (unsigned)(y) < (unsigned)dat->h )
		return 0;
	TCOD_IFNOT(msg != NULL) return 0;
	if ( rh == 0 ) rh = dat->h-y;
	if ( rw == 0 ) switch(align) {
		case TCOD_LEFT : rw = dat->w-x; break;
		case TCOD_RIGHT : rw=x+1; break;
		case TCOD_CENTER : default : rw=dat->w; break;
	}
	oldFore=dat->fore;
	oldBack=dat->back;
	miny=y;
	maxy=dat->h-1;
	if (rh > 0) maxy=MIN(maxy,y+rh-1);
	switch (align) {
		case TCOD_LEFT : minx=MAX(0,x); maxx=MIN(dat->w-1,x+rw-1); break;
		case TCOD_RIGHT : minx=MAX(0,x-rw+1); maxx=MIN(dat->w-1,x); break;
		case TCOD_CENTER : default : minx=MAX(0,x-rw/2); maxx=MIN(dat->w-1,x+rw/2); break;
	}

	do {
		/* get \n delimited sub-message */
		unsigned char *end=TCOD_console_strchr(c,'\n');
		char bak=0;
		int cl;
		unsigned char *split=NULL;
		if ( end ) *end=0;
		cl= TCOD_console_stringLength(c);
		/* find starting x */
		switch (align) {
			case TCOD_LEFT : cx=x; break;
			case TCOD_RIGHT : cx=x-cl+1; break;
			case TCOD_CENTER : cx= x-cl/2;break;
		}
		/* check if the string is completely out of the minx,miny,maxx,maxy frame */
		if ( cy >= miny && cy <= maxy && cx <= maxx && cx+cl -1 >= minx ) {
			if ( can_split && cy < maxy ) {
				/* if partially out of screen, try to split the sub-message */
				if ( cx < minx ) split = TCOD_console_forward(c, align == TCOD_CENTER ? cl-2*(minx-cx) : cl-(minx-cx));
				else if ( align == TCOD_CENTER ) {
					if ( cx + cl/2 > maxx+1 ) split = TCOD_console_forward(c, maxx+1 - cx);
				} else {
					if ( cx + cl > maxx+1 ) split = TCOD_console_forward(c, maxx+1 - cx);
				}
			}
			if ( split ) {
				unsigned char *oldsplit=split;
				while ( ! isspace(*split) && split > c ) split --;
				if (end) *end='\n';
				if (!isspace(*split) ) {
					split=oldsplit;
				}
				end=split;
				bak=*split;
				*split=0;
				cl=TCOD_console_stringLength(c);
				switch (align) {
					case TCOD_LEFT : cx=x; break;
					case TCOD_RIGHT : cx=x-cl+1; break;
					case TCOD_CENTER : cx= x-cl/2;break;
				}
			}
			if ( cx < minx ) {
				/* truncate left part */
				c += minx-cx;
				cl -= minx-cx;
				cx=minx;
			}
			if ( cx + cl > maxx+1 ) {
				/* truncate right part */
				split = TCOD_console_forward(c, maxx+1 - cx);
				*split=0;
			}
			/* render the sub-message */
			if ( cy >= 0 && cy < dat->h )
			while (*c) {
				if ( *c >= TCOD_COLCTRL_1 && *c <= TCOD_COLCTRL_NUMBER ) {
					dat->fore=color_control_fore[(int)(*c)-1];
					dat->back=color_control_back[(int)(*c)-1];
				} else if ( *c == TCOD_COLCTRL_FORE_RGB ) {
					c++;
					dat->fore.r=*c++;
					dat->fore.g=*c++;
					dat->fore.b=*c;
				} else if ( *c == TCOD_COLCTRL_BACK_RGB ) {
					c++;
					dat->back.r=*c++;
					dat->back.g=*c++;
					dat->back.b=*c;
				} else if ( *c == TCOD_COLCTRL_STOP ) {
					dat->fore=oldFore;
					dat->back=oldBack;
				} else {
					if (! count_only) TCOD_console_put_char(con,cx,cy,(int)(*c),flag);
					cx++;
				}
				c++;
			}
		}
		if ( end ) {
			/* next line */
			if ( split && ! isspace(bak) ) {
				*end=bak;
				c=end;
			} else {
				c=end+1;
			}
			cy++;
		} else c=NULL;
	} while ( c && cy < dat->h && (rh == 0 || cy < y+rh) );
	return cy-y+1;
}

#ifndef NO_UNICODE

wchar_t *TCOD_console_strchr_utf(wchar_t *s, char c) {
	while ( *s && *s != c ) {
		if ( *s == (int)TCOD_COLCTRL_FORE_RGB || *s == (int)TCOD_COLCTRL_BACK_RGB ) s+=3;
		s++;
	}
	return (*s ? s : NULL);
}

void TCOD_console_map_string_to_font_utf(const wchar_t *s, int fontCharX, int fontCharY) {
	TCOD_IFNOT(s != NULL) return;
	while (*s) {
		TCOD_sys_map_ascii_to_font(*s, fontCharX, fontCharY);
		fontCharX++;
		if ( fontCharX == TCOD_ctx.fontNbCharHoriz ) {
			fontCharX=0;
			fontCharY++;
		}
		s++;
	}
}

wchar_t *TCOD_console_vsprint_utf(const wchar_t *fmt, va_list ap) {
	#define NB_BUFFERS 10
	#define INITIAL_SIZE 512
	/* several static buffers in case the function is used more than once in a single function call */
	static wchar_t *msg[NB_BUFFERS]={NULL};
	static int buflen[NB_BUFFERS]={0};
	static int curbuf=0;
	wchar_t *ret;
	bool ok=false;
	if (!msg[0]) {
		int i;
		for (i=0; i < NB_BUFFERS; i++) {
			buflen[i]=INITIAL_SIZE;
			msg[i]=(wchar_t *)calloc(sizeof(wchar_t),INITIAL_SIZE);
		}
	}
	do {
		/* warning ! depending on the compiler, vsnprintf return -1 or
		 the expected string length if the buffer is not big enough */
		int len = vsnwprintf(msg[curbuf],buflen[curbuf],fmt,ap);
		ok=true;
		if (len < 0 || len >= buflen[curbuf]) {
			/* buffer too small. */
			if ( len > 0 ) {
				while ( buflen[curbuf] < len+1 ) buflen[curbuf]*=2;
			} else {
				buflen[curbuf]*=2;
			}
			free( msg[curbuf] );
			msg[curbuf]=(wchar_t *)calloc(sizeof(wchar_t),buflen[curbuf]);
			ok=false;
		}
	} while (! ok);
	ret=msg[curbuf];
	curbuf = (curbuf+1)%NB_BUFFERS;
	return ret;
}


int TCOD_console_stringLength_utf(const wchar_t *s) {
	int l=0;
	while (*s) {
		if ( *s == (int)TCOD_COLCTRL_FORE_RGB || *s == (int)TCOD_COLCTRL_BACK_RGB ) s+=3;
		else if ( *s > (int)TCOD_COLCTRL_STOP ) l++;
		s++;
	}
	return l;
}

wchar_t * TCOD_console_forward_utf(wchar_t *s,int l) {
	while ( *s && l > 0 ) {
		if ( *s == (int)TCOD_COLCTRL_FORE_RGB || *s == (int)TCOD_COLCTRL_BACK_RGB ) s+=3;
		else if ( *s > (int)TCOD_COLCTRL_STOP ) l--;
		s++;
	}
	return s;
}

int TCOD_console_print_internal_utf(TCOD_console_t con,int x,int y, int rw, int rh, TCOD_bkgnd_flag_t flag,
	TCOD_alignment_t align, wchar_t *msg, bool can_split, bool count_only) {
	wchar_t *c=msg;
	int cx=0,cy=y;
	int minx,maxx,miny,maxy;
	TCOD_color_t oldFore;
	TCOD_color_t oldBack;
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT ( dat != NULL
		&& (unsigned)(x) < (unsigned)dat->w && (unsigned)(y) < (unsigned)dat->h )
		return 0;
	TCOD_IFNOT(msg != NULL) return 0;
	if ( rh == 0 ) rh = dat->h-y;
	if ( rw == 0 ) switch(align) {
		case TCOD_LEFT : rw = dat->w-x; break;
		case TCOD_RIGHT : rw=x+1; break;
		case TCOD_CENTER : default : rw=dat->w; break;
	}
	oldFore=dat->fore;
	oldBack=dat->back;
	miny=y;
	maxy=dat->h-1;
	if (rh > 0) maxy=MIN(maxy,y+rh-1);
	switch (align) {
		case TCOD_LEFT : minx=MAX(0,x); maxx=MIN(dat->w-1,x+rw-1); break;
		case TCOD_RIGHT : minx=MAX(0,x-rw+1); maxx=MIN(dat->w-1,x); break;
		case TCOD_CENTER : default : minx=MAX(0,x-rw/2); maxx=MIN(dat->w-1,x+rw/2); break;
	}

	do {
		/* get \n delimited sub-message */
		wchar_t *end=TCOD_console_strchr_utf(c,'\n');
		wchar_t bak=0;
		int cl;
		wchar_t *split=NULL;
		if ( end ) *end=0;
		cl= TCOD_console_stringLength_utf(c);
		/* find starting x */
		switch (align) {
			case TCOD_LEFT : cx=x; break;
			case TCOD_RIGHT : cx=x-cl+1; break;
			case TCOD_CENTER : cx= x-cl/2;break;
		}
		/* check if the string is completely out of the minx,miny,maxx,maxy frame */
		if ( cy >= miny && cy <= maxy && cx <= maxx && cx+cl -1 >= minx ) {
			if ( can_split && cy < maxy ) {
				/* if partially out of screen, try to split the sub-message */
				if ( cx < minx ) split = TCOD_console_forward_utf(c, align == TCOD_CENTER ? cl-2*(minx-cx) : cl-(minx-cx));
				else if ( align==TCOD_CENTER ) {
					if ( cx + cl/2 > maxx+1 ) split = TCOD_console_forward_utf(c, maxx+1 - cx);
				} else {
					if ( cx + cl > maxx+1 ) split = TCOD_console_forward_utf(c, maxx+1 - cx);
				}
			}
			if ( split ) {
				wchar_t *oldsplit=split;
				while ( ! iswspace(*split) && split > c ) split --;
				if (end) *end='\n';
				if (!iswspace(*split) ) {
					split=oldsplit;
				}
				end=split;
				bak=*split;
				*split=0;
				cl=TCOD_console_stringLength_utf(c);
				switch (align) {
					case TCOD_LEFT : cx=x; break;
					case TCOD_RIGHT : cx=x-cl+1; break;
					case TCOD_CENTER : cx= x-cl/2;break;
				}
			}
			if ( cx < minx ) {
				/* truncate left part */
				c += minx-cx;
				cl -= minx-cx;
				cx=minx;
			}
			if ( cx + cl > maxx+1 ) {
				/* truncate right part */
				split = TCOD_console_forward_utf(c, maxx+1 - cx);
				*split=0;
			}
			/* render the sub-message */
			if ( cy >= 0 && cy < dat->h )
			while (*c) {
				if ( *c >= TCOD_COLCTRL_1 && *c <= TCOD_COLCTRL_NUMBER ) {
					dat->fore=color_control_fore[(int)(*c)-1];
					dat->back=color_control_back[(int)(*c)-1];
				} else if ( *c == TCOD_COLCTRL_FORE_RGB ) {
					c++;
					dat->fore.r=(uint8)(*c++);
					dat->fore.g=(uint8)(*c++);
					dat->fore.b=(uint8)(*c);
				} else if ( *c == TCOD_COLCTRL_BACK_RGB ) {
					c++;
					dat->back.r=(uint8)(*c++);
					dat->back.g=(uint8)(*c++);
					dat->back.b=(uint8)(*c);
				} else if ( *c == TCOD_COLCTRL_STOP ) {
					dat->fore=oldFore;
					dat->back=oldBack;
				} else {
					if (! count_only) TCOD_console_put_char(con,cx,cy,(int)(*c),flag);
					cx++;
				}
				c++;
			}
		}
		if ( end ) {
			/* next line */
			if ( split && ! iswspace(bak) ) {
				*end=bak;
				c=end;
			} else {
				c=end+1;
			}
			cy++;
		} else c=NULL;
	} while ( c && cy < dat->h && (rh == 0 || cy < y+rh) );
	return cy-y+1;
}

void TCOD_console_print_utf(TCOD_console_t con,int x, int y, const wchar_t *fmt, ...) {
	va_list ap;
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT ( dat != NULL ) return;
	va_start(ap,fmt);
	TCOD_console_print_internal_utf(con,x,y,0,0,dat->bkgnd_flag,dat->alignment,
		TCOD_console_vsprint_utf(fmt,ap), false, false);
	va_end(ap);
}

void TCOD_console_print_ex_utf(TCOD_console_t con,int x, int y,
	TCOD_bkgnd_flag_t flag, TCOD_alignment_t alignment, const wchar_t *fmt, ...) {
	va_list ap;
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT ( dat != NULL ) return;
	va_start(ap,fmt);
	TCOD_console_print_internal_utf(con,x,y,0,0,flag,alignment,TCOD_console_vsprint_utf(fmt,ap), false, false);
	va_end(ap);
}

int TCOD_console_print_rect_utf(TCOD_console_t con,int x, int y, int w, int h,
	const wchar_t *fmt, ...) {
	int ret;
	va_list ap;
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT ( dat != NULL ) return 0;
	va_start(ap,fmt);
	ret = TCOD_console_print_internal_utf(con,x,y,w,h,dat->bkgnd_flag,dat->alignment,
		TCOD_console_vsprint_utf(fmt,ap), true, false);
	va_end(ap);
	return ret;
}

int TCOD_console_print_rect_ex_utf(TCOD_console_t con,int x, int y, int w, int h,
	TCOD_bkgnd_flag_t flag, TCOD_alignment_t alignment, const wchar_t *fmt, ...) {
	int ret;
	va_list ap;
	va_start(ap,fmt);
	ret=TCOD_console_print_internal_utf(con,x,y,w,h,flag,alignment,TCOD_console_vsprint_utf(fmt,ap), true, false);
	va_end(ap);
	return ret;
}

int TCOD_console_get_height_rect_utf(TCOD_console_t con,int x, int y, int w, int h, const wchar_t *fmt, ...) {
	int ret;
	va_list ap;
	va_start(ap,fmt);
	ret = TCOD_console_print_internal_utf(con,x,y,w,h,TCOD_BKGND_NONE,TCOD_LEFT,TCOD_console_vsprint_utf(fmt,ap), true, true);
	va_end(ap);
	return ret;
}

#endif

void TCOD_console_init_root(int w, int h, const char*title, bool fullscreen, TCOD_renderer_t renderer) {
	TCOD_IF(w > 0 && h > 0) {
		TCOD_console_data_t *con=(TCOD_console_data_t *)calloc(sizeof(TCOD_console_data_t),1);
		int i;
		con->w=w;
		con->h=h;
		TCOD_ctx.root=con;
		TCOD_ctx.renderer=renderer;
		for (i=0; i < TCOD_COLCTRL_NUMBER; i++) {
			color_control_fore[i]=TCOD_white;
			color_control_back[i]=TCOD_black;
		}
		TCOD_console_init((TCOD_console_t)con,title,fullscreen);
	}
}

bool TCOD_console_init(TCOD_console_t con,const char *title, bool fullscreen) {
	int i;
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT(dat != NULL) return false;
	dat->fore=TCOD_white;
	dat->back=TCOD_black;
	dat->fade=255;
	dat->buf = (char_t *)calloc(sizeof(char_t),dat->w*dat->h);
	dat->oldbuf = (char_t *)calloc(sizeof(char_t),dat->w*dat->h);
	dat->bkgnd_flag=TCOD_BKGND_NONE;
	dat->alignment=TCOD_LEFT;
	for (i=0; i< dat->w*dat->h; i++) {
		dat->buf[i].c=' ';
		dat->buf[i].cf=-1;
	}
	if ( title ) {
		if (! TCOD_sys_init(dat->w,dat->h,dat->buf,dat->oldbuf,fullscreen) ) return false;
		TCOD_sys_set_window_title(title);
	}
	return true;
}

void TCOD_console_set_default_foreground(TCOD_console_t con,TCOD_color_t col) {
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT(dat != NULL) return;
	dat->fore=col;
}

void TCOD_console_set_default_background(TCOD_console_t con,TCOD_color_t col) {
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT(dat != NULL) return;
	dat->back=col;
}

TCOD_color_t TCOD_console_get_default_foreground(TCOD_console_t con) {
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT(dat != NULL) return TCOD_white;
	return dat->fore;
}

TCOD_color_t TCOD_console_get_default_background(TCOD_console_t con) {
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT(dat != NULL) return TCOD_black;
	return dat->back;
}

int TCOD_console_get_width(TCOD_console_t con) {
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT(dat != NULL) return 0;
	return dat->w;
}

int TCOD_console_get_height(TCOD_console_t con) {
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT(dat != NULL) return 0;
	return dat->h;
}

char_t *TCOD_console_get_buf(TCOD_console_t con) {
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT(dat != NULL) return NULL;
	return dat->buf;
}

void TCOD_console_set_custom_font(const char *fontFile, int flags,int nb_char_horiz, int nb_char_vertic) {
	TCOD_sys_set_custom_font(fontFile, nb_char_horiz, nb_char_vertic, flags);
}

void TCOD_console_map_ascii_code_to_font(int asciiCode, int fontCharX, int fontCharY) {
	/* cannot change mapping before initRoot is called */
	TCOD_IFNOT(TCOD_ctx.root != NULL) return;
	TCOD_sys_map_ascii_to_font(asciiCode, fontCharX, fontCharY);
}

void TCOD_console_map_ascii_codes_to_font(int asciiCode, int nbCodes, int fontCharX, int fontCharY) {
	int c;
	/* cannot change mapping before initRoot is called */
	TCOD_IFNOT(TCOD_ctx.root != NULL) return;
	TCOD_IFNOT(asciiCode >= 0 && asciiCode+nbCodes <= TCOD_ctx.max_font_chars) return;
	for (c=asciiCode; c < asciiCode+nbCodes; c++ ) {
		TCOD_sys_map_ascii_to_font(c, fontCharX, fontCharY);
		fontCharX++;
		if ( fontCharX == TCOD_ctx.fontNbCharHoriz ) {
			fontCharX=0;
			fontCharY++;
		}
	}
}

void TCOD_console_map_string_to_font(const char *s, int fontCharX, int fontCharY) {
	TCOD_IFNOT(s != NULL) return;
	/* cannot change mapping before initRoot is called */
	TCOD_IFNOT(TCOD_ctx.root != NULL) return;
	while (*s) {
		TCOD_console_map_ascii_code_to_font(*s, fontCharX, fontCharY);
		fontCharX++;
		if ( fontCharX == TCOD_ctx.fontNbCharHoriz ) {
			fontCharX=0;
			fontCharY++;
		}
		s++;
	}
}

void TCOD_console_set_keyboard_repeat(int initial_delay, int interval) {
	TCOD_sys_set_keyboard_repeat(initial_delay,interval);
}

void TCOD_console_disable_keyboard_repeat() {
	TCOD_sys_set_keyboard_repeat(0,0);
}

bool TCOD_console_is_key_pressed(TCOD_keycode_t key) {
	return TCOD_sys_is_key_pressed(key);
}
void TCOD_console_set_key_color(TCOD_console_t con,TCOD_color_t col) {
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT(dat != NULL) return;
	dat->key = col;
	dat->haskey=true;
}

void TCOD_console_credits() {
	bool end=false;
	int x=TCOD_console_get_width(NULL)/2-6;
	int y=TCOD_console_get_height(NULL)/2;
	int fade=260;
	TCOD_sys_save_fps();
	TCOD_sys_set_fps(25);
	while (!end ) {
		TCOD_key_t k;
		end=TCOD_console_credits_render(x,y,false);
		TCOD_sys_check_for_event(TCOD_EVENT_KEY_PRESS,&k,NULL);
		if ( fade == 260 && k.vk != TCODK_NONE ) {
			fade -= 10;
		}
		TCOD_console_flush();
		if ( fade < 260 ) {
			fade -= 10;
			TCOD_console_set_fade(fade,TCOD_black);
			if ( fade == 0 ) end=true;
		}
	}
	TCOD_console_set_fade(255,TCOD_black);
	TCOD_sys_restore_fps();
}

static bool init2=false;

void TCOD_console_credits_reset() {
	init2=false;
}

bool TCOD_console_credits_render(int x, int y, bool alpha) {
	static char poweredby[128];
	static float char_heat[128];
	static int char_x[128];
	static int char_y[128];
	static bool init1=false;
	static int len,len1,cw=-1,ch=-1;
	static float xstr;
	static TCOD_color_t colmap[64];
	static TCOD_color_t colmap_light[64];
	static TCOD_noise_t noise;
	static TCOD_color_t colkeys[4] = {
		{255,255,204},
		{255,204,0},
		{255,102,0},
		{102,153,255},
	};
	static TCOD_color_t colkeys_light[4] = {
		{255,255,204},
		{128,128,77},
		{51,51,31},
		{0,0,0},
	};
	static int colpos[4]={
		0,21,42,63
	};
	static TCOD_image_t img=NULL;
	int i,xc,yc,xi,yi,j;
	static int left,right,top,bottom;
	float sparklex,sparkley,sparklerad,sparklerad2,noisex;
	/* mini particule system */
#define MAX_PARTICULES 50
	static float pheat[MAX_PARTICULES];
	static float px[MAX_PARTICULES],py[MAX_PARTICULES], pvx[MAX_PARTICULES],pvy[MAX_PARTICULES];
	static int nbpart=0, firstpart=0;
	static float partDelay=0.1f;
	float elapsed=TCOD_sys_get_last_frame_length();
	TCOD_color_t fbackup; /* backup fg color */

	if (!init1) {
		/* initialize all static data, colormaps, ... */
		TCOD_color_t col;
		TCOD_color_gen_map(colmap,4,colkeys,colpos);
		TCOD_color_gen_map(colmap_light,4,colkeys_light,colpos);
		sprintf(poweredby,"Powered by\n%s",version_string);
		noise=TCOD_noise_new(1,TCOD_NOISE_DEFAULT_HURST,TCOD_NOISE_DEFAULT_LACUNARITY,NULL);
		len=strlen(poweredby);
		len1=11; /* sizeof "Powered by\n" */
		left=MAX(x-4,0);
		top=MAX(y-4,0);
		col= TCOD_console_get_default_background(NULL);
		TCOD_console_set_default_background(NULL,TCOD_black);
		TCOD_console_set_default_background(NULL,col);
		init1=true;
	}
	if (!init2) {
		/* reset the credits vars ... */
		int curx,cury;
		xstr=-4.0f;
		curx=x;
		cury=y;
		for (i=0; i < len ;i++) {
			char_heat[i]=-1;
			char_x[i]=curx;
			char_y[i]=cury;
			curx++;
			if ( poweredby[i] == '\n' ) {
				curx=x;
				cury++;
			}
		}
		nbpart=firstpart=0;
		init2=true;
	}
	if (TCOD_console_get_width(NULL) != cw || TCOD_console_get_height(NULL)!=ch) {
		/* console size has changed */
		int width,height;
		cw=TCOD_console_get_width(NULL);
		ch=TCOD_console_get_height(NULL);
		right=MIN(x+len,cw-1);
		bottom=MIN(y+6,ch-1);
		width=right - left + 1;
		height=bottom - top + 1;
		if ( img ) TCOD_image_delete(img);
		img = TCOD_image_new(width*2,height*2);
	}
	fbackup=TCOD_console_get_default_foreground(NULL);
	if ( xstr < (float)len1 ) {
		sparklex=x+xstr;
		sparkley=(float)y;
	} else {
		sparklex=x-len1+xstr;
		sparkley=(float)y+1;
	}
	noisex=xstr*6;
	sparklerad=3.0f+2*TCOD_noise_get(noise,&noisex);
	if ( xstr >= len-1 ) sparklerad -= (xstr-len+1)*4.0f;
	else if ( xstr < 0.0f ) sparklerad += xstr*4.0f;
	else if ( poweredby[ (int)(xstr+0.5f) ] == ' ' || poweredby[ (int)(xstr+0.5f) ] == '\n' ) sparklerad/=2;
	sparklerad2=sparklerad*sparklerad*4;

	/* draw the light */
	for (xc=left*2,xi=0; xc < (right+1)*2; xc++,xi++) {
		for (yc=top*2,yi=0; yc < (bottom+1)*2; yc++,yi++) {
			float dist=((xc-2*sparklex)*(xc-2*sparklex)+(yc-2*sparkley)*(yc-2*sparkley));
			TCOD_color_t pixcol;
			if ( sparklerad >= 0.0f && dist < sparklerad2 ) {
				int colidx=63-(int)(63*(sparklerad2-dist)/sparklerad2) + TCOD_random_get_int(NULL,-10,10);
				colidx=CLAMP(0,63,colidx);
				pixcol=colmap_light[colidx];
			} else {
				pixcol=TCOD_black;
			}
			if ( alpha ) {
				/*	console cells have following flag values :
				  		1 2
				  		4 8
				  	flag indicates which subcell uses foreground color */
				static int asciiToFlag[] = {
					1, /* TCOD_CHAR_SUBP_NW */
					2, /* TCOD_CHAR_SUBP_NE */
					3, /* TCOD_CHAR_SUBP_N */
					8, /* TCOD_CHAR_SUBP_SE */
					9, /* TCOD_CHAR_SUBP_DIAG */
					10, /* TCOD_CHAR_SUBP_E */
					4, /* TCOD_CHAR_SUBP_SW */
				};
				int conc= TCOD_console_get_char(NULL,xc/2,yc/2);
				TCOD_color_t bk=TCOD_console_get_char_background(NULL,xc/2,yc/2);
				if ( conc >= TCOD_CHAR_SUBP_NW && conc <= TCOD_CHAR_SUBP_SW ) {
					/* merge two subcell chars...
					   get the flag for the existing cell on root console */
					int bkflag=asciiToFlag[conc - TCOD_CHAR_SUBP_NW ];
					int xflag = (xc & 1);
					int yflag = (yc & 1);
					/* get the flag for the current subcell */
					int credflag = (1+3*yflag) * (xflag+1);
					if ( (credflag & bkflag) != 0 ) {
						/* the color for this subcell on root console
						   is foreground, not background */
						bk = TCOD_console_get_char_foreground(NULL,xc/2,yc/2);
					}
				}
				pixcol.r = MIN(255,(int)(bk.r)+pixcol.r);
				pixcol.g = MIN(255,(int)(bk.g)+pixcol.g);
				pixcol.b = MIN(255,(int)(bk.b)+pixcol.b);
			}
			TCOD_image_put_pixel(img,xi,yi,pixcol);
		}
	}

	/* draw and update the particules */
	j=nbpart;i=firstpart;
	while (j > 0) {
		int colidx=(int)(64*(1.0f-pheat[i]));
		TCOD_color_t col;
		colidx=MIN(63,colidx);
		col=colmap[colidx];
		if ( (int)py[i]< (bottom-top+1)*2 ) {
			int ipx = (int)px[i];
			int ipy = (int)py[i];
			float fpx = px[i]-ipx;
			float fpy = py[i]-ipy;
			TCOD_color_t col2=TCOD_image_get_pixel(img,ipx,ipy);
			col2=TCOD_color_lerp(col,col2,0.5f*(fpx+fpy));
			TCOD_image_put_pixel(img,ipx,ipy,col2);
			col2=TCOD_image_get_pixel(img,ipx+1,ipy);
			col2=TCOD_color_lerp(col2,col,fpx);
			TCOD_image_put_pixel(img,ipx+1,ipy,col2);
			col2=TCOD_image_get_pixel(img,ipx,ipy+1);
			col2=TCOD_color_lerp(col2,col,fpy);
			TCOD_image_put_pixel(img,ipx,ipy+1,col2);
		} else pvy[i]=-pvy[i] * 0.5f;
		pvx[i] *= (1.0f-elapsed);
		pvy[i] += (1.0f-pheat[i])*elapsed*300.0f;
		px[i] += pvx[i]*elapsed;
		py[i] += pvy[i]*elapsed;
		pheat[i] -= elapsed*0.3f;
		if ( pheat[i] < 0.0f ) {
			firstpart = (firstpart+1)%MAX_PARTICULES;
			nbpart--;
		}
		i = (i+1)%MAX_PARTICULES;
		j--;
	}
	partDelay -= elapsed;
	if ( partDelay < 0.0f && nbpart < MAX_PARTICULES && sparklerad > 2.0f ) {
		/* fire a new particule */
		int lastpart = firstpart;
		int nb=nbpart;
		while (nb > 0 ) {
			lastpart = ( lastpart + 1 )%MAX_PARTICULES;
			nb--;
		}
		nbpart++;
		px[lastpart] = 2*(sparklex-left);
		py[lastpart] = 2*(sparkley-top)+2;
		pvx[lastpart] = TCOD_random_get_float(NULL,-5.0f,5.0f);
		pvy[lastpart] = TCOD_random_get_float(NULL,-0.5f, -15.0f);
		pheat[lastpart] = 1.0f;
		partDelay += 0.1f;
	}
	TCOD_image_blit_2x(img,NULL,left,top,0,0,-1,-1);
	/* draw the text */
	for (i=0; i < len ;i++) {
		if ( char_heat[i] >= 0.0f && poweredby[i]!='\n') {
			int colidx=(int)(64*char_heat[i]);
			TCOD_color_t col;
			colidx=MIN(63,colidx);
			col=colmap[colidx];
			if ( xstr >= len  ) {
				float coef=(xstr-len)/len;
				if ( alpha ) {
					TCOD_color_t fore=TCOD_console_get_char_background(NULL,char_x[i],char_y[i]);
					int r=(int)(coef*fore.r + (1.0f-coef)*col.r);
					int g=(int)(coef*fore.g + (1.0f-coef)*col.g);
					int b=(int)(coef*fore.b + (1.0f-coef)*col.b);
					col.r = CLAMP(0,255,r);
					col.g = CLAMP(0,255,g);
					col.b = CLAMP(0,255,b);
					TCOD_console_set_char_foreground(NULL,char_x[i],char_y[i],col);
				} else {
					col=TCOD_color_lerp(col,TCOD_black,coef);
				}
			}
			TCOD_console_set_char(NULL,char_x[i],char_y[i],poweredby[i]);
			TCOD_console_set_char_foreground(NULL,char_x[i],char_y[i],col);
		}
	}
	/* update letters heat */
	xstr += elapsed * 4;
	for (i=0; i < (int)(xstr+0.5f); i++) {
		char_heat[i]=(xstr-i)/(len/2);
	}
	/* restore fg color */
	TCOD_console_set_default_foreground(NULL,fbackup);
	if ( xstr <= 2*len ) return false;
	init2=false;
	return true;
}

static void TCOD_console_read_asc(TCOD_console_t con,FILE *f,int width, int height, float version) {
	int x,y;
	TCOD_console_data_t *dat=con ? (TCOD_console_data_t *)con : TCOD_ctx.root;
	TCOD_IFNOT(dat != NULL) return;
	while(fgetc(f) != '#');
	for(x = 0; x < width; x++) {
	    for(y = 0; y < height; y++) {
	    	TCOD_color_t fore,back;
		    int c = fgetc(f);
		    fore.r = fgetc(f);
		    fore.g = fgetc(f);
		    fore.b = fgetc(f);
		    back.r = fgetc(f);
		    back.g = fgetc(f);
		    back.b = fgetc(f);
		    /* skip solid/walkable info */
		    if ( version >= 0.3f ) {
		    	fgetc(f); 
		    	fgetc(f);
		    }
		    TCOD_console_put_char_ex(con,x,y,c,fore,back);
	    }
    }
    fclose(f);
}

static void TCOD_console_read_apf(TCOD_console_t con,FILE *f,int width, int height, float version) {
}

TCOD_console_t TCOD_console_from_file(const char *filename) {
	float version;
	int width,height;
	TCOD_console_t con;
	FILE *f;
	TCOD_IFNOT( filename != NULL ) {
		return NULL;
	}
	f=fopen(filename,"rb");
	TCOD_IFNOT( f!=NULL ) {
		return NULL;
	}
	if (fscanf(f, "ASCII-Paint v%g", &version) != 1 ) {
		fclose(f);
		return NULL;
	}
	if (fscanf(f, "%i %i", &width, &height) != 2 ) {
		fclose(f);
		return NULL;
	}
	TCOD_IFNOT ( width > 0 && height > 0) {
		fclose(f);
		return NULL;
	}
	con=TCOD_console_new(width,height);
	if ( strstr(filename,".asc") )
		TCOD_console_read_asc(con,f,width,height,version);
	else TCOD_console_read_apf(con,f,width,height,version);
	return con;
}

bool TCOD_console_load_asc(TCOD_console_t pcon, const char *filename) {
	float version;
	int width,height;
	FILE *f;
	TCOD_console_data_t *con=pcon ? (TCOD_console_data_t *)pcon : TCOD_ctx.root;
	TCOD_IFNOT(con != NULL) return false;
	TCOD_IFNOT( filename != NULL ) {
		return false;
	}
	f=fopen(filename,"rb");
	TCOD_IFNOT( f!=NULL ) {
		return false;
	}
	if (fscanf(f, "ASCII-Paint v%g", &version) != 1 ) {
		fclose(f);
		return false;
	}
	if (fscanf(f, "%i %i", &width, &height) != 2 ) {
		fclose(f);
		return false;
	}
	TCOD_IFNOT ( width > 0 && height > 0) {
		fclose(f);
		return false;
	}
	if ( con->w != width || con->h != height ) {
		/* resize console */
		if (con->buf) free(con->buf);
		if (con->oldbuf) free(con->oldbuf);
		con->buf = (char_t *)calloc(sizeof(char_t),width*height);
		con->oldbuf = (char_t *)calloc(sizeof(char_t),width*height);
		con->w=width;
		con->h=height;
	}
	TCOD_console_read_asc(con,f,width,height,version);
	return true;
}

bool TCOD_console_save_asc(TCOD_console_t pcon, const char *filename) {
	static float version = 0.3f;
	FILE *f;
	int x,y;
	TCOD_console_data_t *con=pcon ? (TCOD_console_data_t *)pcon : TCOD_ctx.root;
	TCOD_IFNOT(con != NULL) return false;
	TCOD_IFNOT( filename != NULL ) {
		return false;
	}
	TCOD_IFNOT(con->w > 0 && con->h > 0) return false;
	f=fopen(filename,"wb");
	TCOD_IFNOT( f != NULL ) return false;
	fprintf(f, "ASCII-Paint v%g\n", version);
	fprintf(f, "%i %i\n", con->w, con->h);
	fputc('#', f);
	for(x = 0; x < con->w; x++) {
		for(y = 0; y < con->h; y++) {
			TCOD_color_t fore,back;
			int c=TCOD_console_get_char(con,x,y);
			fore=TCOD_console_get_char_foreground(con,x,y);			
			back=TCOD_console_get_char_background(con,x,y);
			fputc(c, f);
			fputc(fore.r,f);			
			fputc(fore.g,f);			
			fputc(fore.b,f);			
			fputc(back.r,f);			
			fputc(back.g,f);			
			fputc(back.b,f);
			fputc(0,f); /* solid */
			fputc(1,f); /* walkable */			
		}
	}
	fclose(f);
	return true;
}

static bool hasDetectedBigEndianness = false;
static bool isBigEndian;
void detectBigEndianness(){
	if (!hasDetectedBigEndianness){
		uint32 Value32;
		uint8 *VPtr = (uint8 *)&Value32;
		VPtr[0] = VPtr[1] = VPtr[2] = 0; VPtr[3] = 1;
		if(Value32 == 1) isBigEndian = true;
		else isBigEndian = false;
		hasDetectedBigEndianness = true;
	}
}

uint16 bswap16(uint16 s){
	uint8* ps = (uint8*)&s;
	uint16 res;
	uint8* pres = (uint8*)&res;
	pres[0] = ps[1];
	pres[1] = ps[0];
	return res;
}

uint32 bswap32(uint32 s){
	uint8 *ps=(uint8 *)(&s);
	uint32 res;
	uint8 *pres=(uint8 *)&res;
	pres[0]=ps[3];
	pres[1]=ps[2];
	pres[2]=ps[1];
	pres[3]=ps[0];
	return res;
}

uint16 l16(uint16 s){
	if (isBigEndian) return bswap16(s); else return s;
}

uint32 l32(uint32 s){
	if (isBigEndian) return bswap32(s); else return s;
}

/* fix the endianness */
void fix16(uint16* u){
	*u = l16(*u);
}

void fix32(uint32* u){
	*u = l32(*u);
}

/************ RIFF helpers  */

uint32 fourCC(const char* c){
	return (*(uint32*)c);
}

/* checks if u equals str */
bool fourCCequals(uint32 u, const char* str){
	return fourCC(str)==u;
}

void fromFourCC(uint32 u, char*s){
	const char* c = (const char*)(&u);
	s[0]=c[0];
	s[1]=c[1];
	s[2]=c[2];
	s[3]=c[3];
	s[4]=0;
}

void put8(uint8 d, FILE* fp){
	fwrite(&d,1,1,fp);
}

void put16(uint16 d, FILE* fp){
	fwrite(&d,2,1,fp);
}

void put32(uint32 d, FILE* fp){
	fwrite(&d,4,1,fp);
}

void putFourCC(const char* c, FILE* fp){
	put32(fourCC(c),fp);
}

void putData(void* what, int length, FILE* fp){
	fwrite(what,length,1,fp);
}

bool get8(uint8* u, FILE* fp){
	return 1==fread((void*)u, sizeof(uint8),1,fp);
}

bool get16(uint16* u, FILE* fp){
	return 1==fread((void*)u, sizeof(uint16),1,fp);
}

bool get32(uint32* u, FILE* fp){
	return 1==fread((void*)u, sizeof(uint32),1,fp);
}

bool getData(void* u, size_t sz, FILE* fp){
	return 1==fread(u, sz,1,fp);
}


/********* APF RIFF structures */

typedef struct {
	uint32 show_grid;
	uint32 grid_width;
	uint32 grid_height;
} SettingsDataV1;

#define FILTER_TYPE_UNCOMPRESSED 0
#define FORMAT_TYPE_CRGBRGB 0

typedef struct {
	uint32 width;
	uint32 height;
	uint32 filter;
	uint32 format;
} ImageDetailsV1;

/* Layers */

typedef struct {
	uint32 name;
	uint32 mode;
	uint32 index;
	uint32 dataSize;
} LayerV1 ;

typedef struct {
	uint32 name;
	uint32 mode;
	uint32 fgalpha;
	uint32 bgalpha;
	uint32 visible;
	uint32 index;
	uint32 dataSize;
} LayerV2;

/* fix the endianness */
void fixSettings(SettingsDataV1* s){
	fix32(&s->show_grid);
	fix32(&s->grid_width);
	fix32(&s->grid_height);
}

void fixImage(ImageDetailsV1* v){
	fix32(&v->width);
	fix32(&v->height);
	fix32(&v->filter);
	fix32(&v->format);
}

void fixLayerv1(LayerV1* l){
	fix32(&l->mode);
	fix32(&l->index);
	fix32(&l->dataSize);
}

void fixLayerv2(LayerV2* l){
	fix32(&l->mode);
	fix32(&l->fgalpha);
	fix32(&l->bgalpha);
	fix32(&l->visible);
	fix32(&l->index);
	fix32(&l->dataSize);
}


/*********** ApfFile */

bool TCOD_console_save_apf(TCOD_console_t pcon, const char *filename) {
	TCOD_console_data_t *con=pcon ? (TCOD_console_data_t *)pcon : TCOD_ctx.root;
	FILE* fp ;
	TCOD_IFNOT(con != NULL) return false;
	detectBigEndianness();

	fp = fopen(filename, "wb");
	if(fp == NULL) {
		return false;
	}
	else {
		int x,y;
		uint32 riffSize = 0;
		uint32 imgDetailsSize ;
		SettingsDataV1 settingsData;
		ImageDetailsV1 imgData;
		fpos_t posRiffSize;
		uint32 settingsSz ;
		uint32 layerImageSize ;
		uint32 layerChunkSize ;
		/*  riff header*/
		putFourCC("RIFF",fp);
		fgetpos(fp,&posRiffSize);
		put32(0,fp); 

			/* APF_ header */
			putFourCC("apf ",fp);
			riffSize += 4;

				/* settings */
				settingsData.show_grid = 0;
				settingsData.grid_width = 8;
				settingsData.grid_height = 8;
				settingsSz = sizeof(uint32) + sizeof settingsData;
				putFourCC("sett",fp);
				put32(l32(settingsSz),fp);
				put32(l32(1),fp);
				putData((void*)&settingsData,sizeof settingsData,fp);
				if (settingsSz&1){
					put8(0,fp);
					riffSize++;
				}
				riffSize += 4+4+settingsSz;

				/* image details */
				imgData.width = con->w;
				imgData.height = con->h;
				imgData.filter = 0;
				imgData.format = 0;
				imgDetailsSize = sizeof(uint32) + sizeof imgData;
				putFourCC("imgd",fp);
				put32(l32(imgDetailsSize),fp);
				put32(l32(1),fp); 
				putData((void*)&imgData,sizeof imgData,fp);
				if (imgDetailsSize&1){
					put8(0,fp);
					riffSize++;
				}
				riffSize += 4+4+imgDetailsSize;

				/* now write the layers as a RIFF list
				   the first layer is the lowest layer
				   Assume imgData filter = uncompressed, and imgData format = CRGB */
				layerImageSize = imgData.width*imgData.height*7;
				layerChunkSize = sizeof(uint32) /* version */
						+ sizeof(LayerV2) /* header */
						+ layerImageSize; /* data */

				putFourCC("layr",fp); /* layer */
				put32(l32(layerChunkSize),fp);
					/* VERSION -> */
					put32(l32(2),fp);
					/* Data */
					putFourCC("LAY0",fp);
					put32(l32(0),fp);
					put32(l32(255),fp);
					put32(l32(255),fp);
					put32(l32(1),fp);
					put32(l32(0),fp);
					put32(l32(layerImageSize),fp);

					/* now write out the data */

					for(x = 0; x < con->w; x++) {
						for(y = 0; y < con->h; y++) {
							TCOD_color_t fore,back;
							int c=TCOD_console_get_char(con,x,y);
							fore=TCOD_console_get_char_foreground(con,x,y);			
							back=TCOD_console_get_char_background(con,x,y);
							put8(c, fp);
							put8(fore.r,fp);			
							put8(fore.g,fp);			
							put8(fore.b,fp);			
							put8(back.r,fp);			
							put8(back.g,fp);			
							put8(back.b,fp);
						}
					}

					if (layerChunkSize&1){
						put8(0,fp); /* padding bit */
						riffSize++;
					}

				riffSize += 2*sizeof(uint32)+layerChunkSize;

		fsetpos(fp,&posRiffSize);
		put32(l32(riffSize),fp);
	}

	fclose(fp);
	return true;
}

typedef struct {
	LayerV1 headerv1;
	LayerV2 headerv2;
	uint8* data; /* dynamically allocated */
}  LayerData;

typedef struct {
	ImageDetailsV1 details;
	SettingsDataV1 settings;
	LayerData layer;
} Data;

bool TCOD_console_load_apf(TCOD_console_t pcon, const char *filename) {
	uint32 sett = fourCC("sett");
	uint32 imgd = fourCC("imgd");
	/*
	uint32 LIST = fourCC("LIST");
	uint32 LAYR = fourCC("LAYR");
	*/
	uint32 layr = fourCC("layr");
	FILE* fp ;
	Data data; 
	TCOD_console_data_t *con=pcon ? (TCOD_console_data_t *)pcon : TCOD_ctx.root;
	TCOD_IFNOT(con != NULL) return false;

	detectBigEndianness();
	data.details.width = 1;
	data.details.height = 1;
	data.details.filter = 0;
	data.details.format = 0;

	data.settings.show_grid = true;
	data.settings.grid_width = 10;
	data.settings.grid_height = 10;

	#define ERR(x) {printf("Error: %s\n. Aborting operation.",x); return false;}
	#define ERR_NEWER(x) {printf("Error: It looks like this file was made with a newer version of Ascii-Paint\n. In particular the %s field. Aborting operation.",x); return false;}

	fp = fopen(filename, "rb");
	if(fp == NULL) {
		printf("The file %s could not be loaded.\n", filename);
		return false;
	}
	else {
		/* read the header */
		uint32 riff;
		uint32 riffSize;
		int index = 0;
		int x,y;
		uint8 *imgData;
		bool keepGoing = true;
		if (! get32(&riff,fp) || ! fourCCequals(riff,"RIFF")){
			ERR("File doesn't have a RIFF header");
		}
		if (!get32(&riffSize,fp)) ERR("No RIFF size field!");
		fix32(&riffSize);

		while(keepGoing && fp){ /* for each subfield, try to find the APF_ field */
			uint32 apf;
			if (! get32(&apf,fp)) break;
			if (fourCCequals(apf,"apf ") || fourCCequals(apf,"APF ")){
				/* Process APF segment */
				while(keepGoing && fp){
					uint32 seg;
					if (! get32(&seg,fp)){
						keepGoing = false;
						break;
					}
					else {
						if (seg==sett){
							/* size */
							uint32 sz;
							uint32 ver;
							SettingsDataV1 settingsData;

							get32(&sz,fp);
							fix32(&sz);
							/* version */
							get32(&ver,fp);
							fix32(&ver);
							if (ver!=1) ERR_NEWER("settings");
							/* ver must be 1 */
							if (! getData((void*)&settingsData,sizeof settingsData,fp)) ERR("Can't read settings.");
							data.settings = settingsData;
							fixSettings(&data.settings);

						}
						else if (seg==imgd){
							/* sz */
							uint32 sz;
							uint32 ver;
							ImageDetailsV1 dets;

							get32(&sz,fp);
							fix32(&sz);
							/* version */
							get32(&ver,fp);
							fix32(&ver);
							if (ver!=1) ERR_NEWER("image details");
							/* ver must be 1 */
							if (! getData((void*)&dets, sizeof dets, fp)) ERR("Can't read image details.");
							data.details = dets;
							fixImage(&data.details);

							/* get canvas ready */
							TCOD_IFNOT ( data.details.width > 0 && data.details.height > 0) {
								fclose(fp);
								return false;
							}
							if ( con->w != data.details.width || con->h != data.details.height ) {
								/* resize console */
								if (con->buf) free(con->buf);
								if (con->oldbuf) free(con->oldbuf);
								con->buf = (char_t *)calloc(sizeof(char_t),data.details.width*data.details.height);

								con->oldbuf = (char_t *)calloc(sizeof(char_t),data.details.width*data.details.height);
								con->w=data.details.width;
								con->h=data.details.height;
							}

						}
						else if (seg==layr){
							uint32 sz;
							uint32 ver;

							get32(&sz,fp);
							fix32(&sz);
							/* version */
							get32(&ver,fp);
							fix32(&ver);
							if (ver>2) ERR_NEWER("layer spec");

							if (ver==1){
								if (! getData((void*)&data.layer.headerv1, sizeof( LayerV1 ), fp)) ERR("Can't read layer header.");
								fixLayerv1(&data.layer.headerv1);

								/* Read in the data chunk*/
								data.layer.data = (uint8*)malloc(sizeof(uint8)*data.layer.headerv1.dataSize);
								getData((void*) data.layer.data, data.layer.headerv1.dataSize, fp);
							}
							else if (ver==2){
								if (! getData((void*)&data.layer.headerv2, sizeof( LayerV2 ), fp)) ERR("Can't read layer header.");
								fixLayerv2(&data.layer.headerv2);

								/* Read in the data chunk */
								data.layer.data = (uint8*)malloc(sizeof(uint8)*data.layer.headerv2.dataSize);
								getData((void*) data.layer.data, data.layer.headerv2.dataSize, fp);

							}
						}
						else {
							/* skip unknown segment */
							uint32 sz;
							get32(&sz,fp);
							fix32(&sz);
							fseek(fp,sz,SEEK_CUR);
						}
					}
				}

				/* we're done! */
				keepGoing = false;
			}
			else {
				/* skip this segment */
				uint32 sz;
				get32(&sz,fp);
				fseek(fp,sz,SEEK_CUR);
			}
		}

		imgData = data.layer.data;
		for(x = 0; x < con->w; x++) {
			for(y = 0; y < con->h; y++) {
	    	TCOD_color_t fore,back;
		    int c = (unsigned char)(imgData[index++]);
		    fore.r = (uint8)(imgData[index++]);
		    fore.g = (uint8)(imgData[index++]);
		    fore.b = (uint8)(imgData[index++]);
		    back.r = (uint8)(imgData[index++]);
		    back.g = (uint8)(imgData[index++]);
		    back.b = (uint8)(imgData[index++]);
		    TCOD_console_put_char_ex(con,x,y,c,fore,back);
			}
		}

		free (data.layer.data);
	}
	fclose(fp);

	return true;
}
/*

bool ApfFile::Load(std::string filename){
	detectBigEndianness();

	uint32 sett = fourCC("sett");
	uint32 imgd = fourCC("imgd");
	uint32 LIST = fourCC("LIST");
	uint32 LAYR = fourCC("LAYR");
	uint32 layr = fourCC("layr");

	Data data; // File data

	data.details.width = 1;
	data.details.height = 1;
	data.details.filter = FILTER_TYPE_UNCOMPRESSED;
	data.details.format = FORMAT_TYPE_CRGBRGB;

	data.settings.show_grid = true;
	data.settings.grid_width = 10;
	data.settings.grid_height = 10;

	data.currentLayer = NULL;

	#define ERR(x) {printf("Error: %s\n. Aborting operation.",x); return false;}
	#define ERR_NEWER(x) {printf("Error: It looks like this file was made with a newer version of Ascii-Paint\n. In particular the %s field. Aborting operation.",x); return false;}

	FILE* fp = fopen(filename.c_str(), "rb");
	if(fp == NULL) {
		printf("The file %s could not be loaded.\n", filename.c_str());
		return false;
	}
	else {
		// read the header
		uint32 riff;
		if (not get32(&riff,fp)
			or
			not fourCCequals(riff,"RIFF")){
			ERR("File doesn't have a RIFF header");
		}
		// else
		uint32 riffSize;
		if (!get32(&riffSize,fp)) ERR("No RIFF size field!");
		fix(&riffSize);

		bool keepGoing = true;
		while(keepGoing and fp){ // for each subfield, try to find the APF_ field
			uint32 apf;
			if (not get32(&apf,fp)) break;
			if (fourCCequals(apf,"apf ") or fourCCequals(apf,"APF ")){
				// Process APF segment
				while(keepGoing and fp){
					uint32 seg;
					if (not get32(&seg,fp)){
						keepGoing = false;
						break;
					}
					else {
						if (seg==sett){
							// size
							uint32 sz;
							get32(&sz,fp);
							fix(&sz);
							// version
							uint32 ver;
							get32(&ver,fp);
							fix(&ver);
							if (ver!=1) ERR_NEWER("settings");
							// ver must be 1
							SettingsDataV1 settingsData;
							if (not getData((void*)&settingsData,sizeof settingsData,fp)) ERR("Can't read settings.");
							data.settings = settingsData;
							fix(&data.settings);

							// Change app settings
							app->setGridDimensions(data.settings.grid_width,data.settings.grid_height);
							app->setShowGrid(data.settings.show_grid==1);
						}
						else if (seg==imgd){
							// sz
							uint32 sz;
							get32(&sz,fp);
							fix(&sz);
							// version
							uint32 ver;
							get32(&ver,fp);
							fix(&ver);
							if (ver!=1) ERR_NEWER("image details");
							// ver must be 1
							ImageDetailsV1 dets;
							if (not getData((void*)&dets, sizeof dets, fp)) ERR("Can't read image details.");
							data.details = dets;
							fix(&data.details);

							// get canvas ready
							app->canvasWidth = data.details.width;
							app->canvasHeight = data.details.height;
							app->initCanvas();

							// delete new layer
							app->deleteLayer(app->getCurrentLayer()->name);

						}
						else if (seg==layr){
							// printf("Found a layer\n");

							// sz
							uint32 sz;
							get32(&sz,fp);
							fix(&sz);
							// version
							uint32 ver;
							get32(&ver,fp);
							fix(&ver);
							if (ver>2) ERR_NEWER("layer spec");

							if (ver==1){
								LayerV1 layerHeader;
								if (not getData((void*)&layerHeader, sizeof layerHeader, fp)) ERR("Can't read layer header.");
								fix(&layerHeader);

								// creat new layer data
								LayerData* ld = new LayerData;
								ld->header = layerHeader; // already fix'd
								ld->data = new uint8[ld->header.dataSize];

								// Read in the data chunk
								getData((void*) ld->data, ld->header.dataSize, fp);

								// push layer onto the list
								data.currentLayer = ld;
								data.layers.push(ld);
							}
							else if (ver==2){
								LayerV2 layerHeader;
								if (not getData((void*)&layerHeader, sizeof layerHeader, fp)) ERR("Can't read layer header.");
								fix(&layerHeader);

								// creat new layer data
								LayerData* ld = new LayerData;
								ld->header = layerHeader; // already fix'd
								ld->data = new uint8[ld->header.dataSize];

								// Read in the data chunk
								getData((void*) ld->data, ld->header.dataSize, fp);

								// push layer onto the list
								data.currentLayer = ld;
								data.layers.push(ld);
							}
						}
						else {
							// skip unknown segment
							uint32 sz;
							get32(&sz,fp);
							fix(&sz);
							fseek(fp,sz,SEEK_CUR);
						}
					}
				}

				// we're done!
				keepGoing = false;
			}
			else {
				// skip this segment
				uint32 sz;
				get32(&sz,fp);
				fseek(fp,sz,SEEK_CUR);
			}
		}

		// finally, copy the layers into the current document
		for(int i=0;i<data.layers.size();i++){
			// for now, just load the first layer
			LayerData* ld = data.layers.get(i);

			// canvas width/height have already been set...
			Layer* l = app->addNewLayer();

			// Parse layer header
			l->name = fromFourCC(ld->header.name);
			l->fgalpha = ld->header.fgalpha;
			l->bgalpha = ld->header.bgalpha;
			l->visible = (ld->header.visible==1);
			// l->compositingMode =

			// Copy data into currently selected canvas
			uint8* imgData = ld->data;
			CanvasImage *img = new CanvasImage;
			// Write the brush data for every brush in the image
			int index = 0;
			for(int x = 0; x < app->canvasWidth; x++) {
				for(int y = 0; y < app->canvasHeight; y++) {
					Brush b;
					b.symbol = (unsigned char)(imgData[index++]);
					b.fore.r = (uint8)(imgData[index++]);
					b.fore.g = (uint8)(imgData[index++]);
					b.fore.b = (uint8)(imgData[index++]);
					b.back.r = (uint8)(imgData[index++]);
					b.back.g = (uint8)(imgData[index++]);
					b.back.b = (uint8)(imgData[index++]);
					b.solid = true; // deprecated
					b.walkable = true; // deprecated
					img->push_back(b);
				}
			}

			app->setCanvasImage(*img);
			delete img;
		}

		// then free all the temporary layer data
		for(int i=0;i<data.layers.size();i++){
			delete[]data.layers.get(i)->data;
			delete data.layers.get(i);
		}

		// and update the layer widget
		app->gui->layerWidget->regenerateLayerList();
	}
	fclose(fp);

	return true;
}
*/
