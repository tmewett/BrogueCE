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
#include <stdlib.h>
#include <string.h>
#include "libtcod.h"

#define GET_VALUE(hm,x,y) (hm)->values[(x)+(y)*(hm)->w]

TCOD_heightmap_t *TCOD_heightmap_new(int w,int h) {
	TCOD_heightmap_t *hm=(TCOD_heightmap_t *)malloc(sizeof(TCOD_heightmap_t));
	hm->values = (float *)calloc(sizeof(float),w*h);
	hm->w=w;
	hm->h=h;
	return hm;
}
void TCOD_heightmap_delete(TCOD_heightmap_t *hm) {
	free(hm->values);
	free(hm);
}

void TCOD_heightmap_clear(TCOD_heightmap_t *hm) {
	memset(hm->values,0,hm->w*hm->h*sizeof(float));
}

float TCOD_heightmap_get_value(const TCOD_heightmap_t *hm, int x, int y) {
	return GET_VALUE(hm,x,y);
}

void TCOD_heightmap_set_value(TCOD_heightmap_t *hm, int x, int y, float value) {
	GET_VALUE(hm,x,y)=value;
}

void TCOD_heightmap_get_minmax(const TCOD_heightmap_t *hm, float *min, float *max) {
	float curmax=hm->values[0];
	float curmin=hm->values[0];
	int x,y;
	float *value = hm->values;
	/* get max and min height */
	for (y=0; y < hm->h; y++) {
		for (x=0; x < hm->w; x++) {
			float val=*value;
			if ( val > curmax ) curmax = val;
			else if ( val < curmin ) curmin = val;
			value++;
		}
	}
	*min=curmin;
	*max=curmax;
}

void TCOD_heightmap_normalize(TCOD_heightmap_t *hm, float min, float max) {
	float curmin,curmax;
	int x,y;
	float invmax;
	float *value = hm->values;
	TCOD_heightmap_get_minmax(hm,&curmin,&curmax);
	if (curmax - curmin == 0.0f) invmax=0.0f;
	else invmax = (max-min) / (curmax-curmin);
	/* normalize */
	for (y=0; y < hm->h; y++) {
		for (x=0; x < hm->w; x++) {
			*value = min + (*value - curmin) * invmax ;
			value++;			
		}
	}
}
void TCOD_heightmap_add_hill(TCOD_heightmap_t *hm, float hx, float hy, float hradius, float hheight) {
	int x,y;
	float hradius2=hradius*hradius;
	float coef=hheight / hradius2;
	int minx=(int)MAX(0,hx-hradius);
	int maxx=(int)MIN(hm->w,hx+hradius);
	int miny=(int)MAX(0,hy-hradius);
	int maxy=(int)MIN(hm->h,hy+hradius);
	for (x=minx; x < maxx; x++) {
		float xdist=( x - hx )*( x - hx );
		for (y=miny; y < maxy; y++) {
			float z = hradius2 - xdist - (y - hy)*(y - hy);
			if ( z > 0.0 ) GET_VALUE(hm,x,y) += z * coef;
		}
	}
}

void TCOD_heightmap_dig_hill(TCOD_heightmap_t *hm, float hx, float hy, float hradius, float hheight) {
	int x,y;
	float hradius2=hradius*hradius;
	float coef=hheight / hradius2;
	int minx=(int)MAX(0,hx-hradius);
	int maxx=(int)MIN(hm->w,hx+hradius);
	int miny=(int)MAX(0,hy-hradius);
	int maxy=(int)MIN(hm->h,hy+hradius);
	for (x=minx; x < maxx; x++) {
		float xdist=( x - hx )*( x - hx );
		for (y=miny; y < maxy; y++) {
			float dist=xdist + (y - hy)*(y - hy);
			if ( dist < hradius2 ) {
				float z = (hradius2 - dist) * coef;
				if ( hheight > 0.0 ) {
					if ( GET_VALUE(hm,x,y) < z ) GET_VALUE(hm,x,y) = z;
				} else {
					if ( GET_VALUE(hm,x,y) > z ) GET_VALUE(hm,x,y) = z;
				}
			}
		}
	}
}

void TCOD_heightmap_copy(const TCOD_heightmap_t *hm_source,TCOD_heightmap_t *hm_dest) {
	if ( hm_source->w != hm_dest->w || hm_source->h != hm_dest->h) return;
	memcpy(hm_dest->values,hm_source->values,sizeof(float)*hm_source->w*hm_source->h);
}

