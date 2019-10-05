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

#ifndef _TCOD_COLOR_H
#define _TCOD_COLOR_H

typedef struct {
	uint8 r,g,b;
} TCOD_color_t;

/* constructors */
TCODLIB_API TCOD_color_t TCOD_color_RGB(uint8 r, uint8 g, uint8 b);
TCODLIB_API TCOD_color_t TCOD_color_HSV(float h, float s, float v);
/* basic operations */
TCODLIB_API bool TCOD_color_equals (TCOD_color_t c1, TCOD_color_t c2);
TCODLIB_API TCOD_color_t TCOD_color_add (TCOD_color_t c1, TCOD_color_t c2);
TCODLIB_API TCOD_color_t TCOD_color_subtract (TCOD_color_t c1, TCOD_color_t c2);
TCODLIB_API TCOD_color_t TCOD_color_multiply (TCOD_color_t c1, TCOD_color_t c2);
TCODLIB_API TCOD_color_t TCOD_color_multiply_scalar (TCOD_color_t c1, float value);
TCODLIB_API TCOD_color_t TCOD_color_lerp (TCOD_color_t c1, TCOD_color_t c2, float coef);
/* HSV transformations */
TCODLIB_API void TCOD_color_set_HSV (TCOD_color_t *c,float h, float s, float v);
TCODLIB_API void TCOD_color_get_HSV (TCOD_color_t c,float * h, float * s, float * v);
TCODLIB_API float TCOD_color_get_hue (TCOD_color_t c);
TCODLIB_API void TCOD_color_set_hue (TCOD_color_t *c, float h);
TCODLIB_API float TCOD_color_get_saturation (TCOD_color_t c);
TCODLIB_API void TCOD_color_set_saturation (TCOD_color_t *c, float s);
TCODLIB_API float TCOD_color_get_value (TCOD_color_t c);
TCODLIB_API void TCOD_color_set_value (TCOD_color_t *c, float v);
TCODLIB_API void TCOD_color_shift_hue (TCOD_color_t *c, float hshift);
TCODLIB_API void TCOD_color_scale_HSV (TCOD_color_t *c, float scoef, float vcoef);
/* color map */
TCODLIB_API void TCOD_color_gen_map(TCOD_color_t *map, int nb_key, TCOD_color_t const *key_color, int const *key_index);

/* color names */
enum {
	TCOD_COLOR_RED,
	TCOD_COLOR_FLAME,
	TCOD_COLOR_ORANGE,
	TCOD_COLOR_AMBER,
	TCOD_COLOR_YELLOW,
	TCOD_COLOR_LIME,
	TCOD_COLOR_CHARTREUSE,
	TCOD_COLOR_GREEN,
	TCOD_COLOR_SEA,
	TCOD_COLOR_TURQUOISE,
	TCOD_COLOR_CYAN,
	TCOD_COLOR_SKY,
	TCOD_COLOR_AZURE,
	TCOD_COLOR_BLUE,
	TCOD_COLOR_HAN,
	TCOD_COLOR_VIOLET,
	TCOD_COLOR_PURPLE,
	TCOD_COLOR_FUCHSIA,
	TCOD_COLOR_MAGENTA,
	TCOD_COLOR_PINK,
	TCOD_COLOR_CRIMSON,
	TCOD_COLOR_NB
};

/* color levels */
enum {
	TCOD_COLOR_DESATURATED,
	TCOD_COLOR_LIGHTEST,
	TCOD_COLOR_LIGHTER,
	TCOD_COLOR_LIGHT,
	TCOD_COLOR_NORMAL,
	TCOD_COLOR_DARK,
	TCOD_COLOR_DARKER,
	TCOD_COLOR_DARKEST,
	TCOD_COLOR_LEVELS
};

/* color array */
extern TCODLIB_API const TCOD_color_t  TCOD_colors[TCOD_COLOR_NB][TCOD_COLOR_LEVELS];

/* grey levels */
extern TCODLIB_API const TCOD_color_t TCOD_black;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_grey;
extern TCODLIB_API const TCOD_color_t TCOD_darker_grey;
extern TCODLIB_API const TCOD_color_t TCOD_dark_grey;
extern TCODLIB_API const TCOD_color_t TCOD_grey;
extern TCODLIB_API const TCOD_color_t TCOD_light_grey;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_grey;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_grey;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_gray;
extern TCODLIB_API const TCOD_color_t TCOD_darker_gray;
extern TCODLIB_API const TCOD_color_t TCOD_dark_gray;
extern TCODLIB_API const TCOD_color_t TCOD_gray;
extern TCODLIB_API const TCOD_color_t TCOD_light_gray;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_gray;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_gray;
extern TCODLIB_API const TCOD_color_t TCOD_white;

