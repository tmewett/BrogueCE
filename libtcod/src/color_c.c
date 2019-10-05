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
#include <math.h>
#include "libtcod.h"
#include "libtcod_int.h"

/* grey levels */
const TCOD_color_t TCOD_black={TCOD_BLACK};
const TCOD_color_t TCOD_darkest_grey={TCOD_DARKEST_GREY};
const TCOD_color_t TCOD_darker_grey={TCOD_DARKER_GREY};
const TCOD_color_t TCOD_dark_grey={TCOD_DARK_GREY};
const TCOD_color_t TCOD_grey={TCOD_GREY};
const TCOD_color_t TCOD_light_grey={TCOD_LIGHT_GREY};
const TCOD_color_t TCOD_lighter_grey={TCOD_LIGHTER_GREY};
const TCOD_color_t TCOD_lightest_grey={TCOD_LIGHTEST_GREY};
const TCOD_color_t TCOD_darkest_gray={TCOD_DARKEST_GREY};
const TCOD_color_t TCOD_darker_gray={TCOD_DARKER_GREY};
const TCOD_color_t TCOD_dark_gray={TCOD_DARK_GREY};
const TCOD_color_t TCOD_gray={TCOD_GREY};
const TCOD_color_t TCOD_light_gray={TCOD_LIGHT_GREY};
const TCOD_color_t TCOD_lighter_gray={TCOD_LIGHTER_GREY};
const TCOD_color_t TCOD_lightest_gray={TCOD_LIGHTEST_GREY};
const TCOD_color_t TCOD_white={TCOD_WHITE};

/* sepia */
const TCOD_color_t TCOD_darkest_sepia={TCOD_DARKEST_SEPIA};
const TCOD_color_t TCOD_darker_sepia={TCOD_DARKER_SEPIA};
const TCOD_color_t TCOD_dark_sepia={TCOD_DARK_SEPIA};
const TCOD_color_t TCOD_sepia={TCOD_SEPIA};
const TCOD_color_t TCOD_light_sepia={TCOD_LIGHT_SEPIA};
const TCOD_color_t TCOD_lighter_sepia={TCOD_LIGHTER_SEPIA};
const TCOD_color_t TCOD_lightest_sepia={TCOD_LIGHTEST_SEPIA};

/* standard colors */
const TCOD_color_t TCOD_red = {TCOD_RED};
const TCOD_color_t TCOD_flame = {TCOD_FLAME};
const TCOD_color_t TCOD_orange = {TCOD_ORANGE};
const TCOD_color_t TCOD_amber = {TCOD_AMBER};
const TCOD_color_t TCOD_yellow = {TCOD_YELLOW};
const TCOD_color_t TCOD_lime = {TCOD_LIME};
const TCOD_color_t TCOD_chartreuse = {TCOD_CHARTREUSE};
const TCOD_color_t TCOD_green = {TCOD_GREEN};
const TCOD_color_t TCOD_sea = {TCOD_SEA};
const TCOD_color_t TCOD_turquoise = {TCOD_TURQUOISE};
const TCOD_color_t TCOD_cyan = {TCOD_CYAN};
const TCOD_color_t TCOD_sky = {TCOD_SKY};
const TCOD_color_t TCOD_azure = {TCOD_AZURE};
const TCOD_color_t TCOD_blue = {TCOD_BLUE};
const TCOD_color_t TCOD_han = {TCOD_HAN};
const TCOD_color_t TCOD_violet = {TCOD_VIOLET};
const TCOD_color_t TCOD_purple = {TCOD_PURPLE};
const TCOD_color_t TCOD_fuchsia = {TCOD_FUCHSIA};
const TCOD_color_t TCOD_magenta = {TCOD_MAGENTA};
const TCOD_color_t TCOD_pink = {TCOD_PINK};
const TCOD_color_t TCOD_crimson = {TCOD_CRIMSON};

