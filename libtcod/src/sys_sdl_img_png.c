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

#if defined (__HAIKU__) || defined (__ANDROID__)
#include <SDL.h>
#elif defined(TCOD_SDL2)
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#include "png/lodepng.h"
#include "libtcod.h"
#include "libtcod_int.h"

bool TCOD_sys_check_png(const char *filename) {
	static uint8 magic_number[]={137, 80, 78, 71, 13, 10, 26, 10};
	return TCOD_sys_check_magic_number(filename,sizeof(magic_number),magic_number);
}

SDL_Surface *TCOD_sys_read_png(const char *filename) {
	unsigned error;
	unsigned char* image;
	unsigned width, height, y, bpp;
	unsigned char* png;
	size_t pngsize;
	LodePNGState state;
	SDL_Surface *bitmap;
	unsigned char *source;
	unsigned int rowsize;

	lodepng_state_init(&state);
	/*optionally customize the state*/
	if (!TCOD_sys_read_file(filename,&png,&pngsize)) return NULL;

	lodepng_inspect(&width,&height,&state, png, pngsize);
	bpp=lodepng_get_bpp(&state.info_png.color);

	if ( bpp == 24 ) {
		/* don't convert to 32 bits because libtcod's 24bits renderer is faster */
		state.info_raw.colortype=LCT_RGB;
	} else if (  bpp != 24 && bpp != 32 ) { 
		/* paletted png. convert to 24 bits */
		state.info_raw.colortype=LCT_RGB;
		state.info_raw.bitdepth=8;
		bpp=24;
	}
	error = lodepng_decode(&image, &width, &height, &state, png, pngsize);
	free(png);
	if(error) {
		printf("error %u: %s\n", error, lodepng_error_text(error));
		lodepng_state_cleanup(&state);
		return NULL;
	}
		
	/* create the SDL surface */
	bitmap=TCOD_sys_get_surface(width,height,bpp==32);
	source=image;
	rowsize=width*bpp/8;
	for (y=0; y<  height; y++ ) {
		Uint8 *row_pointer=(Uint8 *)(bitmap->pixels) + y * bitmap->pitch;
		memcpy(row_pointer,source,rowsize);
		source+=rowsize;
	}

	lodepng_state_cleanup(&state);
	free(image);	
	return bitmap;
}

void TCOD_sys_write_png(const SDL_Surface *surf, const char *filename) {
	unsigned char *image, *dest=(unsigned char *)malloc(surf->h*surf->w*3*sizeof(char));
	int x,y;
	unsigned char *buf;
	size_t size;
	int error;
	/* SDL uses 32bits format without alpha layer for screen. convert it to 24 bits */
	image=dest;
	for (y=0; y<  surf->h; y++ ) {
		for (x=0; x < surf->w; x++ ) {
			Uint8 *pixel=(Uint8 *)(surf->pixels) + y * surf->pitch + x * surf->format->BytesPerPixel;
			*dest++=*((pixel)+surf->format->Rshift/8);
			*dest++=*((pixel)+surf->format->Gshift/8);
			*dest++=*((pixel)+surf->format->Bshift/8);
		}
	}
	error=lodepng_encode_memory(&buf,&size,image,surf->w,surf->h,LCT_RGB,8);
	free(image);
	if ( ! error ) {
		TCOD_sys_write_file(filename,buf,size);
		free(buf);
	} else {
		printf("error %u: %s\n", error, lodepng_error_text(error));
	}
}