void TCOD_heightmap_add_fbm(TCOD_heightmap_t *hm, TCOD_noise_t noise,float mulx, float muly, float addx, float addy, float octaves, float delta, float scale) {
	int x,y;
	float xcoef=mulx / hm->w;
	float ycoef=muly / hm->h;
	float min=1.0f;
	float max=0.0f;
	for (x=0; x < hm->w; x++) {
		float f[2];
		int offset=x;
		f[0] = (x + addx) * xcoef;
		for (y=0; y < hm->h; y++) {
            float value;
			f[1] = (y + addy)*ycoef;
			value=delta+TCOD_noise_get_fbm(noise,f,octaves)*scale;
			hm->values[offset] += value;
			if ( value < min ) min = value;
			if ( value > max ) max=value;
			offset+=hm->w;
		}
	}
}
void TCOD_heightmap_scale_fbm(TCOD_heightmap_t *hm, TCOD_noise_t noise,float mulx, float muly, float addx, float addy, float octaves, float delta, float scale) {
	int x,y;
	float xcoef=mulx / hm->w;
	float ycoef=muly / hm->h;
	for (x=0; x < hm->w; x++) {
		float f[2];
		int offset=x;
		f[0] = (x + addx) * xcoef;
		for (y=0; y < hm->h; y++) {
			f[1] = (y + addy)*ycoef;
			hm->values[offset] *= (delta+TCOD_noise_get_fbm(noise,f,octaves)*scale);
			offset+=hm->w;
		}
	}
}

float TCOD_heightmap_get_interpolated_value(const TCOD_heightmap_t *hm, float x, float y) {
	int ix = (int) x;
	int iy = (int) y;
	if ( ix >= hm->w-1 || iy >= hm->h-1 ) {
		return GET_VALUE(hm,ix,iy);
	} else {
		float dx = x - ix;
		float dy = y - iy;
		float c1 = GET_VALUE(hm,ix,iy);
		float c2 = GET_VALUE(hm,ix+1,iy);
		float c3 = GET_VALUE(hm,ix,iy+1);
		float c4 = GET_VALUE(hm,ix+1,iy+1);
		float top = (1.0f-dx)*c1 + dx*c2;
		float bottom = (1.0f-dx)*c3 + dx*c4;
		return (1.0f-dy)*top + dy*bottom;
	}
}

void TCOD_heightmap_get_normal(const TCOD_heightmap_t *hm, float x, float y, float n[3], float waterLevel) {
	float h0,hx,hy,invlen; /* map heights at x,y x+1,y and x,y+1 */
	n[0]=0.0f;n[1]=0.0f;n[2]=1.0f;
	if ( x >= hm->w-1 || y >= hm->h-1 ) return;
	h0 = TCOD_heightmap_get_interpolated_value(hm,x,y);
	if ( h0 < waterLevel ) h0=waterLevel;
	hx = TCOD_heightmap_get_interpolated_value(hm,x+1,y);
	if ( hx < waterLevel ) hx=waterLevel;
	hy = TCOD_heightmap_get_interpolated_value(hm,x,y+1);
	if ( hy < waterLevel ) hy=waterLevel;
	/* vx = 1       vy = 0 */
	/*      0            1 */
	/*      hx-h0        hy-h0 */
	/* vz = vx cross vy */
	n[0] = 255*(h0-hx);
	n[1] = 255*(h0-hy);
	n[2] = 16.0f;
	/* normalize */
	invlen=1.0f / (float)sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
	n[0]*=invlen;
	n[1]*=invlen;
	n[2]*=invlen;
}

void TCOD_heightmap_dig_bezier(TCOD_heightmap_t *hm, int px[4], int py[4], float startRadius, float startDepth, float endRadius, float endDepth) {
	int xFrom=px[0];
	int yFrom=py[0];
	float t;
	for ( t=0.0f;t <= 1.0f ; t += 0.001f ) {
		int xTo,yTo;
		float it=1.0f-t;
		xTo=(int)(px[0]*it*it*it + 3*px[1]*t*it*it + 3*px[2]*t*t*it + px[3]*t*t*t);
		yTo=(int)(py[0]*it*it*it + 3*py[1]*t*it*it + 3*py[2]*t*t*it + py[3]*t*t*t);
		if ( xTo != xFrom || yTo != yFrom ) {
			float radius=startRadius+(endRadius-startRadius)*t;
			float depth=startDepth+(endDepth-startDepth)*t;
			TCOD_heightmap_dig_hill(hm,(float)xTo,(float)yTo,radius,depth);
			xFrom=xTo;
			yFrom=yTo;
		}
	}
}