/* dark colors */
const TCOD_color_t TCOD_dark_red = {TCOD_DARK_RED};
const TCOD_color_t TCOD_dark_flame = {TCOD_DARK_FLAME};
const TCOD_color_t TCOD_dark_orange = {TCOD_DARK_ORANGE};
const TCOD_color_t TCOD_dark_amber = {TCOD_DARK_AMBER};
const TCOD_color_t TCOD_dark_yellow = {TCOD_DARK_YELLOW};
const TCOD_color_t TCOD_dark_lime = {TCOD_DARK_LIME};
const TCOD_color_t TCOD_dark_chartreuse = {TCOD_DARK_CHARTREUSE};
const TCOD_color_t TCOD_dark_green = {TCOD_DARK_GREEN};
const TCOD_color_t TCOD_dark_sea = {TCOD_DARK_SEA};
const TCOD_color_t TCOD_dark_turquoise = {TCOD_DARK_TURQUOISE};
const TCOD_color_t TCOD_dark_cyan = {TCOD_DARK_CYAN};
const TCOD_color_t TCOD_dark_sky = {TCOD_DARK_SKY};
const TCOD_color_t TCOD_dark_azure = {TCOD_DARK_AZURE};
const TCOD_color_t TCOD_dark_blue = {TCOD_DARK_BLUE};
const TCOD_color_t TCOD_dark_han = {TCOD_DARK_HAN};
const TCOD_color_t TCOD_dark_violet = {TCOD_DARK_VIOLET};
const TCOD_color_t TCOD_dark_purple = {TCOD_DARK_PURPLE};
const TCOD_color_t TCOD_dark_fuchsia = {TCOD_DARK_FUCHSIA};
const TCOD_color_t TCOD_dark_magenta = {TCOD_DARK_MAGENTA};
const TCOD_color_t TCOD_dark_pink = {TCOD_DARK_PINK};
const TCOD_color_t TCOD_dark_crimson = {TCOD_DARK_CRIMSON};

/* darker colors */
const TCOD_color_t TCOD_darker_red = {TCOD_DARKER_RED};
const TCOD_color_t TCOD_darker_flame = {TCOD_DARKER_FLAME};
const TCOD_color_t TCOD_darker_orange = {TCOD_DARKER_ORANGE};
const TCOD_color_t TCOD_darker_amber = {TCOD_DARKER_AMBER};
const TCOD_color_t TCOD_darker_yellow = {TCOD_DARKER_YELLOW};
const TCOD_color_t TCOD_darker_lime = {TCOD_DARKER_LIME};
const TCOD_color_t TCOD_darker_chartreuse = {TCOD_DARKER_CHARTREUSE};
const TCOD_color_t TCOD_darker_green = {TCOD_DARKER_GREEN};
const TCOD_color_t TCOD_darker_sea = {TCOD_DARKER_SEA};
const TCOD_color_t TCOD_darker_turquoise = {TCOD_DARKER_TURQUOISE};
const TCOD_color_t TCOD_darker_cyan = {TCOD_DARKER_CYAN};
const TCOD_color_t TCOD_darker_sky = {TCOD_DARKER_SKY};
const TCOD_color_t TCOD_darker_azure = {TCOD_DARKER_AZURE};
const TCOD_color_t TCOD_darker_blue = {TCOD_DARKER_BLUE};
const TCOD_color_t TCOD_darker_han = {TCOD_DARKER_HAN};
const TCOD_color_t TCOD_darker_violet = {TCOD_DARKER_VIOLET};
const TCOD_color_t TCOD_darker_purple = {TCOD_DARKER_PURPLE};
const TCOD_color_t TCOD_darker_fuchsia = {TCOD_DARKER_FUCHSIA};
const TCOD_color_t TCOD_darker_magenta = {TCOD_DARKER_MAGENTA};
const TCOD_color_t TCOD_darker_pink = {TCOD_DARKER_PINK};
const TCOD_color_t TCOD_darker_crimson = {TCOD_DARKER_CRIMSON};

/* darkest colors */
const TCOD_color_t TCOD_darkest_red = {TCOD_DARKEST_RED};
const TCOD_color_t TCOD_darkest_flame = {TCOD_DARKEST_FLAME};
const TCOD_color_t TCOD_darkest_orange = {TCOD_DARKEST_ORANGE};
const TCOD_color_t TCOD_darkest_amber = {TCOD_DARKEST_AMBER};
const TCOD_color_t TCOD_darkest_yellow = {TCOD_DARKEST_YELLOW};
const TCOD_color_t TCOD_darkest_lime = {TCOD_DARKEST_LIME};
const TCOD_color_t TCOD_darkest_chartreuse = {TCOD_DARKEST_CHARTREUSE};
const TCOD_color_t TCOD_darkest_green = {TCOD_DARKEST_GREEN};
const TCOD_color_t TCOD_darkest_sea = {TCOD_DARKEST_SEA};
const TCOD_color_t TCOD_darkest_turquoise = {TCOD_DARKEST_TURQUOISE};
const TCOD_color_t TCOD_darkest_cyan = {TCOD_DARKEST_CYAN};
const TCOD_color_t TCOD_darkest_sky = {TCOD_DARKEST_SKY};
const TCOD_color_t TCOD_darkest_azure = {TCOD_DARKEST_AZURE};
const TCOD_color_t TCOD_darkest_blue = {TCOD_DARKEST_BLUE};
const TCOD_color_t TCOD_darkest_han = {TCOD_DARKEST_HAN};
const TCOD_color_t TCOD_darkest_violet = {TCOD_DARKEST_VIOLET};
const TCOD_color_t TCOD_darkest_purple = {TCOD_DARKEST_PURPLE};
const TCOD_color_t TCOD_darkest_fuchsia = {TCOD_DARKEST_FUCHSIA};
const TCOD_color_t TCOD_darkest_magenta = {TCOD_DARKEST_MAGENTA};
const TCOD_color_t TCOD_darkest_pink = {TCOD_DARKEST_PINK};
const TCOD_color_t TCOD_darkest_crimson = {TCOD_DARKEST_CRIMSON};