/* sepia */
extern TCODLIB_API const TCOD_color_t TCOD_darkest_sepia;
extern TCODLIB_API const TCOD_color_t TCOD_darker_sepia;
extern TCODLIB_API const TCOD_color_t TCOD_dark_sepia;
extern TCODLIB_API const TCOD_color_t TCOD_sepia;
extern TCODLIB_API const TCOD_color_t TCOD_light_sepia;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_sepia;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_sepia;

/* standard colors */
extern TCODLIB_API const TCOD_color_t TCOD_red;
extern TCODLIB_API const TCOD_color_t TCOD_flame;
extern TCODLIB_API const TCOD_color_t TCOD_orange;
extern TCODLIB_API const TCOD_color_t TCOD_amber;
extern TCODLIB_API const TCOD_color_t TCOD_yellow;
extern TCODLIB_API const TCOD_color_t TCOD_lime;
extern TCODLIB_API const TCOD_color_t TCOD_chartreuse;
extern TCODLIB_API const TCOD_color_t TCOD_green;
extern TCODLIB_API const TCOD_color_t TCOD_sea;
extern TCODLIB_API const TCOD_color_t TCOD_turquoise;
extern TCODLIB_API const TCOD_color_t TCOD_cyan;
extern TCODLIB_API const TCOD_color_t TCOD_sky;
extern TCODLIB_API const TCOD_color_t TCOD_azure;
extern TCODLIB_API const TCOD_color_t TCOD_blue;
extern TCODLIB_API const TCOD_color_t TCOD_han;
extern TCODLIB_API const TCOD_color_t TCOD_violet;
extern TCODLIB_API const TCOD_color_t TCOD_purple;
extern TCODLIB_API const TCOD_color_t TCOD_fuchsia;
extern TCODLIB_API const TCOD_color_t TCOD_magenta;
extern TCODLIB_API const TCOD_color_t TCOD_pink;
extern TCODLIB_API const TCOD_color_t TCOD_crimson;

/* dark colors */
extern TCODLIB_API const TCOD_color_t TCOD_dark_red;
extern TCODLIB_API const TCOD_color_t TCOD_dark_flame;
extern TCODLIB_API const TCOD_color_t TCOD_dark_orange;
extern TCODLIB_API const TCOD_color_t TCOD_dark_amber;
extern TCODLIB_API const TCOD_color_t TCOD_dark_yellow;
extern TCODLIB_API const TCOD_color_t TCOD_dark_lime;
extern TCODLIB_API const TCOD_color_t TCOD_dark_chartreuse;
extern TCODLIB_API const TCOD_color_t TCOD_dark_green;
extern TCODLIB_API const TCOD_color_t TCOD_dark_sea;
extern TCODLIB_API const TCOD_color_t TCOD_dark_turquoise;
extern TCODLIB_API const TCOD_color_t TCOD_dark_cyan;
extern TCODLIB_API const TCOD_color_t TCOD_dark_sky;
extern TCODLIB_API const TCOD_color_t TCOD_dark_azure;
extern TCODLIB_API const TCOD_color_t TCOD_dark_blue;
extern TCODLIB_API const TCOD_color_t TCOD_dark_han;
extern TCODLIB_API const TCOD_color_t TCOD_dark_violet;
extern TCODLIB_API const TCOD_color_t TCOD_dark_purple;
extern TCODLIB_API const TCOD_color_t TCOD_dark_fuchsia;
extern TCODLIB_API const TCOD_color_t TCOD_dark_magenta;
extern TCODLIB_API const TCOD_color_t TCOD_dark_pink;
extern TCODLIB_API const TCOD_color_t TCOD_dark_crimson;

/* darker colors */
extern TCODLIB_API const TCOD_color_t TCOD_darker_red;
extern TCODLIB_API const TCOD_color_t TCOD_darker_flame;
extern TCODLIB_API const TCOD_color_t TCOD_darker_orange;
extern TCODLIB_API const TCOD_color_t TCOD_darker_amber;
extern TCODLIB_API const TCOD_color_t TCOD_darker_yellow;
extern TCODLIB_API const TCOD_color_t TCOD_darker_lime;
extern TCODLIB_API const TCOD_color_t TCOD_darker_chartreuse;
extern TCODLIB_API const TCOD_color_t TCOD_darker_green;
extern TCODLIB_API const TCOD_color_t TCOD_darker_sea;
extern TCODLIB_API const TCOD_color_t TCOD_darker_turquoise;
extern TCODLIB_API const TCOD_color_t TCOD_darker_cyan;
extern TCODLIB_API const TCOD_color_t TCOD_darker_sky;
extern TCODLIB_API const TCOD_color_t TCOD_darker_azure;
extern TCODLIB_API const TCOD_color_t TCOD_darker_blue;
extern TCODLIB_API const TCOD_color_t TCOD_darker_han;
extern TCODLIB_API const TCOD_color_t TCOD_darker_violet;
extern TCODLIB_API const TCOD_color_t TCOD_darker_purple;
extern TCODLIB_API const TCOD_color_t TCOD_darker_fuchsia;
extern TCODLIB_API const TCOD_color_t TCOD_darker_magenta;
extern TCODLIB_API const TCOD_color_t TCOD_darker_pink;
extern TCODLIB_API const TCOD_color_t TCOD_darker_crimson;

