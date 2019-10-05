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

#include "libtcod.h"
#include "libtcod_int.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

static void cast_ray(map_t *map, int xo, int yo, int xd, int yd, int r2,bool light_walls) {
	int curx=xo,cury=yo;
	bool in=false;
	bool blocked=false;
	bool end=false;
	int offset;
	TCOD_line_init(xo,yo,xd,yd);
	offset=curx+cury*map->width;
	if ( 0 <= offset && offset < map->nbcells ) {
		in=true;
		map->cells[offset].fov=1;
	}
	while (!end) {
		end = TCOD_line_step(&curx,&cury);	/* reached xd,yd */
		offset=curx+cury*map->width;
		if ( r2 > 0 ) {
			/* check radius */
			int cur_radius=(curx-xo)*(curx-xo)+(cury-yo)*(cury-yo);
			if ( cur_radius > r2 ) return;
		}
		if ( 0 <= offset && offset < map->nbcells ) {
			in=true;
			if ( !blocked && ! map->cells[offset].transparent ) {
				blocked=true;
			} else if ( blocked ) {
					return; /* wall */
			}
			if ( light_walls || ! blocked ) map->cells[offset].fov=1;
		} else if (in) return; /* ray out of map */
	}
}

void TCOD_map_postproc(map_t *map,int x0,int y0, int x1, int y1, int dx, int dy) {
	int cx,cy;
	for (cx=x0; cx <= x1; cx++) {
		for (cy=y0;cy <= y1; cy ++ ) {
			int x2 = cx+dx;
			int y2 = cy+dy;
			unsigned int offset=cx+cy*map->width;
			if ( offset < (unsigned)map->nbcells && map->cells[offset].fov == 1 
				&& map->cells[offset].transparent ) {
				if ( x2 >= x0 && x2 <= x1 ) {
					unsigned int offset2=x2+cy*map->width;
					if ( offset2 < (unsigned)map->nbcells && ! map->cells[offset2].transparent )
						map->cells[offset2].fov=1;
				}
				if ( y2 >= y0 && y2 <= y1 ) {
					unsigned int offset2=cx+y2*map->width;
					if ( offset2 < (unsigned)map->nbcells && ! map->cells[offset2].transparent )
						map->cells[offset2].fov=1;
				}
				if ( x2 >= x0 && x2 <= x1 && y2 >= y0 && y2 <= y1 ) {
					unsigned int offset2=x2+y2*map->width;
					if ( offset2 < (unsigned)map->nbcells && ! map->cells[offset2].transparent )
						map->cells[offset2].fov=1;
				}
			}
		}
	}
}


void TCOD_map_compute_fov_circular_raycastingi(TCOD_map_t map, int player_x, int player_y, int max_radius, bool light_walls) {
	int xo,yo;
	map_t *m = (map_t *)map;
	/* circular ray casting */
	int xmin=0, ymin=0, xmax=m->width, ymax=m->height;
	int c;
	int r2=max_radius*max_radius;
	if ( max_radius > 0 ) {
		xmin=MAX(0,player_x-max_radius);
		ymin=MAX(0,player_y-max_radius);
		xmax=MIN(m->width,player_x+max_radius+1);
		ymax=MIN(m->height,player_y+max_radius+1);
	}
	for (c=m->nbcells-1; c >= 0; c--) {
		m->cells[c].fov=0;
	}
	xo=xmin; yo=ymin;
	while ( xo < xmax ) {
		cast_ray(m,player_x,player_y,xo++,yo,r2,light_walls);
	}
	xo=xmax-1;yo=ymin+1;
	while ( yo < ymax ) {
		cast_ray(m,player_x,player_y,xo,yo++,r2,light_walls);
	}
	xo=xmax-2;yo=ymax-1;
	while ( xo >= 0 ) {
		cast_ray(m,player_x,player_y,xo--,yo,r2,light_walls);
	}
	xo=xmin;yo=ymax-2;
	while ( yo > 0 ) {
		cast_ray(m,player_x,player_y,xo,yo--,r2,light_walls);
	}
	if ( light_walls ) {
		/* post-processing artefact fix */
		TCOD_map_postproc(m,xmin,ymin,player_x,player_y,-1,-1);
		TCOD_map_postproc(m,player_x,ymin,xmax-1,player_y,1,-1);
		TCOD_map_postproc(m,xmin,player_y,player_x,ymax-1,-1,1);
		TCOD_map_postproc(m,player_x,player_y,xmax-1,ymax-1,1,1);
	}
}