/* light colors */
const TCOD_color_t TCOD_light_red = {TCOD_LIGHT_RED};
const TCOD_color_t TCOD_light_flame = {TCOD_LIGHT_FLAME};
const TCOD_color_t TCOD_light_orange = {TCOD_LIGHT_ORANGE};
const TCOD_color_t TCOD_light_amber = {TCOD_LIGHT_AMBER};
const TCOD_color_t TCOD_light_yellow = {TCOD_LIGHT_YELLOW};
const TCOD_color_t TCOD_light_lime = {TCOD_LIGHT_LIME};
const TCOD_color_t TCOD_light_chartreuse = {TCOD_LIGHT_CHARTREUSE};
const TCOD_color_t TCOD_light_green = {TCOD_LIGHT_GREEN};
const TCOD_color_t TCOD_light_sea = {TCOD_LIGHT_SEA};
const TCOD_color_t TCOD_light_turquoise = {TCOD_LIGHT_TURQUOISE};
const TCOD_color_t TCOD_light_cyan = {TCOD_LIGHT_CYAN};
const TCOD_color_t TCOD_light_sky = {TCOD_LIGHT_SKY};
const TCOD_color_t TCOD_light_azure = {TCOD_LIGHT_AZURE};
const TCOD_color_t TCOD_light_blue = {TCOD_LIGHT_BLUE};
const TCOD_color_t TCOD_light_han = {TCOD_LIGHT_HAN};
const TCOD_color_t TCOD_light_violet = {TCOD_LIGHT_VIOLET};
const TCOD_color_t TCOD_light_purple = {TCOD_LIGHT_PURPLE};
const TCOD_color_t TCOD_light_fuchsia = {TCOD_LIGHT_FUCHSIA};
const TCOD_color_t TCOD_light_magenta = {TCOD_LIGHT_MAGENTA};
const TCOD_color_t TCOD_light_pink = {TCOD_LIGHT_PINK};
const TCOD_color_t TCOD_light_crimson = {TCOD_LIGHT_CRIMSON};

/*lighter colors */
const TCOD_color_t TCOD_lighter_red = {TCOD_LIGHTER_RED};
const TCOD_color_t TCOD_lighter_flame = {TCOD_LIGHTER_FLAME};
const TCOD_color_t TCOD_lighter_orange = {TCOD_LIGHTER_ORANGE};
const TCOD_color_t TCOD_lighter_amber = {TCOD_LIGHTER_AMBER};
const TCOD_color_t TCOD_lighter_yellow = {TCOD_LIGHTER_YELLOW};
const TCOD_color_t TCOD_lighter_lime = {TCOD_LIGHTER_LIME};
const TCOD_color_t TCOD_lighter_chartreuse = {TCOD_LIGHTER_CHARTREUSE};
const TCOD_color_t TCOD_lighter_green = {TCOD_LIGHTER_GREEN};
const TCOD_color_t TCOD_lighter_sea = {TCOD_LIGHTER_SEA};
const TCOD_color_t TCOD_lighter_turquoise = {TCOD_LIGHTER_TURQUOISE};
const TCOD_color_t TCOD_lighter_cyan = {TCOD_LIGHTER_CYAN};
const TCOD_color_t TCOD_lighter_sky = {TCOD_LIGHTER_SKY};
const TCOD_color_t TCOD_lighter_azure = {TCOD_LIGHTER_AZURE};
const TCOD_color_t TCOD_lighter_blue = {TCOD_LIGHTER_BLUE};
const TCOD_color_t TCOD_lighter_han = {TCOD_LIGHTER_HAN};
const TCOD_color_t TCOD_lighter_violet = {TCOD_LIGHTER_VIOLET};
const TCOD_color_t TCOD_lighter_purple = {TCOD_LIGHTER_PURPLE};
const TCOD_color_t TCOD_lighter_fuchsia = {TCOD_LIGHTER_FUCHSIA};
const TCOD_color_t TCOD_lighter_magenta = {TCOD_LIGHTER_MAGENTA};
const TCOD_color_t TCOD_lighter_pink = {TCOD_LIGHTER_PINK};
const TCOD_color_t TCOD_lighter_crimson = {TCOD_LIGHTER_CRIMSON};

