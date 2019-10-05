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
#include "libtcod.h"
#include "libtcod_int.h"
#include <string.h>

typedef struct _ray_data_t {
	int xloc,yloc; /* position */
	int xob,yob; /* obscurity vector */
	int xerr,yerr; /* bresenham error */
	struct _ray_data_t * xinput, * yinput; /* offset of input rays */
	bool added; /* already in the fov */
	bool ignore; /* non visible. don't bother processing it */
} ray_data_t;

static int origx,origy; /* fov origin */
static ray_data_t **raymap; /* result rays */
static ray_data_t *raymap2; /* temporary rays */
static int perimidx;

static ray_data_t *new_ray(map_t *m,int x, int y) {
    ray_data_t *r;
	if ( (unsigned) (x+origx) >= (unsigned)m->width ) return NULL;
	if ( (unsigned) (y+origy) >= (unsigned)m->height ) return NULL;
	r=&raymap2[ x+origx + (y+origy)*m->width ];
	r->xloc=x;
	r->yloc=y;
	return r;
}

static void processRay(map_t *m, TCOD_list_t perim, ray_data_t *new_ray, ray_data_t *input_ray) {
	if ( new_ray ) {
		int mapx=origx+new_ray->xloc;
		int mapy=origy+new_ray->yloc;
		int newrayidx;
		newrayidx=mapx+mapy*m->width;
		if ( new_ray->yloc == input_ray->yloc ) new_ray->xinput=input_ray;
		else new_ray->yinput=input_ray;
		if (! new_ray->added) {
			TCOD_list_push(perim,new_ray);
			new_ray->added=true;
			raymap[newrayidx] = new_ray;
		}
	}
}

#define IS_OBSCURE(r) ((r->xerr > 0 && r->xerr <= r->xob) || (r->yerr > 0 && r->yerr <= r->yob) )

static void process_x_input(ray_data_t *new_ray, ray_data_t *xinput) {
	if ( xinput->xob == 0 && xinput->yob == 0 ) return;
	if ( xinput->xerr > 0 && new_ray->xob == 0) {
		new_ray->xerr = xinput->xerr - xinput->yob;
		new_ray->yerr = xinput->yerr + xinput->yob;
		new_ray->xob=xinput->xob;
		new_ray->yob=xinput->yob;
	}
	if ( xinput->yerr <= 0 && xinput->yob > 0 && xinput->xerr > 0) {
		new_ray->yerr = xinput->yerr + xinput->yob;
		new_ray->xerr = xinput->xerr - xinput->yob;
		new_ray->xob=xinput->xob;
		new_ray->yob=xinput->yob;
	}
}

static void process_y_input(ray_data_t *new_ray, ray_data_t *yinput) {
	if ( yinput->xob == 0 && yinput->yob == 0 ) return;
	if ( yinput->yerr > 0 && new_ray->yob == 0) {
		new_ray->yerr = yinput->yerr - yinput->xob;
		new_ray->xerr = yinput->xerr + yinput->xob;
		new_ray->xob=yinput->xob;
		new_ray->yob=yinput->yob;
	}
	if ( yinput->xerr <= 0 && yinput->xob > 0 && yinput->yerr > 0) {
		new_ray->yerr = yinput->yerr - yinput->xob;
		new_ray->xerr = yinput->xerr + yinput->xob;
		new_ray->xob=yinput->xob;
		new_ray->yob=yinput->yob;
	}
}

static void merge_input(map_t *m, ray_data_t *r) {
	int rayidx=r->xloc+origx+(r->yloc+origy)*m->width;
	ray_data_t *xi=r->xinput;
	ray_data_t *yi=r->yinput;
	if ( xi ) process_x_input(r,xi);
	if ( yi ) process_y_input(r,yi);
	if ( ! xi ) {
		if ( IS_OBSCURE(yi) ) r->ignore=true;
	} else if ( ! yi ) {
		if ( IS_OBSCURE(xi) ) r->ignore=true;
	} else if ( IS_OBSCURE(xi) && IS_OBSCURE(yi) ) {
		r->ignore=true;
	}
	if (! r->ignore && !m->cells[rayidx].transparent) {
		r->xerr = r->xob = ABS(r->xloc);
		r->yerr = r->yob = ABS(r->yloc);
	}
}

static void expandPerimeterFrom(map_t *m,TCOD_list_t perim,ray_data_t *r) {
	if ( r->xloc >= 0 ) {
		processRay(m,perim,new_ray(m,r->xloc+1,r->yloc),r);
	}
	if ( r->xloc <= 0 ) {
		processRay(m,perim,new_ray(m,r->xloc-1,r->yloc),r);
	}
	if ( r->yloc >= 0 ) {
		processRay(m,perim,new_ray(m,r->xloc,r->yloc+1),r);
	}
	if ( r->yloc <= 0 ) {
		processRay(m,perim,new_ray(m,r->xloc,r->yloc-1),r);
	}
}


void TCOD_map_compute_fov_diamond_raycasting(TCOD_map_t map, int player_x, int player_y, int max_radius, bool light_walls) {
	map_t *m = (map_t *)map;
	TCOD_list_t perim=TCOD_list_allocate(m->nbcells);
	cell_t *c;
	ray_data_t **r;
	int nbcells;
	int r2=max_radius*max_radius;

	perimidx=0;
	raymap=(ray_data_t **)calloc(sizeof(ray_data_t*),m->nbcells);
	raymap2=(ray_data_t *)calloc(sizeof(ray_data_t),m->nbcells);
	origx=player_x;
	origy=player_y;
	expandPerimeterFrom(m,perim,new_ray(m,0,0));
	while ( perimidx < TCOD_list_size(perim) ) {
		ray_data_t *ray=(ray_data_t *)TCOD_list_get(perim,perimidx);
		int distance = 0;
		if ( r2 > 0 ) distance = ((ray->xloc * ray->xloc) + (ray->yloc * ray->yloc));
		perimidx++;
		if ( distance <= r2) {
			merge_input(m, ray);
			if ( !ray->ignore ) expandPerimeterFrom(m,perim,ray);
		} else ray->ignore=true;
	}

	/* set fov data */
	c=m->cells;
	r=raymap;
	nbcells=m->nbcells;
	while ( nbcells!= 0 ) {
		if ( *r == NULL || (*r)->ignore
			|| ((*r)->xerr > 0 && (*r)->xerr <= (*r)->xob )
			|| ((*r)->yerr > 0 && (*r)->yerr <= (*r)->yob )
		) {
			c->fov=0;
		} else {
			c->fov=1;
		}
		c++;
		r++;
		nbcells--;
	}
	m->cells[origx+origy*m->width].fov=1;

	/* light walls */
	if ( light_walls ) {
		int xmin=0, ymin=0, xmax=m->width, ymax=m->height;
		if ( max_radius > 0 ) {
			xmin=MAX(0,player_x-max_radius);
			ymin=MAX(0,player_y-max_radius);
			xmax=MIN(m->width,player_x+max_radius+1);
			ymax=MIN(m->height,player_y+max_radius+1);
		}
		TCOD_map_postproc(m,xmin,ymin,player_x,player_y,-1,-1);
		TCOD_map_postproc(m,player_x,ymin,xmax-1,player_y,1,-1);
		TCOD_map_postproc(m,xmin,player_y,player_x,ymax-1,-1,1);
		TCOD_map_postproc(m,player_x,player_y,xmax-1,ymax-1,1,1);
	}

	free(raymap);
	free(raymap2);
	TCOD_list_delete(perim);
}
