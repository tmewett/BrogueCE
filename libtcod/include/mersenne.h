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

#ifndef _TCOD_RANDOM_H
#define _TCOD_RANDOM_H

#include "mersenne_types.h"

typedef void *TCOD_random_t;

TCODLIB_API TCOD_random_t TCOD_random_get_instance(void);
TCODLIB_API TCOD_random_t TCOD_random_new(TCOD_random_algo_t algo);
TCODLIB_API TCOD_random_t TCOD_random_save(TCOD_random_t mersenne);
TCODLIB_API void TCOD_random_restore(TCOD_random_t mersenne, TCOD_random_t backup);
TCODLIB_API TCOD_random_t TCOD_random_new_from_seed(TCOD_random_algo_t algo, uint32 seed);
TCODLIB_API void TCOD_random_delete(TCOD_random_t mersenne);

TCODLIB_API void TCOD_random_set_distribution (TCOD_random_t mersenne, TCOD_distribution_t distribution);

TCODLIB_API int TCOD_random_get_int (TCOD_random_t mersenne, int min, int max);
TCODLIB_API float TCOD_random_get_float (TCOD_random_t mersenne, float min, float max);
TCODLIB_API double TCOD_random_get_double (TCOD_random_t mersenne, double min, double max);

TCODLIB_API int TCOD_random_get_int_mean (TCOD_random_t mersenne, int min, int max, int mean);
TCODLIB_API float TCOD_random_get_float_mean (TCOD_random_t mersenne, float min, float max, float mean);
TCODLIB_API double TCOD_random_get_double_mean (TCOD_random_t mersenne, double min, double max, double mean);

TCODLIB_API TCOD_dice_t TCOD_random_dice_new (const char * s);
TCODLIB_API int TCOD_random_dice_roll (TCOD_random_t mersenne, TCOD_dice_t dice);
TCODLIB_API int TCOD_random_dice_roll_s (TCOD_random_t mersenne, const char * s);

#endif