/* lightest colors */
const TCOD_color_t TCOD_lightest_red = {TCOD_LIGHTEST_RED};
const TCOD_color_t TCOD_lightest_flame = {TCOD_LIGHTEST_FLAME};
const TCOD_color_t TCOD_lightest_orange = {TCOD_LIGHTEST_ORANGE};
const TCOD_color_t TCOD_lightest_amber = {TCOD_LIGHTEST_AMBER};
const TCOD_color_t TCOD_lightest_yellow = {TCOD_LIGHTEST_YELLOW};
const TCOD_color_t TCOD_lightest_lime = {TCOD_LIGHTEST_LIME};
const TCOD_color_t TCOD_lightest_chartreuse = {TCOD_LIGHTEST_CHARTREUSE};
const TCOD_color_t TCOD_lightest_green = {TCOD_LIGHTEST_GREEN};
const TCOD_color_t TCOD_lightest_sea = {TCOD_LIGHTEST_SEA};
const TCOD_color_t TCOD_lightest_turquoise = {TCOD_LIGHTEST_TURQUOISE};
const TCOD_color_t TCOD_lightest_cyan = {TCOD_LIGHTEST_CYAN};
const TCOD_color_t TCOD_lightest_sky = {TCOD_LIGHTEST_SKY};
const TCOD_color_t TCOD_lightest_azure = {TCOD_LIGHTEST_AZURE};
const TCOD_color_t TCOD_lightest_blue = {TCOD_LIGHTEST_BLUE};
const TCOD_color_t TCOD_lightest_han = {TCOD_LIGHTEST_HAN};
const TCOD_color_t TCOD_lightest_violet = {TCOD_LIGHTEST_VIOLET};
const TCOD_color_t TCOD_lightest_purple = {TCOD_LIGHTEST_PURPLE};
const TCOD_color_t TCOD_lightest_fuchsia = {TCOD_LIGHTEST_FUCHSIA};
const TCOD_color_t TCOD_lightest_magenta = {TCOD_LIGHTEST_MAGENTA};
const TCOD_color_t TCOD_lightest_pink = {TCOD_LIGHTEST_PINK};
const TCOD_color_t TCOD_lightest_crimson = {TCOD_LIGHTEST_CRIMSON};

/* desaturated colors */
const TCOD_color_t TCOD_desaturated_red = {TCOD_DESATURATED_RED};
const TCOD_color_t TCOD_desaturated_flame = {TCOD_DESATURATED_FLAME};
const TCOD_color_t TCOD_desaturated_orange = {TCOD_DESATURATED_ORANGE};
const TCOD_color_t TCOD_desaturated_amber = {TCOD_DESATURATED_AMBER};
const TCOD_color_t TCOD_desaturated_yellow = {TCOD_DESATURATED_YELLOW};
const TCOD_color_t TCOD_desaturated_lime = {TCOD_DESATURATED_LIME};
const TCOD_color_t TCOD_desaturated_chartreuse = {TCOD_DESATURATED_CHARTREUSE};
const TCOD_color_t TCOD_desaturated_green = {TCOD_DESATURATED_GREEN};
const TCOD_color_t TCOD_desaturated_sea = {TCOD_DESATURATED_SEA};
const TCOD_color_t TCOD_desaturated_turquoise = {TCOD_DESATURATED_TURQUOISE};
const TCOD_color_t TCOD_desaturated_cyan = {TCOD_DESATURATED_CYAN};
const TCOD_color_t TCOD_desaturated_sky = {TCOD_DESATURATED_SKY};
const TCOD_color_t TCOD_desaturated_azure = {TCOD_DESATURATED_AZURE};
const TCOD_color_t TCOD_desaturated_blue = {TCOD_DESATURATED_BLUE};
const TCOD_color_t TCOD_desaturated_han = {TCOD_DESATURATED_HAN};
const TCOD_color_t TCOD_desaturated_violet = {TCOD_DESATURATED_VIOLET};
const TCOD_color_t TCOD_desaturated_purple = {TCOD_DESATURATED_PURPLE};
const TCOD_color_t TCOD_desaturated_fuchsia = {TCOD_DESATURATED_FUCHSIA};
const TCOD_color_t TCOD_desaturated_magenta = {TCOD_DESATURATED_MAGENTA};
const TCOD_color_t TCOD_desaturated_pink = {TCOD_DESATURATED_PINK};
const TCOD_color_t TCOD_desaturated_crimson = {TCOD_DESATURATED_CRIMSON};

/* metallic */
const TCOD_color_t TCOD_brass = {TCOD_BRASS};
const TCOD_color_t TCOD_copper = {TCOD_COPPER};
const TCOD_color_t TCOD_gold = {TCOD_GOLD};
const TCOD_color_t TCOD_silver = {TCOD_SILVER};