bool TCOD_heightmap_has_land_on_border(const TCOD_heightmap_t *hm, float waterLevel) {
	int x,y;
	for (x=0; x < hm->w; x++) {
		if ( GET_VALUE(hm,x,0) > waterLevel
			|| GET_VALUE(hm,x,hm->h-1) > waterLevel ) {
			return true;
		}
	}
	for (y=0; y < hm->h; y++) {
		if ( GET_VALUE(hm,0,y) > waterLevel
			|| GET_VALUE(hm,hm->w-1,y) > waterLevel ) {
			return true;
		}
	}
	return false;
}

void TCOD_heightmap_islandify(TCOD_heightmap_t *hm, float seaLevel,TCOD_random_t rnd) {
	/* TODO */
}

void TCOD_heightmap_add(TCOD_heightmap_t *hm, float value) {
	int i;
	for (i=0; i < hm->w*hm->h; i++ ) hm->values[i]+=value;
}

int TCOD_heightmap_count_cells(const TCOD_heightmap_t *hm, float min, float max) {
	int count=0,i;
	for (i=0; i < hm->w*hm->h; i++ ) if (hm->values[i] >= min && hm->values[i] <= max) count++;
	return count;
}

void TCOD_heightmap_scale(TCOD_heightmap_t *hm, float value) {
	int i;
	for (i=0; i < hm->w*hm->h; i++ ) hm->values[i]*=value;
}

void TCOD_heightmap_clamp(TCOD_heightmap_t *hm, float min, float max) {
	int i;
	for (i=0; i < hm->w*hm->h; i++ ) {
		hm->values[i]=CLAMP(min,max,hm->values[i]);
	}
}

void TCOD_heightmap_lerp_hm(const TCOD_heightmap_t *hm1, const TCOD_heightmap_t *hm2, TCOD_heightmap_t *hmres, float coef) {
	int i;
	if ( hm1->w != hm2->w || hm1->h != hm2->h
		||  hm1->w != hmres->w || hm1->h != hmres->h
	) {
		return;
	}
	for (i=0; i < hm1->w*hm1->h; i++ ) {
		hmres->values[i]=hm1->values[i]+(hm2->values[i]-hm1->values[i])*coef;
	}
}

void TCOD_heightmap_add_hm(const TCOD_heightmap_t *hm1, const TCOD_heightmap_t *hm2, TCOD_heightmap_t *hmres) {
	int i;
	if ( hm1->w != hm2->w || hm1->h != hm2->h
		||  hm1->w != hmres->w || hm1->h != hmres->h
	) {
		return;
	}
	for (i=0; i < hm1->w*hm1->h; i++ ) {
		hmres->values[i]=hm1->values[i]+hm2->values[i];
	}
}

void TCOD_heightmap_multiply_hm(const TCOD_heightmap_t *hm1, const TCOD_heightmap_t *hm2, TCOD_heightmap_t *hmres) {
	int i;
	if ( hm1->w != hm2->w || hm1->h != hm2->h
		||  hm1->w != hmres->w || hm1->h != hmres->h
	) {
		return;
	}
	for (i=0; i < hm1->w*hm1->h; i++ ) {
		hmres->values[i]=hm1->values[i]*hm2->values[i];
	}
}


float TCOD_heightmap_get_slope(const TCOD_heightmap_t *hm, int x, int y) {
	static int dix[8]={-1,0,1,-1,1,-1,0,1};
	static int diy[8]={-1,-1,-1,0,0,1,1,1};
	float mindy=0.0f,maxdy=0.0f;
	float v=GET_VALUE(hm,x,y);
	int i;
	for (i=0; i < 8; i++ ) {
		int nx=x+dix[i];
		int ny=y+diy[i];
		if ( nx >= 0 && nx < hm->w && ny >= 0 && ny < hm->h ) {
			float nslope=(GET_VALUE(hm,nx,ny)-v);
			if ( nslope > maxdy ) {
				maxdy=nslope;
			} else if ( nslope < mindy ) {
				mindy=nslope;
			}
		}
	}
	return (float)atan2(maxdy+mindy,1.0f);
}

