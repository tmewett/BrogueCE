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

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "libtcod.h"
#include "libtcod_int.h"

static int mult[4][8]= {
	{1,0,0,-1,-1,0,0,1},
	{0,1,-1,0,0,-1,1,0},
	{0,1,1,0,0,-1,-1,0},
	{1,0,0,1,-1,0,0,-1},
};

static void cast_light(map_t *map,int cx, int cy,int row,float start, float end, int radius, int r2,
	int xx, int xy, int yx, int yy, int id, bool light_walls) {
	int j;
	float new_start=0.0f;
	if ( start < end ) return;
	for (j=row; j< radius+1; j++) {
		int dx=-j-1;
		int dy=-j;
		bool blocked=false;
		while ( dx <= 0 ) {
			int X,Y;
			dx++;
			X=cx+dx*xx+dy*xy;
			Y=cy+dx*yx+dy*yy;
			if ((unsigned)X < (unsigned)map->width && (unsigned)Y < (unsigned)map->height) {
				float l_slope,r_slope;
				int offset;
				offset=X+Y*map->width;
				l_slope=(dx-0.5f)/(dy+0.5f);
				r_slope=(dx+0.5f)/(dy-0.5f);
				if( start < r_slope ) continue;
				else if( end > l_slope ) break;
				if ( dx*dx+dy*dy <= r2
					&& (light_walls || map->cells[offset].transparent)) map->cells[offset].fov=1;
				if ( blocked ) {
					if (!map->cells[offset].transparent) {
						new_start=r_slope;
						continue;
					} else {
						blocked=false;
						start=new_start;
					}
				} else {
					if (!map->cells[offset].transparent && j < radius ) {
						blocked=true;
						cast_light(map,cx,cy,j+1,start,l_slope,radius,r2,xx,xy,yx,yy,id+1,light_walls);
						new_start=r_slope;
					}
				}
			}
		}
		if ( blocked ) break;
	}
}

void TCOD_map_compute_fov_recursive_shadowcasting(TCOD_map_t map, int player_x, int player_y, int max_radius, bool light_walls) {
	int oct,c,r2;
	map_t *m = (map_t *)map;
	/* clean the map */
	for (c=m->nbcells-1; c >= 0; c--) {
		m->cells[c].fov=0;
	}
	if ( max_radius == 0 ) {
		int max_radius_x=m->width-player_x;
		int max_radius_y=m->height-player_y;
		max_radius_x=MAX(max_radius_x,player_x);
		max_radius_y=MAX(max_radius_y,player_y);
		max_radius = (int)(sqrt(max_radius_x*max_radius_x+max_radius_y*max_radius_y))+1;
	}
	r2=max_radius*max_radius;
	/* recursive shadow casting */
	for (oct=0; oct < 8; oct++) cast_light(m,player_x,player_y,1,1.0,0.0,max_radius,r2,
		mult[0][oct],mult[1][oct],mult[2][oct],mult[3][oct],0,light_walls);
	m->cells[player_x+player_y*m->width].fov=1;
}