/* miscellaneous */
const TCOD_color_t TCOD_celadon = {TCOD_CELADON};
const TCOD_color_t TCOD_peach = {TCOD_PEACH};

/* color array */
const TCOD_color_t TCOD_colors[TCOD_COLOR_NB][TCOD_COLOR_LEVELS] = {
 {{TCOD_DESATURATED_RED},{TCOD_LIGHTEST_RED},{TCOD_LIGHTER_RED},{TCOD_LIGHT_RED},{TCOD_RED},{TCOD_DARK_RED},{TCOD_DARKER_RED},{TCOD_DARKEST_RED}},
 {{TCOD_DESATURATED_FLAME},{TCOD_LIGHTEST_FLAME},{TCOD_LIGHTER_FLAME},{TCOD_LIGHT_FLAME},{TCOD_FLAME},{TCOD_DARK_FLAME},{TCOD_DARKER_FLAME},{TCOD_DARKEST_FLAME}},
 {{TCOD_DESATURATED_ORANGE},{TCOD_LIGHTEST_ORANGE},{TCOD_LIGHTER_ORANGE},{TCOD_LIGHT_ORANGE},{TCOD_ORANGE},{TCOD_DARK_ORANGE},{TCOD_DARKER_ORANGE},{TCOD_DARKEST_ORANGE}},
 {{TCOD_DESATURATED_AMBER},{TCOD_LIGHTEST_AMBER},{TCOD_LIGHTER_AMBER},{TCOD_LIGHT_AMBER},{TCOD_AMBER},{TCOD_DARK_AMBER},{TCOD_DARKER_AMBER},{TCOD_DARKEST_AMBER}},
 {{TCOD_DESATURATED_YELLOW},{TCOD_LIGHTEST_YELLOW},{TCOD_LIGHTER_YELLOW},{TCOD_LIGHT_YELLOW},{TCOD_YELLOW},{TCOD_DARK_YELLOW},{TCOD_DARKER_YELLOW},{TCOD_DARKEST_YELLOW}},
 {{TCOD_DESATURATED_LIME},{TCOD_LIGHTEST_LIME},{TCOD_LIGHTER_LIME},{TCOD_LIGHT_LIME},{TCOD_LIME},{TCOD_DARK_LIME},{TCOD_DARKER_LIME},{TCOD_DARKEST_LIME}},
 {{TCOD_DESATURATED_CHARTREUSE},{TCOD_LIGHTEST_CHARTREUSE},{TCOD_LIGHTER_CHARTREUSE},{TCOD_LIGHT_CHARTREUSE},{TCOD_CHARTREUSE},{TCOD_DARK_CHARTREUSE},{TCOD_DARKER_CHARTREUSE},{TCOD_DARKEST_CHARTREUSE}},
 {{TCOD_DESATURATED_GREEN},{TCOD_LIGHTEST_GREEN},{TCOD_LIGHTER_GREEN},{TCOD_LIGHT_GREEN},{TCOD_GREEN},{TCOD_DARK_GREEN},{TCOD_DARKER_GREEN},{TCOD_DARKEST_GREEN}},
 {{TCOD_DESATURATED_SEA},{TCOD_LIGHTEST_SEA},{TCOD_LIGHTER_SEA},{TCOD_LIGHT_SEA},{TCOD_SEA},{TCOD_DARK_SEA},{TCOD_DARKER_SEA},{TCOD_DARKEST_SEA}},
 {{TCOD_DESATURATED_TURQUOISE},{TCOD_LIGHTEST_TURQUOISE},{TCOD_LIGHTER_TURQUOISE},{TCOD_LIGHT_TURQUOISE},{TCOD_TURQUOISE},{TCOD_DARK_TURQUOISE},{TCOD_DARKER_TURQUOISE},{TCOD_DARKEST_TURQUOISE}},
 {{TCOD_DESATURATED_CYAN},{TCOD_LIGHTEST_CYAN},{TCOD_LIGHTER_CYAN},{TCOD_LIGHT_CYAN},{TCOD_CYAN},{TCOD_DARK_CYAN},{TCOD_DARKER_CYAN},{TCOD_DARKEST_CYAN}},
 {{TCOD_DESATURATED_SKY},{TCOD_LIGHTEST_SKY},{TCOD_LIGHTER_SKY},{TCOD_LIGHT_SKY},{TCOD_SKY},{TCOD_DARK_SKY},{TCOD_DARKER_SKY},{TCOD_DARKEST_SKY}},
 {{TCOD_DESATURATED_AZURE},{TCOD_LIGHTEST_AZURE},{TCOD_LIGHTER_AZURE},{TCOD_LIGHT_AZURE},{TCOD_AZURE},{TCOD_DARK_AZURE},{TCOD_DARKER_AZURE},{TCOD_DARKEST_AZURE}},
 {{TCOD_DESATURATED_BLUE},{TCOD_LIGHTEST_BLUE},{TCOD_LIGHTER_BLUE},{TCOD_LIGHT_BLUE},{TCOD_BLUE},{TCOD_DARK_BLUE},{TCOD_DARKER_BLUE},{TCOD_DARKEST_BLUE}},
 {{TCOD_DESATURATED_HAN},{TCOD_LIGHTEST_HAN},{TCOD_LIGHTER_HAN},{TCOD_LIGHT_HAN},{TCOD_HAN},{TCOD_DARK_HAN},{TCOD_DARKER_HAN},{TCOD_DARKEST_HAN}},
 {{TCOD_DESATURATED_VIOLET},{TCOD_LIGHTEST_VIOLET},{TCOD_LIGHTER_VIOLET},{TCOD_LIGHT_VIOLET},{TCOD_VIOLET},{TCOD_DARK_VIOLET},{TCOD_DARKER_VIOLET},{TCOD_DARKEST_VIOLET}},
 {{TCOD_DESATURATED_PURPLE},{TCOD_LIGHTEST_PURPLE},{TCOD_LIGHTER_PURPLE},{TCOD_LIGHT_PURPLE},{TCOD_PURPLE},{TCOD_DARK_PURPLE},{TCOD_DARKER_PURPLE},{TCOD_DARKEST_PURPLE}},
 {{TCOD_DESATURATED_FUCHSIA},{TCOD_LIGHTEST_FUCHSIA},{TCOD_LIGHTER_FUCHSIA},{TCOD_LIGHT_FUCHSIA},{TCOD_FUCHSIA},{TCOD_DARK_FUCHSIA},{TCOD_DARKER_FUCHSIA},{TCOD_DARKEST_FUCHSIA}},
 {{TCOD_DESATURATED_MAGENTA},{TCOD_LIGHTEST_MAGENTA},{TCOD_LIGHTER_MAGENTA},{TCOD_LIGHT_MAGENTA},{TCOD_MAGENTA},{TCOD_DARK_MAGENTA},{TCOD_DARKER_MAGENTA},{TCOD_DARKEST_MAGENTA}},
 {{TCOD_DESATURATED_PINK},{TCOD_LIGHTEST_PINK},{TCOD_LIGHTER_PINK},{TCOD_LIGHT_PINK},{TCOD_PINK},{TCOD_DARK_PINK},{TCOD_DARKER_PINK},{TCOD_DARKEST_PINK}},
 {{TCOD_DESATURATED_CRIMSON},{TCOD_LIGHTEST_CRIMSON},{TCOD_LIGHTER_CRIMSON},{TCOD_LIGHT_CRIMSON},{TCOD_CRIMSON},{TCOD_DARK_CRIMSON},{TCOD_DARKER_CRIMSON},{TCOD_DARKEST_CRIMSON}}
};

