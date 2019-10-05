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
#ifndef _TCOD_HEIGHTMAP_H
#define _TCOD_HEIGHTMAP_H

typedef struct {
	int w,h;
	float *values;
} TCOD_heightmap_t;

TCODLIB_API TCOD_heightmap_t *TCOD_heightmap_new(int w,int h);
TCODLIB_API void TCOD_heightmap_delete(TCOD_heightmap_t *hm);

TCODLIB_API float TCOD_heightmap_get_value(const TCOD_heightmap_t *hm, int x, int y);
TCODLIB_API float TCOD_heightmap_get_interpolated_value(const TCOD_heightmap_t *hm, float x, float y);
TCODLIB_API void TCOD_heightmap_set_value(TCOD_heightmap_t *hm, int x, int y, float value);
TCODLIB_API float TCOD_heightmap_get_slope(const TCOD_heightmap_t *hm, int x, int y);
TCODLIB_API void TCOD_heightmap_get_normal(const TCOD_heightmap_t *hm, float x, float y, float n[3], float waterLevel);
TCODLIB_API int TCOD_heightmap_count_cells(const TCOD_heightmap_t *hm, float min, float max);
TCODLIB_API bool TCOD_heightmap_has_land_on_border(const TCOD_heightmap_t *hm, float waterLevel);
TCODLIB_API void TCOD_heightmap_get_minmax(const TCOD_heightmap_t *hm, float *min, float *max);

TCODLIB_API void TCOD_heightmap_copy(const TCOD_heightmap_t *hm_source,TCOD_heightmap_t *hm_dest);
TCODLIB_API void TCOD_heightmap_add(TCOD_heightmap_t *hm, float value);
TCODLIB_API void TCOD_heightmap_scale(TCOD_heightmap_t *hm, float value);
TCODLIB_API void TCOD_heightmap_clamp(TCOD_heightmap_t *hm, float min, float max);
TCODLIB_API void TCOD_heightmap_normalize(TCOD_heightmap_t *hm, float min, float max);
TCODLIB_API void TCOD_heightmap_clear(TCOD_heightmap_t *hm);
TCODLIB_API void TCOD_heightmap_lerp_hm(const TCOD_heightmap_t *hm1, const TCOD_heightmap_t *hm2, TCOD_heightmap_t *hmres, float coef);
TCODLIB_API void TCOD_heightmap_add_hm(const TCOD_heightmap_t *hm1, const TCOD_heightmap_t *hm2, TCOD_heightmap_t *hmres);
TCODLIB_API void TCOD_heightmap_multiply_hm(const TCOD_heightmap_t *hm1, const TCOD_heightmap_t *hm2, TCOD_heightmap_t *hmres);

TCODLIB_API void TCOD_heightmap_add_hill(TCOD_heightmap_t *hm, float hx, float hy, float hradius, float hheight);
TCODLIB_API void TCOD_heightmap_dig_hill(TCOD_heightmap_t *hm, float hx, float hy, float hradius, float hheight);
TCODLIB_API void TCOD_heightmap_dig_bezier(TCOD_heightmap_t *hm, int px[4], int py[4], float startRadius, float startDepth, float endRadius, float endDepth);
TCODLIB_API void TCOD_heightmap_rain_erosion(TCOD_heightmap_t *hm, int nbDrops,float erosionCoef,float sedimentationCoef,TCOD_random_t rnd);
/* TCODLIB_API void TCOD_heightmap_heat_erosion(TCOD_heightmap_t *hm, int nbPass,float minSlope,float erosionCoef,float sedimentationCoef,TCOD_random_t rnd); */
TCODLIB_API void TCOD_heightmap_kernel_transform(TCOD_heightmap_t *hm, int kernelsize, const int *dx, const int *dy, const float *weight, float minLevel,float maxLevel);
TCODLIB_API void TCOD_heightmap_add_voronoi(TCOD_heightmap_t *hm, int nbPoints, int nbCoef, const float *coef,TCOD_random_t rnd);
TCODLIB_API void TCOD_heightmap_mid_point_displacement(TCOD_heightmap_t *hm, TCOD_random_t rnd, float roughness);
TCODLIB_API void TCOD_heightmap_add_fbm(TCOD_heightmap_t *hm, TCOD_noise_t noise,float mulx, float muly, float addx, float addy, float octaves, float delta, float scale); 
TCODLIB_API void TCOD_heightmap_scale_fbm(TCOD_heightmap_t *hm, TCOD_noise_t noise,float mulx, float muly, float addx, float addy, float octaves, float delta, float scale); 
TCODLIB_API void TCOD_heightmap_islandify(TCOD_heightmap_t *hm, float seaLevel,TCOD_random_t rnd);

#endif

