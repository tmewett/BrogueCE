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

typedef void *TCOD_image_t;

TCODLIB_API TCOD_image_t TCOD_image_new(int width, int height);
TCODLIB_API TCOD_image_t TCOD_image_from_console(TCOD_console_t console);
TCODLIB_API void TCOD_image_refresh_console(TCOD_image_t image, TCOD_console_t console);
TCODLIB_API TCOD_image_t TCOD_image_load(const char *filename);
TCODLIB_API void TCOD_image_clear(TCOD_image_t image, TCOD_color_t color);
TCODLIB_API void TCOD_image_invert(TCOD_image_t image);
TCODLIB_API void TCOD_image_hflip(TCOD_image_t image);
TCODLIB_API void TCOD_image_rotate90(TCOD_image_t image, int numRotations);
TCODLIB_API void TCOD_image_vflip(TCOD_image_t image);
TCODLIB_API void TCOD_image_scale(TCOD_image_t image, int neww, int newh);
TCODLIB_API void TCOD_image_save(TCOD_image_t image, const char *filename);
TCODLIB_API void TCOD_image_get_size(TCOD_image_t image, int *w,int *h);
TCODLIB_API TCOD_color_t TCOD_image_get_pixel(TCOD_image_t image,int x, int y);
TCODLIB_API int TCOD_image_get_alpha(TCOD_image_t image,int x, int y);
TCODLIB_API TCOD_color_t TCOD_image_get_mipmap_pixel(TCOD_image_t image,float x0,float y0, float x1, float y1);
TCODLIB_API void TCOD_image_put_pixel(TCOD_image_t image,int x, int y,TCOD_color_t col);
TCODLIB_API void TCOD_image_blit(TCOD_image_t image, TCOD_console_t console, float x, float y, 
	TCOD_bkgnd_flag_t bkgnd_flag, float scalex, float scaley, float angle);
TCODLIB_API void TCOD_image_blit_rect(TCOD_image_t image, TCOD_console_t console, int x, int y, int w, int h, 
	TCOD_bkgnd_flag_t bkgnd_flag);
TCODLIB_API void TCOD_image_blit_2x(TCOD_image_t image, TCOD_console_t dest, int dx, int dy, int sx, int sy, int w, int h);
TCODLIB_API void TCOD_image_delete(TCOD_image_t image);
TCODLIB_API void TCOD_image_set_key_color(TCOD_image_t image, TCOD_color_t key_color);
TCODLIB_API bool TCOD_image_is_pixel_transparent(TCOD_image_t image, int x, int y);