TCOD_color_t TCOD_color_RGB(uint8 r, uint8 g, uint8 b) {
	TCOD_color_t ret = { r, g, b };
	return ret;
}

TCOD_color_t TCOD_color_HSV(float h, float s, float v) {
	TCOD_color_t ret;
	int i;
	float f, p, q, t;

	if( s == 0 ) {
		/* achromatic (grey) */
		ret.r = ret.g = ret.b = (uint8)(v*255.0f+0.5f);
	}
	else {
		while (h < 0.0f) h += 360.0f; /*for H < 0 */
		while (h >= 360.0f) h -= 360.0f; /*for H >= 360 */
		h /= 60;
		i = (int)(h); /*hue sector 0-5 */
		f = h - i;			/* factorial part of h */
		p = v * ( 1 - s );
		q = v * ( 1 - s * f );
		t = v * ( 1 - s * ( 1 - f ) );

		switch (i) {
			case 0:
				ret.r = (uint8)(v*255.0f+0.5f);
				ret.g = (uint8)(t*255.0f+0.5f);
				ret.b = (uint8)(p*255.0f+0.5f);
				break;
			case 1:
				ret.r = (uint8)(q*255.0f+0.5f);
				ret.g = (uint8)(v*255.0f+0.5f);
				ret.b = (uint8)(p*255.0f+0.5f);
				break;
			case 2:
				ret.r = (uint8)(p*255.0f+0.5f);
				ret.g = (uint8)(v*255.0f+0.5f);
				ret.b = (uint8)(t*255.0f+0.5f);
				break;
			case 3:
				ret.r = (uint8)(p*255.0f+0.5f);
				ret.g = (uint8)(q*255.0f+0.5f);
				ret.b = (uint8)(v*255.0f+0.5f);
				break;
			case 4:
				ret.r = (uint8)(t*255.0f+0.5f);
				ret.g = (uint8)(p*255.0f+0.5f);
				ret.b = (uint8)(v*255.0f+0.5f);
				break;
			default:
				ret.r = (uint8)(v*255.0f+0.5f);
				ret.g = (uint8)(p*255.0f+0.5f);
				ret.b = (uint8)(q*255.0f+0.5f);
				break;
		}
	}
	return ret;
}