#if 0
#define CELL_RADIUS 0.4f
#define RAY_RADIUS 0.2f
static bool ray_blocked(map_t *map,float x, float y, int cx, int cy) {
	int offset=cx+cy*map->width;
	float d;
	if ( (unsigned)offset >= (unsigned)map->nbcells ) return false; /* out of the map */
	if ( map->cells[offset].transparent ) return false; /* empty cell */
	d=(cx-x+0.5f)*(cx-x+0.5f)+(cy-y+0.5f)*(cy-y+0.5f);
	return d < (CELL_RADIUS+RAY_RADIUS)*(CELL_RADIUS+RAY_RADIUS);
}
static void cast_rayf(map_t *map, int xo, int yo, int xd, int yd, int r2,bool light_walls) {
	float fxo=xo+0.5f, fyo=yo+0.5f;
	float curx=fxo, cury=fyo;
	float fxd=xd+0.5f;
	float fyd=yd+0.5f;
	bool in=false;
	bool end=false;
	int offset;
	float dx=(float)(fxd-curx), dy=(float)(fyd-cury),idx,idy;
	if ( dx == 0 && dy == 0 ) return;
	if ( fabs(dx) > fabs(dy) ) {
		idy = (float)(dy/fabs(dx));
		idx = (float)(dx/fabs(dx));
	} else {
		idx = (float)(dx/fabs(dy));
		idy = (float)(dy/fabs(dy));
	}
	offset=(int)(curx)+(int)(cury)*map->width;
	if ( (unsigned)offset < (unsigned)map->nbcells ) {
		in=true;
		map->cells[offset].fov=1;
	}
	while (!end) {
		int cx,cy;
		curx+=idx;
		cury+=idy;
		cx=(int)curx;
		cy=(int)cury;
		end = (cx==xd && cy==yd);
		offset=cx+cy*map->width;
		if ( r2 > 0 ) {
			/* check radius */
			int cur_radius=(int)((curx-fxo)*(curx-fxo)+(cury-fyo)*(cury-fyo));
			if ( cur_radius > r2 ) return;
		}
		if ( (unsigned)offset < (unsigned)map->nbcells ) {
			in=true;
			if ( ray_blocked(map,curx,cury,cx,cy) ) return;
			if ( curx+RAY_RADIUS > cx+0.5f-CELL_RADIUS && ray_blocked(map,curx,cury,cx+1,cy) ) return;
			if ( curx-RAY_RADIUS < cx-0.5f+CELL_RADIUS && ray_blocked(map,curx,cury,cx-1,cy) ) return;
			if ( cury+RAY_RADIUS > cy+0.5f-CELL_RADIUS && ray_blocked(map,curx,cury,cx,cy+1) ) return;
			if ( cury-RAY_RADIUS < cy-0.5f+CELL_RADIUS && ray_blocked(map,curx,cury,cx,cy-1) ) return;
			map->cells[offset].fov=1;
		} else if (in) return; /* ray out of map */
	}
}
#endif

void TCOD_map_compute_fov_circular_raycasting(TCOD_map_t map, int player_x, int player_y, int max_radius, bool light_walls) {
	int xo,yo;
	map_t *m = (map_t *)map;
	/* circular ray casting */
	int xmin=0, ymin=0, xmax=m->width, ymax=m->height;
	int c;
	int r2=max_radius*max_radius;
	if ( max_radius > 0 ) {
		xmin=MAX(0,player_x-max_radius);
		ymin=MAX(0,player_y-max_radius);
		xmax=MIN(m->width,player_x+max_radius+1);
		ymax=MIN(m->height,player_y+max_radius+1);
	}
	for (c=m->nbcells-1; c >= 0; c--) {
		m->cells[c].fov=0;
	}
	xo=xmin; yo=ymin;
	while ( xo < xmax ) {
		cast_ray(m,player_x,player_y,xo++,yo,r2,light_walls);
	}
	xo=xmax-1;yo=ymin+1;
	while ( yo < ymax ) {
		cast_ray(m,player_x,player_y,xo,yo++,r2,light_walls);
	}
	xo=xmax-2;yo=ymax-1;
	while ( xo >= 0 ) {
		cast_ray(m,player_x,player_y,xo--,yo,r2,light_walls);
	}
	xo=xmin;yo=ymax-2;
	while ( yo > 0 ) {
		cast_ray(m,player_x,player_y,xo,yo--,r2,light_walls);
	}
	if ( light_walls ) {
		/* post-processing artefact fix */
		TCOD_map_postproc(m,xmin,ymin,player_x,player_y,-1,-1);
		TCOD_map_postproc(m,player_x,ymin,xmax-1,player_y,1,-1);
		TCOD_map_postproc(m,xmin,player_y,player_x,ymax-1,-1,1);
		TCOD_map_postproc(m,player_x,player_y,xmax-1,ymax-1,1,1);
	}
}