/* darkest colors */
extern TCODLIB_API const TCOD_color_t TCOD_darkest_red;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_flame;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_orange;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_amber;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_yellow;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_lime;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_chartreuse;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_green;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_sea;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_turquoise;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_cyan;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_sky;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_azure;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_blue;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_han;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_violet;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_purple;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_fuchsia;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_magenta;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_pink;
extern TCODLIB_API const TCOD_color_t TCOD_darkest_crimson;

/* light colors */
extern TCODLIB_API const TCOD_color_t TCOD_light_red;
extern TCODLIB_API const TCOD_color_t TCOD_light_flame;
extern TCODLIB_API const TCOD_color_t TCOD_light_orange;
extern TCODLIB_API const TCOD_color_t TCOD_light_amber;
extern TCODLIB_API const TCOD_color_t TCOD_light_yellow;
extern TCODLIB_API const TCOD_color_t TCOD_light_lime;
extern TCODLIB_API const TCOD_color_t TCOD_light_chartreuse;
extern TCODLIB_API const TCOD_color_t TCOD_light_green;
extern TCODLIB_API const TCOD_color_t TCOD_light_sea;
extern TCODLIB_API const TCOD_color_t TCOD_light_turquoise;
extern TCODLIB_API const TCOD_color_t TCOD_light_cyan;
extern TCODLIB_API const TCOD_color_t TCOD_light_sky;
extern TCODLIB_API const TCOD_color_t TCOD_light_azure;
extern TCODLIB_API const TCOD_color_t TCOD_light_blue;
extern TCODLIB_API const TCOD_color_t TCOD_light_han;
extern TCODLIB_API const TCOD_color_t TCOD_light_violet;
extern TCODLIB_API const TCOD_color_t TCOD_light_purple;
extern TCODLIB_API const TCOD_color_t TCOD_light_fuchsia;
extern TCODLIB_API const TCOD_color_t TCOD_light_magenta;
extern TCODLIB_API const TCOD_color_t TCOD_light_pink;
extern TCODLIB_API const TCOD_color_t TCOD_light_crimson;

/* lighter colors */
extern TCODLIB_API const TCOD_color_t TCOD_lighter_red;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_flame;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_orange;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_amber;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_yellow;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_lime;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_chartreuse;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_green;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_sea;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_turquoise;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_cyan;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_sky;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_azure;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_blue;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_han;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_violet;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_purple;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_fuchsia;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_magenta;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_pink;
extern TCODLIB_API const TCOD_color_t TCOD_lighter_crimson;

/* lightest colors */
extern TCODLIB_API const TCOD_color_t TCOD_lightest_red;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_flame;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_orange;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_amber;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_yellow;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_lime;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_chartreuse;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_green;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_sea;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_turquoise;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_cyan;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_sky;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_azure;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_blue;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_han;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_violet;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_purple;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_fuchsia;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_magenta;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_pink;
extern TCODLIB_API const TCOD_color_t TCOD_lightest_crimson;

/* desaturated */
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_red;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_flame;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_orange;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_amber;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_yellow;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_lime;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_chartreuse;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_green;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_sea;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_turquoise;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_cyan;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_sky;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_azure;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_blue;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_han;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_violet;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_purple;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_fuchsia;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_magenta;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_pink;
extern TCODLIB_API const TCOD_color_t TCOD_desaturated_crimson;

/* metallic */
extern TCODLIB_API const TCOD_color_t TCOD_brass;
extern TCODLIB_API const TCOD_color_t TCOD_copper;
extern TCODLIB_API const TCOD_color_t TCOD_gold;
extern TCODLIB_API const TCOD_color_t TCOD_silver;

/* miscellaneous */
extern TCODLIB_API const TCOD_color_t TCOD_celadon;
extern TCODLIB_API const TCOD_color_t TCOD_peach;

#endif