void TCOD_heightmap_rain_erosion(TCOD_heightmap_t *hm, int nbDrops,float erosionCoef,float agregationCoef,TCOD_random_t rnd) {
	while ( nbDrops > 0 ) {
		int curx=TCOD_random_get_int(rnd,0,hm->w-1);
		int cury=TCOD_random_get_int(rnd,0,hm->h-1);
		static int dx[8]={-1,0,1,-1,1,-1,0,1};
		static int dy[8]={-1,-1,-1,0,0,1,1,1};
		float slope=0.0f;
		float sediment=0.0f;
		do {
			int nextx=0,nexty=0,i;
			float v=GET_VALUE(hm,curx,cury);
			/* calculate slope at x,y */
			slope=0.0f;
			for (i=0; i < 8; i++ ) {
				int nx=curx+dx[i];
				int ny=cury+dy[i];
				if ( nx >= 0 && nx < hm->w && ny >= 0 && ny < hm->h ) {
					float nslope=v-GET_VALUE(hm,nx,ny);
					if ( nslope > slope ) {
						slope=nslope;
						nextx=nx;
						nexty=ny;
					}
				}
			}
			if ( slope > 0.0f ) {
/*				GET_VALUE(hm,curx,cury) *= 1.0f - (erosionCoef * slope); */
				GET_VALUE(hm,curx,cury) -= erosionCoef * slope;
				curx=nextx;
				cury=nexty;
				sediment+=slope;
			} else {
/*				GET_VALUE(hm,curx,cury) *= 1.0f + (agregationCoef*sediment); */
				GET_VALUE(hm,curx,cury) += agregationCoef*sediment;
			}
		} while ( slope > 0.0f );
		nbDrops--;
	}
}

#if 0
static void setMPDHeight(TCOD_heightmap_t *hm, TCOD_random_t rnd,int x,int y, float z, float offset);
static void setMDPHeightSquare(TCOD_heightmap_t *hm, TCOD_random_t rnd,int x, int y, int initsz, int sz,float offset);

void TCOD_heightmap_heat_erosion(TCOD_heightmap_t *hm, int nbPass,float minSlope,float erosionCoef,float agregationCoef,TCOD_random_t rnd) {
	int x;
	while ( nbPass > 0 ) {
		for (x=0; x < hm->w; x++) {
			int offset=x,y;
			for (y=0; y < hm->h; y++) {
				static int dx[8]={-1,0,1,-1,1,-1,0,1};
				static int dy[8]={-1,-1,-1,0,0,1,1,1};
				int nextx=0,nexty=0,i;
				float v=hm->values[offset];
				/* calculate slope at x,y */
				float slope=0.0f;
				for (i=0; i < 8; i++ ) { /* 4 : von neumann neighbourhood 8 : moore neighbourhood */
					int nx=x+dx[i];
					int ny=y+dy[i];
					if ( nx >= 0 && nx < hm->w && ny >= 0 && ny < hm->h ) {
						float nslope=v-GET_VALUE(hm,nx,ny);
						if ( nslope > slope ) {
							slope=nslope;
							nextx=nx;
							nexty=ny;
						}
					}
				}
				if ( slope > minSlope ) {
					GET_VALUE(hm,x,y) -= erosionCoef*(slope-minSlope);
					GET_VALUE(hm,nextx,nexty) += agregationCoef*(slope-minSlope);
				}
				offset+=hm->w;
			}
		}
		nbPass--;
	}
}
#endif

void TCOD_heightmap_kernel_transform(TCOD_heightmap_t *hm, int kernelsize, const int *dx, const int *dy, const float *weight, float minLevel,float maxLevel) {
	int x,y;
	for (x=0; x < hm->w; x++) {
		int offset=x;
		for (y=0; y < hm->h; y++) {
			if ( hm->values[offset] >= minLevel && hm->values[offset] <= maxLevel ) {
				float val=0.0f;
				float totalWeight=0.0f;
				int i;
				for (i=0; i < kernelsize; i++ ) {
					int nx=x+dx[i];
					int ny=y+dy[i];
					if ( nx >= 0 && nx < hm->w && ny >= 0 && ny < hm->h ) {
						val+=weight[i]*GET_VALUE(hm,nx,ny);
						totalWeight+=weight[i];
					}
				}
				hm->values[offset]=val/totalWeight;
			}
			offset+=hm->w;
		}
	}

}