bool TCOD_color_equals (TCOD_color_t c1, TCOD_color_t  c2) {
	return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b);
}

TCOD_color_t TCOD_color_add (TCOD_color_t c1, TCOD_color_t c2) {
	TCOD_color_t ret;
	int r,g,b;
	r = (int)(c1.r) + c2.r;
	g = (int)(c1.g) + c2.g;
	b = (int)(c1.b) + c2.b;
	r=MIN(255,r);
	g=MIN(255,g);
	b=MIN(255,b);
	ret.r=(uint8)r;
	ret.g=(uint8)g;
	ret.b=(uint8)b;
	return ret;
}

TCOD_color_t TCOD_color_subtract (TCOD_color_t c1, TCOD_color_t c2) {
	TCOD_color_t ret;
	int r,g,b;
	r = (int)(c1.r) - c2.r;
	g = (int)(c1.g) - c2.g;
	b = (int)(c1.b) - c2.b;
	r=MAX(0,r);
	g=MAX(0,g);
	b=MAX(0,b);
	ret.r=(uint8)r;
	ret.g=(uint8)g;
	ret.b=(uint8)b;
	return ret;
}

TCOD_color_t TCOD_color_multiply (TCOD_color_t c1, TCOD_color_t c2) {
	TCOD_color_t ret;
	ret.r=(uint8)(((int)c1.r)*c2.r/255);
	ret.g=(uint8)(((int)c1.g)*c2.g/255);
	ret.b=(uint8)(((int)c1.b)*c2.b/255);
	return ret;
}

TCOD_color_t TCOD_color_multiply_scalar (TCOD_color_t c1, float value) {
	TCOD_color_t ret;
	int r,g,b;
	r = (int)(c1.r * value);
	g = (int)(c1.g * value);
	b = (int)(c1.b * value);
	ret.r=(uint8)CLAMP(0,255,r);
	ret.g=(uint8)CLAMP(0,255,g);
	ret.b=(uint8)CLAMP(0,255,b);
	return ret;
}

TCOD_color_t TCOD_color_lerp(TCOD_color_t c1, TCOD_color_t c2, float coef) {
	TCOD_color_t ret;
	ret.r=(uint8)(c1.r+(c2.r-c1.r)*coef);
	ret.g=(uint8)(c1.g+(c2.g-c1.g)*coef);
	ret.b=(uint8)(c1.b+(c2.b-c1.b)*coef);
	return ret;
}

/* 0<= h < 360, 0 <= s <= 1, 0 <= v <= 1 */
void TCOD_color_set_HSV(TCOD_color_t *c, float h, float s, float v)
{
	int i;
	float f, p, q, t;

	if( s == 0.0f ) {
		/* achromatic (grey) */
		c->r = c->g = c->b = (uint8)(v*255.0f+0.5f);
		return;
	}

	while (h < 0.0f) h += 360.0f; /*for H < 0 */
	while (h >= 360.0f) h -= 360.0f; /*for H >= 360 */
	h /= 60.0f;			/* sector 0 to 5 */
	i = (int)floor( h );
	f = h - i;			/* factorial part of h */
	p = v * ( 1 - s );
	q = v * ( 1 - s * f );
	t = v * ( 1 - s * ( 1 - f ) );

	switch( i ) {
		case 0:
			c->r = (uint8)(v*255.0f+0.5f);
			c->g = (uint8)(t*255.0f+0.5f);
			c->b = (uint8)(p*255.0f+0.5f);
			break;
		case 1:
			c->r = (uint8)(q*255.0f+0.5f);
			c->g = (uint8)(v*255.0f+0.5f);
			c->b = (uint8)(p*255.0f+0.5f);
			break;
		case 2:
			c->r = (uint8)(p*255.0f+0.5f);
			c->g = (uint8)(v*255.0f+0.5f);
			c->b = (uint8)(t*255.0f+0.5f);
			break;
		case 3:
			c->r = (uint8)(p*255.0f+0.5f);
			c->g = (uint8)(q*255.0f+0.5f);
			c->b = (uint8)(v*255.0f+0.5f);
			break;
		case 4:
			c->r = (uint8)(t*255.0f+0.5f);
			c->g = (uint8)(p*255.0f+0.5f);
			c->b = (uint8)(v*255.0f+0.5f);
			break;
		default:
			c->r = (uint8)(v*255.0f+0.5f);
			c->g = (uint8)(p*255.0f+0.5f);
			c->b = (uint8)(q*255.0f+0.5f);
			break;
	}
}

