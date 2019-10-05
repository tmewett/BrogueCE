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
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#include "libtcod.h"
#include "libtcod_int.h"

bool TCOD_sys_check_bmp(const char *filename) {
	static uint8 magic_number[]={0x42, 0x4d};
	return TCOD_sys_check_magic_number(filename,sizeof(magic_number),magic_number);
}

SDL_Surface *TCOD_sys_read_bmp(const char *filename) {
	SDL_Surface *ret=SDL_LoadBMP(filename);
	if( !ret ) TCOD_fatal("SDL : %s",SDL_GetError());
	/* convert low color images to 24 bits */
	if ( ret->format->BytesPerPixel != 3 ) {
		Uint32 rmask,gmask,bmask;
        SDL_Surface * tmp;
		if ( SDL_BYTEORDER == SDL_LIL_ENDIAN ) {
			rmask=0xFF0000;
			gmask=0x00FF00;
			bmask=0x0000FF;
		} else {
			rmask=0x0000FF;
			gmask=0x00FF00;
			bmask=0xFF0000;
		}
		tmp=SDL_CreateRGBSurface(SDL_SWSURFACE,ret->w,ret->h,24, rmask, gmask, bmask, 0);
		SDL_BlitSurface(ret,NULL,tmp,NULL);
		SDL_FreeSurface(ret);
		ret=tmp;
	}

	return ret;
}

void TCOD_sys_write_bmp(const SDL_Surface *surf, const char *filename) {
	SDL_SaveBMP((SDL_Surface *)surf,filename);
}