void TCOD_heightmap_add_voronoi(TCOD_heightmap_t *hm, int nbPoints, int nbCoef, const float *coef,TCOD_random_t rnd) {
	typedef struct {
		int x,y;
		float dist;
	} point_t;
	point_t *pt;
	int i,x,y;
	if ( nbPoints <= 0 ) return;
	pt=(point_t *)malloc(sizeof(point_t)*nbPoints);
	for (i=0; i < nbPoints; i++ ) {
		pt[i].x = TCOD_random_get_int(rnd,0,hm->w-1);
		pt[i].y = TCOD_random_get_int(rnd,0,hm->h-1);
	}
	for (x=0; x < hm->w; x++) {
		int offset=x;
		for (y=0; y < hm->h; y++) {
			/* calculate distance to voronoi points */
			for (i=0; i < nbPoints; i++ ) {
				pt[i].dist = (float)(pt[i].x-x)*(pt[i].x-x)+(pt[i].y-y)*(pt[i].y-y);
			}
			for (i=0; i < nbCoef; i++ ) {
				/* get closest point */
				float minDist=1E8f;
				int idx=-1,j;
				for (j=0; j < nbPoints; j++ ) {
					if ( pt[j].dist < minDist ) {
						idx=j;
						minDist=pt[j].dist;
					}
				}
				hm->values[offset]+=coef[i]*pt[idx].dist;
				pt[idx].dist=1E8f;
			}
			offset+= hm->w;
		}
	}
	free(pt);
}

static void setMPDHeight(TCOD_heightmap_t *hm, TCOD_random_t rnd,int x,int y, float z, float offset);
static void setMDPHeightSquare(TCOD_heightmap_t *hm, TCOD_random_t rnd,int x, int y, int initsz, int sz,float offset);

void TCOD_heightmap_mid_point_displacement(TCOD_heightmap_t *hm, TCOD_random_t rnd, float roughness) {
	int step = 1;
	float offset = 1.0f;
	int initsz = MIN(hm->w,hm->h)-1;
	int sz = initsz;
	hm->values[0] = TCOD_random_get_float(rnd,0.0f,1.0f);
	hm->values[sz-1] = TCOD_random_get_float(rnd,0.0f,1.0f);
	hm->values[(sz-1)*sz] = TCOD_random_get_float(rnd,0.0f,1.0f);
	hm->values[sz*sz-1] = TCOD_random_get_float(rnd,0.0f,1.0f);
	while (sz > 0) {
		int x,y;
		/* diamond step */
		for (x=0; x < step; x++ ) {
			for (y=0; y < step; y++ ) {
				int diamondx = sz / 2 + x * sz;
				int diamondy = sz / 2 + y * sz;
				float z= GET_VALUE(hm,x*sz,y*sz);
				z += GET_VALUE(hm,(x+1)*sz,y*sz);
				z += GET_VALUE(hm,(x+1)*sz,(y+1)*sz);
				z += GET_VALUE(hm,x*sz,(y+1)*sz);
				z *= 0.25f;
				setMPDHeight(hm,rnd,diamondx,diamondy,z,offset);
			}
		}
		offset*=roughness;
		/* square step */
		for (x=0; x < step; x++ ) {
			for (y=0; y < step; y++ ) {
				int diamondx = sz / 2 + x * sz;
				int diamondy = sz / 2 + y * sz;
				/* north */
				setMDPHeightSquare(hm, rnd, diamondx, diamondy-sz/2, initsz,sz/2, offset);
				/* south */
				setMDPHeightSquare(hm, rnd, diamondx, diamondy+sz/2, initsz,sz/2, offset);
				/* west */
				setMDPHeightSquare(hm, rnd, diamondx-sz/2, diamondy, initsz,sz/2, offset);
				/* east */
				setMDPHeightSquare(hm, rnd, diamondx+sz/2, diamondy, initsz,sz/2, offset);
			}
		}
		sz /= 2;
		step *= 2;
	}
}

/* private stuff */
static void setMPDHeight(TCOD_heightmap_t *hm, TCOD_random_t rnd,int x,int y, float z, float offset) {
	z += TCOD_random_get_float(rnd,-offset,offset);
	GET_VALUE(hm,x,y)=z;
}

static void setMDPHeightSquare(TCOD_heightmap_t *hm, TCOD_random_t rnd,int x, int y, int initsz, int sz,float offset) {
	float z=0;
	int count=0;
	if ( y >= sz ) {
		z += GET_VALUE(hm,x,y-sz);
		count++;
	}
	if ( x >= sz ) {
		z += GET_VALUE(hm,x-sz,y);
		count++;
	}
	if ( y+sz < initsz ) {
		z += GET_VALUE(hm,x,y+sz);
		count++;
	}
	if ( x+sz < initsz ) {
		z += GET_VALUE(hm,x+sz,y);
		count++;
	}
	z /= count;
	setMPDHeight(hm,rnd,x,y,z,offset);
}