void TCOD_color_get_HSV(TCOD_color_t c, float *h, float *s, float *v)
{
  uint8 imax,imin;
	float min, max, delta;

	imax = ( c.r > c.g ?
			( c.r > c.b ? c.r : c.b )
			: ( c.g > c.b ? c.g : c.b) );
	imin = ( c.r < c.g ?
			( c.r < c.b ? c.r : c.b )
			: ( c.g < c.b ? c.g : c.b) );
	max = imax/255.0f;
	min = imin/255.0f;
	*v = max; /* v */

	delta = max - min;
	if( max != 0.0f ) *s = delta / max; /* s */
	else
	{
		*s = 0.0f; /* s */
		*h = 0.0f; /* h */
		return;
	}

	if( c.r == imax ) *h = ( c.g - c.b ) / (255.0f * delta);		/* between yellow & magenta */
	else if( c.g == imax )	*h = 2.0f + ( c.b - c.r ) / (255.0f * delta);	/* between cyan & yellow */
	else *h = 4.0f + ( c.r - c.g ) / (255.0f * delta);	/* between magenta & cyan */

	*h *= 60.0f; /* degrees */
	if( *h < 0 ) *h += 360.0f;
}

float TCOD_color_get_hue (TCOD_color_t c) {
	uint8 max = MAX(c.r,MAX(c.g,c.b));
	uint8 min = MIN(c.r,MIN(c.g,c.b));
	float delta = (float)max - (float)min;
	float ret;
	if (delta == 0.0f) ret = 0.0f; /*achromatic, including black */
	else {
		if (c.r == max) ret = (float)(c.g - c.b) / delta;
		else if (c.g == max) ret = 2.0f + (float)(c.b - c.r) / delta;
		else ret = 4.0f + (float)(c.r - c.g) / delta;
		ret *= 60.0f;
		if (ret < 0.0f) ret += 360.0f;
		if (ret >= 360.0f) ret -= 360.0f;
	}
	return ret;
}

void TCOD_color_set_hue (TCOD_color_t *c, float h) {
	float obsolete, s, v;
	TCOD_color_get_HSV(*c,&obsolete,&s,&v);
	*c = TCOD_color_HSV(h,s,v);
}

float TCOD_color_get_saturation (TCOD_color_t c) {
	float max = (float)(MAX(c.r,MAX(c.g,c.b)))/255.0f;
	float min = (float)(MIN(c.r,MIN(c.g,c.b)))/255.0f;
	float delta = max - min;
	if (max == 0.0f) return 0.0f;
	else return delta/max;
}

void TCOD_color_set_saturation (TCOD_color_t *c, float s) {
	float h, obsolete, v;
	TCOD_color_get_HSV(*c,&h,&obsolete,&v);
	*c = TCOD_color_HSV(h,s,v);
}

float TCOD_color_get_value (TCOD_color_t c) {
	return (float)(MAX(c.r,MAX(c.g,c.b)))/255.0f;
}

void TCOD_color_set_value (TCOD_color_t *c, float v) {
	float h, s, obsolete;
	TCOD_color_get_HSV(*c,&h,&s,&obsolete);
	*c = TCOD_color_HSV(h,s,v);
}

void TCOD_color_shift_hue (TCOD_color_t *c, float hshift) {
	float h, s, v;
	if (hshift == 0.0f) return;
	TCOD_color_get_HSV(*c,&h,&s,&v);
	*c = TCOD_color_HSV(h+hshift,s,v);
}

void TCOD_color_scale_HSV (TCOD_color_t *c, float scoef, float vcoef) {
	float h, s, v;
	TCOD_color_get_HSV(*c,&h,&s,&v);
	s = CLAMP(0.0f,1.0f,s*scoef);
	v = CLAMP(0.0f,1.0f,v*vcoef);
	*c = TCOD_color_HSV(h,s,v);
}

void TCOD_color_gen_map(TCOD_color_t *map, int nb_key, TCOD_color_t const  *key_color, int const  *key_index) {
	int segment=0;
	for (segment=0; segment < nb_key-1; segment++) {
		int idx_start=key_index[segment];
		int idx_end=key_index[segment+1];
		int idx;
		for ( idx=idx_start;idx <= idx_end; idx++) {
			map[idx]=TCOD_color_lerp(key_color[segment],key_color[segment+1],(float)(idx-idx_start)/(idx_end-idx_start));
		}
	}
}


