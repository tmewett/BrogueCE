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

#define WAVELET_TILE_SIZE 32
#define WAVELET_ARAD 16

#define SIMPLEX_SCALE 0.5f
#define WAVELET_SCALE 2.0f

typedef struct {
	int ndim;
	unsigned char map[256]; /* Randomized map of indexes into buffer */
	float buffer[256][TCOD_NOISE_MAX_DIMENSIONS]; 	/* Random 256 x ndim buffer */
	/* fractal stuff */
	float H;
	float lacunarity;
	float exponent[TCOD_NOISE_MAX_OCTAVES];
	float *waveletTileData;
	TCOD_random_t rand;
	/* noise type */
	TCOD_noise_type_t noise_type;
} perlin_data_t;

static float lattice( perlin_data_t *data, int ix, float fx, int iy, float fy, int iz, float fz, int iw, float fw)
{
	int n[4] = {ix, iy, iz, iw};
	float f[4] = {fx, fy, fz, fw};
	int nIndex = 0;
	int i;
	float value = 0;
	for(i=0; i<data->ndim; i++)
		nIndex = data->map[(nIndex + n[i]) & 0xFF];
	for(i=0; i<data->ndim; i++)
		value += data->buffer[nIndex][i] * f[i];
	return value;
}

#define DEFAULT_SEED 0x15687436
#define DELTA				1e-6f
#define SWAP(a, b, t)		t = a; a = b; b = t

#define FLOOR(a) ((a)> 0 ? ((int)a) : (((int)a)-1) )
#define CUBIC(a)	( a * a * (3 - 2*a) )

static void normalize(perlin_data_t *data, float *f)
{
	float magnitude = 0;
	int i;
	for(i=0; i<data->ndim; i++)
		magnitude += f[i]*f[i];
	magnitude = 1.0f / (float)sqrt(magnitude);
	for(i=0; i<data->ndim; i++)
		f[i] *= magnitude;
}


TCOD_noise_t TCOD_noise_new(int ndim, float hurst, float lacunarity, TCOD_random_t random)
{
	perlin_data_t *data=(perlin_data_t *)calloc(sizeof(perlin_data_t),1);
	int i, j;
	unsigned char tmp;
	float f = 1;
	data->rand = random ? random : TCOD_random_get_instance();
	data->ndim = ndim;
	for(i=0; i<256; i++)
	{
		data->map[i] = (unsigned char)i;
		for(j=0; j<data->ndim; j++)
			data->buffer[i][j] = TCOD_random_get_float(data->rand,-0.5, 0.5);
		normalize(data,data->buffer[i]);
	}

	while(--i)
	{
		j = TCOD_random_get_int(data->rand,0, 255);
		SWAP(data->map[i], data->map[j], tmp);
	}

	data->H = hurst;
	data->lacunarity = lacunarity;
	for(i=0; i<TCOD_NOISE_MAX_OCTAVES; i++)
	{
		/*exponent[i] = powf(f, -H); */
		data->exponent[i] = 1.0f / f;
		f *= lacunarity;
	}
	data->noise_type = TCOD_NOISE_DEFAULT;
	return (TCOD_noise_t)data;
}

float TCOD_noise_perlin( TCOD_noise_t noise, float *f )
{
	perlin_data_t *data=(perlin_data_t *)noise;
	int n[TCOD_NOISE_MAX_DIMENSIONS];			/* Indexes to pass to lattice function */
	int i;
	float r[TCOD_NOISE_MAX_DIMENSIONS];		/* Remainders to pass to lattice function */
	float w[TCOD_NOISE_MAX_DIMENSIONS];		/* Cubic values to pass to interpolation function */
	float value;

	for(i=0; i<data->ndim; i++)
	{
		n[i] = FLOOR(f[i]);
		r[i] = f[i] - n[i];
		w[i] = CUBIC(r[i]);
	}

	switch(data->ndim)
	{
		case 1:
			value = LERP(lattice(data,n[0], r[0],0,0,0,0,0,0),
						  lattice(data,n[0]+1, r[0]-1,0,0,0,0,0,0),
						  w[0]);
			break;
		case 2:
			value = LERP(LERP(lattice(data,n[0], r[0], n[1], r[1],0,0,0,0),
							   lattice(data,n[0]+1, r[0]-1, n[1], r[1],0,0,0,0),
							   w[0]),
						  LERP(lattice(data,n[0], r[0], n[1]+1, r[1]-1,0,0,0,0),
							   lattice(data,n[0]+1, r[0]-1, n[1]+1, r[1]-1,0,0,0,0),
							   w[0]),
						  w[1]);
			break;
		case 3:
			value = LERP(LERP(LERP(lattice(data,n[0], r[0], n[1], r[1], n[2], r[2],0,0),
									lattice(data,n[0]+1, r[0]-1, n[1], r[1], n[2], r[2],0,0),
									w[0]),
							   LERP(lattice(data,n[0], r[0], n[1]+1, r[1]-1, n[2], r[2],0,0),
									lattice(data,n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2], r[2],0,0),
									w[0]),
							   w[1]),
						  LERP(LERP(lattice(data,n[0], r[0], n[1], r[1], n[2]+1, r[2]-1,0,0),
									lattice(data,n[0]+1, r[0]-1, n[1], r[1], n[2]+1, r[2]-1,0,0),
									w[0]),
							   LERP(lattice(data,n[0], r[0], n[1]+1, r[1]-1, n[2]+1, r[2]-1,0,0),
									lattice(data,n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2]+1, r[2]-1,0,0),
									w[0]),
							   w[1]),
						  w[2]);
			break;
		case 4:
		default:
			value = LERP(LERP(LERP(LERP(lattice(data,n[0], r[0], n[1], r[1], n[2], r[2], n[3], r[3]),
										 lattice(data,n[0]+1, r[0]-1, n[1], r[1], n[2], r[2], n[3], r[3]),
										 w[0]),
									LERP(lattice(data,n[0], r[0], n[1]+1, r[1]-1, n[2], r[2], n[3], r[3]),
										 lattice(data,n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2], r[2], n[3], r[3]),
										 w[0]),
									w[1]),
									LERP(LERP(lattice(data,n[0], r[0], n[1], r[1], n[2]+1, r[2]-1, n[3], r[3]),
										 lattice(data,n[0]+1, r[0]-1, n[1], r[1], n[2]+1, r[2]-1, n[3], r[3]),
										 w[0]),
									LERP(lattice(data,n[0], r[0], n[1]+1, r[1]-1, n[2]+1, r[2]-1,0,0),
										 lattice(data,n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2]+1, r[2]-1, n[3], r[3]),
										 w[0]),
									w[1]),
							   w[2]),
						  LERP(LERP(LERP(lattice(data,n[0], r[0], n[1], r[1], n[2], r[2], n[3]+1, r[3]-1),
										 lattice(data,n[0]+1, r[0]-1, n[1], r[1], n[2], r[2], n[3]+1, r[3]-1),
										 w[0]),
									LERP(lattice(data,n[0], r[0], n[1]+1, r[1]-1, n[2], r[2], n[3]+1, r[3]-1),
										 lattice(data,n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2], r[2], n[3]+1, r[3]-1),
										 w[0]),
									w[1]),
									LERP(LERP(lattice(data,n[0], r[0], n[1], r[1], n[2]+1, r[2]-1, n[3]+1, r[3]-1),
										 lattice(data,n[0]+1, r[0]-1, n[1], r[1], n[2]+1, r[2]-1, n[3]+1, r[3]-1),
										 w[0]),
									LERP(lattice(data,n[0], r[0], n[1]+1, r[1]-1, n[2]+1, r[2]-1,0,0),
										 lattice(data,n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2]+1, r[2]-1, n[3]+1, r[3]-1),
										 w[0]),
									w[1]),
							   w[2]),
						  w[3]);
			break;
	}
	return CLAMP(-0.99999f, 0.99999f, value);
}

/* simplex noise, adapted from Ken Perlin's presentation at Siggraph 2001 */
/* and Stefan Gustavson implementation */

#define TCOD_NOISE_SIMPLEX_GRADIENT_1D(n,h,x) { float grad; h &= 0xF; grad=1.0f+(h & 7); if ( h & 8 ) grad = -grad; n = grad * x; }

#define TCOD_NOISE_SIMPLEX_GRADIENT_2D(n,h,x,y) { float u,v; h &= 0x7; if ( h < 4 ) { u=x; v=2.0f*y; } else { u=y; v=2.0f*x; } n = ((h & 1) ? -u : u) + ((h & 2) ? -v :v ); }

#define TCOD_NOISE_SIMPLEX_GRADIENT_3D(n,h,x,y,z) { float u,v; h &= 0xF; u = (h < 8 ? x : y); v = (h < 4 ? y : ( h == 12 || h == 14 ? x : z ) ); n= ((h & 1) ? -u : u ) + ((h & 2) ? -v : v); }

#define TCOD_NOISE_SIMPLEX_GRADIENT_4D(n,h,x,y,z,t) { float u,v,w; h &= 0x1F; u = (h < 24 ? x:y); v = (h < 16 ? y:z); w = (h < 8 ? z:t); n= ((h & 1) ? -u : u ) + ((h & 2) ? -v : v) + ((h & 4) ? -w : w);}

static float simplex[64][4] = {
	{0,1,2,3},{0,1,3,2},{0,0,0,0},{0,2,3,1},{0,0,0,0},{0,0,0,0},{0,0,0,0},{1,2,3,0},
	{0,2,1,3},{0,0,0,0},{0,3,1,2},{0,3,2,1},{0,0,0,0},{0,0,0,0},{0,0,0,0},{1,3,2,0},
	{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
	{1,2,0,3},{0,0,0,0},{1,3,0,2},{0,0,0,0},{0,0,0,0},{0,0,0,0},{2,3,0,1},{2,3,1,0},
	{1,0,2,3},{1,0,3,2},{0,0,0,0},{0,0,0,0},{0,0,0,0},{2,0,3,1},{0,0,0,0},{2,1,3,0},
	{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
	{2,0,1,3},{0,0,0,0},{0,0,0,0},{0,0,0,0},{3,0,1,2},{3,0,2,1},{0,0,0,0},{3,1,2,0},
	{2,1,0,3},{0,0,0,0},{0,0,0,0},{0,0,0,0},{3,1,0,2},{0,0,0,0},{3,2,0,1},{3,2,1,0},

};

float TCOD_noise_simplex(TCOD_noise_t noise, float *f) {
	perlin_data_t *data=(perlin_data_t *)noise;
	switch(data->ndim) {
		case 1 :
		{
			int i0=(int)FLOOR(f[0]*SIMPLEX_SCALE);
			int i1=i0+1;
			float x0 = f[0]*SIMPLEX_SCALE - i0;
			float x1 = x0 - 1.0f;
			float t0 = 1.0f - x0*x0;
			float t1 = 1.0f - x1*x1;
			float n0,n1;
			t0 = t0*t0;
			t1 = t1*t1;
			i0=data->map[i0&0xFF];
			TCOD_NOISE_SIMPLEX_GRADIENT_1D(n0,i0,x0);
			n0*=t0*t0;
			i1=data->map[i1&0xFF];
			TCOD_NOISE_SIMPLEX_GRADIENT_1D(n1,i1,x1);
			n1*=t1*t1;
			return 0.25f * (n0+n1);
		}
		break;
		case 2 :
		{
			#define F2 0.366025403f  /* 0.5f * (sqrtf(3.0f)-1.0f); */
			#define G2 0.211324865f  /* (3.0f - sqrtf(3.0f))/6.0f; */

			float s = (f[0]+f[1])*F2*SIMPLEX_SCALE;
			float xs = f[0]*SIMPLEX_SCALE+s;
			float ys = f[1]*SIMPLEX_SCALE+s;
			int i=FLOOR(xs);
			int j=FLOOR(ys);
			float t = (i+j)*G2;
			float xo = i-t;
			float yo = j-t;
			float x0 = f[0]*SIMPLEX_SCALE-xo;
			float y0 = f[1]*SIMPLEX_SCALE-yo;
			int i1,j1,ii = i%256,jj = j%256;
			float n0,n1,n2,x1,y1,x2,y2,t0,t1,t2;
			if ( x0 > y0 ) {
				i1=1;j1=0;
			} else {
				i1=0;j1=1;
			}
			x1 = x0 - i1 + G2;
			y1 = y0 - j1 + G2;
			x2 = x0 - 1.0f + 2.0f * G2;
			y2 = y0 - 1.0f + 2.0f * G2;
			t0 = 0.5f - x0*x0 - y0*y0;
			if ( t0 < 0.0f ) {
				n0 = 0.0f;
			} else {
				int idx = (ii + data->map[jj])&0xFF;
				t0 *= t0;
				idx=data->map[idx];
				TCOD_NOISE_SIMPLEX_GRADIENT_2D(n0,idx,x0,y0);
				n0 *= t0*t0;
			}
			t1 = 0.5f - x1*x1 -y1*y1;
			if ( t1 < 0.0f ) {
				n1 = 0.0f;
			} else {
				int idx = (ii + i1 + data->map[(jj+j1)&0xFF]) & 0xFF;
				t1 *= t1;
				idx=data->map[idx];
				TCOD_NOISE_SIMPLEX_GRADIENT_2D(n1,idx,x1,y1);
				n1 *= t1*t1;
			}
			t2 = 0.5f - x2*x2 -y2*y2;
			if ( t2 < 0.0f ) {
				n2 = 0.0f;
			} else {
				int idx = (ii + 1 + data->map[(jj+1)&0xFF]) & 0xFF;
				t2 *= t2;
				idx=data->map[idx];
				TCOD_NOISE_SIMPLEX_GRADIENT_2D(n2,idx,x2,y2);
				n2 *= t2*t2;
			}
			return 40.0f * (n0+n1+n2);
		}
		break;
		case 3 :
		{
			#define F3 0.333333333f
			#define G3 0.166666667f
			float n0,n1,n2,n3;
			float s =(f[0]+f[1]+f[2])*F3*SIMPLEX_SCALE;
			float xs=f[0]*SIMPLEX_SCALE+s;
			float ys=f[1]*SIMPLEX_SCALE+s;
			float zs=f[2]*SIMPLEX_SCALE+s;
			int i=FLOOR(xs);
			int j=FLOOR(ys);
			int k=FLOOR(zs);
			float t=(float)(i+j+k)*G3;
			float xo = i-t;
			float yo = j-t;
			float zo = k-t;
			float x0 = f[0]*SIMPLEX_SCALE-xo;
			float y0 = f[1]*SIMPLEX_SCALE-yo;
			float z0 = f[2]*SIMPLEX_SCALE-zo;
			int i1,j1,k1,i2,j2,k2,ii,jj,kk;
			float x1,y1,z1,x2,y2,z2,x3,y3,z3,t0,t1,t2,t3;
			if ( x0 >= y0 ) {
				if ( y0 >= z0 ) {
					i1=1;j1=0;k1=0;i2=1;j2=1;k2=0;
				} else if ( x0 >= z0 ) {
					i1=1;j1=0;k1=0;i2=1;j2=0;k2=1;
				} else {
					i1=0;j1=0;k1=1;i2=1;j2=0;k2=1;
				}
			} else {
				if ( y0 < z0 ) {
					i1=0;j1=0;k1=1;i2=0;j2=1;k2=1;
				} else if ( x0 < z0 ) {
					i1=0;j1=1;k1=0;i2=0;j2=1;k2=1;
				} else {
					i1=0;j1=1;k1=0;i2=1;j2=1;k2=0;
				}
			}
			x1 = x0 -i1 + G3;
			y1 = y0 -j1 + G3;
			z1 = z0 -k1 + G3;
			x2 = x0 -i2 + 2.0f*G3;
			y2 = y0 -j2 + 2.0f*G3;
			z2 = z0 -k2 + 2.0f*G3;
			x3 = x0 - 1.0f +3.0f * G3;
			y3 = y0 - 1.0f +3.0f * G3;
			z3 = z0 - 1.0f +3.0f * G3;
			ii = i%256;
			jj = j%256;
			kk = k%256;
			t0 = 0.6f - x0*x0 -y0*y0 -z0*z0;
			if ( t0 < 0.0f ) n0 = 0.0f;
			else {
				int idx = data->map[ (ii + data->map[ (jj + data->map[ kk ]) &0xFF ])& 0xFF ];
				t0 *= t0;
				TCOD_NOISE_SIMPLEX_GRADIENT_3D(n0,idx,x0,y0,z0);
				n0 *= t0*t0;
			}
			t1 = 0.6f - x1*x1 -y1*y1 -z1*z1;
			if ( t1 < 0.0f ) n1 = 0.0f;
			else {
				int idx = data->map[ (ii + i1 +  data->map[ (jj + j1 + data->map[ (kk + k1)& 0xFF ]) &0xFF ])& 0xFF ];
				t1 *= t1;
				TCOD_NOISE_SIMPLEX_GRADIENT_3D(n1,idx,x1,y1,z1);
				n1 *= t1*t1;
			}
			t2 = 0.6f - x2*x2 -y2*y2 -z2*z2;
			if ( t2 < 0.0f ) n2 = 0.0f;
			else {
				int idx = data->map[ (ii + i2 +  data->map[ (jj + j2 + data->map[ (kk + k2)& 0xFF ]) &0xFF ])& 0xFF ];
				t2 *= t2;
				TCOD_NOISE_SIMPLEX_GRADIENT_3D(n2,idx,x2,y2,z2);
				n2 *= t2*t2;
			}
			t3 = 0.6f - x3*x3 -y3*y3 -z3*z3;
			if ( t3 < 0.0f ) n3 = 0.0f;
			else {
				int idx = data->map[ (ii + 1 +  data->map[ (jj + 1 + data->map[ (kk + 1)& 0xFF ]) &0xFF ])& 0xFF ];
				t3 *= t3;
				TCOD_NOISE_SIMPLEX_GRADIENT_3D(n3,idx,x3,y3,z3);
				n3 *= t3*t3;
			}
			return 32.0f * (n0+n1+n2+n3);

		}
		break;
		case 4 :
		{
			#define F4 0.309016994f /* (sqrtf(5.0f)-1.0f)/4.0f */
			#define G4 0.138196601f /* (5.0f - sqrtf(5.0f))/20.0f */
			float n0,n1,n2,n3,n4;
			float s = (f[0]+f[1]+f[2]+f[3])*F4 * SIMPLEX_SCALE;
			float xs=f[0]*SIMPLEX_SCALE+s;
			float ys=f[1]*SIMPLEX_SCALE+s;
			float zs=f[2]*SIMPLEX_SCALE+s;
			float ws=f[3]*SIMPLEX_SCALE+s;
			int i=FLOOR(xs);
			int j=FLOOR(ys);
			int k=FLOOR(zs);
			int l=FLOOR(ws);
			float t=(float)(i+j+k+l)*G4;
			float xo = i-t;
			float yo = j-t;
			float zo = k-t;
			float wo = l-t;
			float x0 = f[0]*SIMPLEX_SCALE-xo;
			float y0 = f[1]*SIMPLEX_SCALE-yo;
			float z0 = f[2]*SIMPLEX_SCALE-zo;
			float w0 = f[3]*SIMPLEX_SCALE-wo;
			int c1 = (x0 > y0 ? 32 : 0);
			int c2 = (x0 > z0 ? 16 : 0);
			int c3 = (y0 > z0 ? 8 : 0);
			int c4 = (x0 > w0 ? 4 : 0);
			int c5 = (y0 > w0 ? 2 : 0);
			int c6 = (z0 > w0 ? 1 : 0);
			int c = c1+c2+c3+c4+c5+c6;


			int i1,j1,k1,l1,i2,j2,k2,l2,i3,j3,k3,l3,ii,jj,kk,ll;
			float x1,y1,z1,w1,x2,y2,z2,w2,x3,y3,z3,w3,x4,y4,z4,w4,t0,t1,t2,t3,t4;
			i1 = simplex[c][0] >= 3 ? 1:0;
			j1 = simplex[c][1] >= 3 ? 1:0;
			k1 = simplex[c][2] >= 3 ? 1:0;
			l1 = simplex[c][3] >= 3 ? 1:0;

			i2 = simplex[c][0] >= 2 ? 1:0;
			j2 = simplex[c][1] >= 2 ? 1:0;
			k2 = simplex[c][2] >= 2 ? 1:0;
			l2 = simplex[c][3] >= 2 ? 1:0;

			i3 = simplex[c][0] >= 1 ? 1:0;
			j3 = simplex[c][1] >= 1 ? 1:0;
			k3 = simplex[c][2] >= 1 ? 1:0;
			l3 = simplex[c][3] >= 1 ? 1:0;

			x1 = x0 -i1 + G4;
			y1 = y0 -j1 + G4;
			z1 = z0 -k1 + G4;
			w1 = w0 -l1 + G4;
			x2 = x0 -i2 + 2.0f*G4;
			y2 = y0 -j2 + 2.0f*G4;
			z2 = z0 -k2 + 2.0f*G4;
			w2 = w0 -l2 + 2.0f*G4;
			x3 = x0 -i3 + 3.0f*G4;
			y3 = y0 -j3 + 3.0f*G4;
			z3 = z0 -k3 + 3.0f*G4;
			w3 = w0 -l3 + 3.0f*G4;
			x4 = x0 - 1.0f +4.0f * G4;
			y4 = y0 - 1.0f +4.0f * G4;
			z4 = z0 - 1.0f +4.0f * G4;
			w4 = w0 - 1.0f +4.0f * G4;

			ii = i%256;
			jj = j%256;
			kk = k%256;
			ll = l%256;

			t0 = 0.6f - x0*x0 -y0*y0 -z0*z0 -w0*w0;
			if ( t0 < 0.0f ) n0 = 0.0f;
			else {
				int idx = data->map[ (ii + data->map[ (jj + data->map[ (kk + data->map[ ll ] ) &0xFF]) &0xFF ])& 0xFF ];
				t0 *= t0;
				TCOD_NOISE_SIMPLEX_GRADIENT_4D(n0,idx,x0,y0,z0,w0);
				n0 *= t0*t0;
			}
			t1 = 0.6f - x1*x1 -y1*y1 -z1*z1 -w1*w1;
			if ( t1 < 0.0f ) n1 = 0.0f;
			else {
				int idx = data->map[ (ii + i1 +  data->map[ (jj + j1 + data->map[ (kk + k1 + data->map[ (ll+l1)&0xFF])& 0xFF ]) &0xFF ])& 0xFF ];
				t1 *= t1;
				TCOD_NOISE_SIMPLEX_GRADIENT_4D(n1,idx,x1,y1,z1,w1);
				n1 *= t1*t1;
			}
			t2 = 0.6f - x2*x2 -y2*y2 -z2*z2 -w2*w2;
			if ( t2 < 0.0f ) n2 = 0.0f;
			else {
				int idx = data->map[ (ii + i2 +  data->map[ (jj + j2 + data->map[ (kk + k2 + data->map[(ll+l2)&0xFF])& 0xFF ]) &0xFF ])& 0xFF ];
				t2 *= t2;
				TCOD_NOISE_SIMPLEX_GRADIENT_4D(n2,idx,x2,y2,z2,w2);
				n2 *= t2*t2;
			}
			t3 = 0.6f - x3*x3 -y3*y3 -z3*z3 -w3*w3;
			if ( t3 < 0.0f ) n3 = 0.0f;
			else {
				int idx = data->map[ (ii + i3 +  data->map[ (jj + j3 + data->map[ (kk + k3 + data->map[(ll+l3)&0xFF])& 0xFF ]) &0xFF ])& 0xFF ];
				t3 *= t3;
				TCOD_NOISE_SIMPLEX_GRADIENT_4D(n3,idx,x3,y3,z3,w3);
				n3 *= t3*t3;
			}
			t4 = 0.6f - x4*x4 -y4*y4 -z4*z4 -w4*w4;
			if ( t4 < 0.0f ) n4 = 0.0f;
			else {
				int idx = data->map[ (ii + 1 +  data->map[ (jj + 1 + data->map[ (kk + 1 + data->map[(ll+1)&0xFF])& 0xFF ]) &0xFF ])& 0xFF ];
				t4 *= t4;
				TCOD_NOISE_SIMPLEX_GRADIENT_4D(n4,idx,x4,y4,z4,w4);
				n4 *= t4*t4;
			}
			return 27.0f * (n0+n1+n2+n3+n4);

		}
		break;
	}
	return 0.0f;
}

typedef float (*TCOD_noise_func_t)( TCOD_noise_t noise, float *f );

static float TCOD_noise_fbm_int(TCOD_noise_t noise,  float *f, float octaves, TCOD_noise_func_t func ) {
	float tf[TCOD_NOISE_MAX_DIMENSIONS];
	perlin_data_t *data=(perlin_data_t *)noise;
	/* Initialize locals */
	double value = 0;
	int i,j;
	memcpy(tf,f,sizeof(float)*data->ndim);

	/* Inner loop of spectral construction, where the fractal is built */
	for(i=0; i<(int)octaves; i++)
	{
		value += (double)(func(noise,tf)) * data->exponent[i];
		for (j=0; j < data->ndim; j++) tf[j] *= data->lacunarity;
	}

	/* Take care of remainder in octaves */
	octaves -= (int)octaves;
	if(octaves > DELTA)
		value += (double)(octaves * func(noise,tf)) * data->exponent[i];
	return CLAMP(-0.99999f, 0.99999f, (float)value);
}

float TCOD_noise_fbm_perlin( TCOD_noise_t noise,  float *f, float octaves )
{
	return TCOD_noise_fbm_int(noise,f,octaves,TCOD_noise_perlin);
}

float TCOD_noise_fbm_simplex( TCOD_noise_t noise,  float *f, float octaves )
{
	return TCOD_noise_fbm_int(noise,f,octaves,TCOD_noise_simplex);
}

static float TCOD_noise_turbulence_int( TCOD_noise_t noise, float *f, float octaves, TCOD_noise_func_t func )
{
	float tf[TCOD_NOISE_MAX_DIMENSIONS];
	perlin_data_t *data=(perlin_data_t *)noise;
	/* Initialize locals */
	double value = 0;
	int i,j;
	memcpy(tf,f,sizeof(float)*data->ndim);

	/* Inner loop of spectral construction, where the fractal is built */
	for(i=0; i<(int)octaves; i++)
	{
		float nval=func(noise,tf);
		value += (double)(ABS(nval)) * data->exponent[i];
		for (j=0; j < data->ndim; j++) tf[j] *= data->lacunarity;
	}

	/* Take care of remainder in octaves */
	octaves -= (int)octaves;
	if(octaves > DELTA) {
		float nval=func(noise,tf);
		value += (double)(octaves * ABS(nval)) * data->exponent[i];
	}
	return CLAMP(-0.99999f, 0.99999f, (float)value);
}

float TCOD_noise_turbulence_perlin( TCOD_noise_t noise, float *f, float octaves ) {
	return TCOD_noise_turbulence_int(noise,f,octaves,TCOD_noise_perlin);
}

float TCOD_noise_turbulence_simplex( TCOD_noise_t noise, float *f, float octaves ) {
	return TCOD_noise_turbulence_int(noise,f,octaves,TCOD_noise_simplex);
}

/* wavelet noise, adapted from Robert L. Cook and Tony Derose 'Wavelet noise' paper */

static int absmod(int x, int n) {
	int m=x%n;
	return m < 0 ? m+n : m;
}

static void TCOD_noise_wavelet_downsample(float *from, float *to, int stride) {
	static float acoeffs[2*WAVELET_ARAD]= {
		0.000334f, -0.001528f, 0.000410f, 0.003545f, -0.000938f, -0.008233f, 0.002172f, 0.019120f,
		-0.005040f,-0.044412f, 0.011655f, 0.103311f, -0.025936f, -0.243780f, 0.033979f, 0.655340f,
		 0.655340f, 0.033979f,-0.243780f,-0.025936f,  0.103311f,  0.011655f,-0.044412f,-0.005040f,
		0.019120f,  0.002172f,-0.008233f,-0.000938f,  0.003546f,  0.000410f,-0.001528f, 0.000334f,
	};
	static float *a = &acoeffs[WAVELET_ARAD];
	int i;
	for (i=0; i < WAVELET_TILE_SIZE/2; i++) {
		int k;
		to[i*stride]=0;
		for (k=2*i-WAVELET_ARAD; k <2*i+WAVELET_ARAD; k++) {
			to[i*stride] += a[k-2*i]* from[ absmod(k,WAVELET_TILE_SIZE) * stride ];
		}
	}
}

static void TCOD_noise_wavelet_upsample(float *from, float *to, int stride) {
	static float pcoeffs[4]= { 0.25f, 0.75f, 0.75f, 0.25f };
	static float *p = &pcoeffs[2];
	int i;
	for (i=0; i < WAVELET_TILE_SIZE; i++) {
		int k;
		to[i*stride]=0;
		for (k=i/2; k <i/2+1; k++) {
			to[i*stride] += p[i-2*k]* from[ absmod(k,WAVELET_TILE_SIZE/2) * stride ];
		}
	}
}

static void TCOD_noise_wavelet_init(TCOD_noise_t pnoise) {
	perlin_data_t *data=(perlin_data_t *)pnoise;
	int ix,iy,iz,i,sz=WAVELET_TILE_SIZE*WAVELET_TILE_SIZE*WAVELET_TILE_SIZE*sizeof(float);
	float *temp1=(float *)malloc(sz);
	float *temp2=(float *)malloc(sz);
	float *noise=(float *)malloc(sz);
	int offset;
	for (i=0; i < WAVELET_TILE_SIZE*WAVELET_TILE_SIZE*WAVELET_TILE_SIZE; i++ ) {
		noise[i]=TCOD_random_get_float(data->rand,-1.0f,1.0f);
	}
	for (iy=0; iy < WAVELET_TILE_SIZE; iy++ ) {
		for (iz=0; iz < WAVELET_TILE_SIZE; iz++ ) {
			i = iy * WAVELET_TILE_SIZE + iz * WAVELET_TILE_SIZE * WAVELET_TILE_SIZE;
			TCOD_noise_wavelet_downsample(&noise[i], &temp1[i], 1);
			TCOD_noise_wavelet_upsample(&temp1[i], &temp2[i], 1);
		}
	}
	for (ix=0; ix < WAVELET_TILE_SIZE; ix++ ) {
		for (iz=0; iz < WAVELET_TILE_SIZE; iz++ ) {
			i = ix + iz * WAVELET_TILE_SIZE * WAVELET_TILE_SIZE;
			TCOD_noise_wavelet_downsample(&temp2[i], &temp1[i], WAVELET_TILE_SIZE);
			TCOD_noise_wavelet_upsample(&temp1[i], &temp2[i], WAVELET_TILE_SIZE);
		}
	}
	for (ix=0; ix < WAVELET_TILE_SIZE; ix++ ) {
		for (iy=0; iy < WAVELET_TILE_SIZE; iy++ ) {
			i = ix + iy * WAVELET_TILE_SIZE;
			TCOD_noise_wavelet_downsample(&temp2[i], &temp1[i], WAVELET_TILE_SIZE * WAVELET_TILE_SIZE);
			TCOD_noise_wavelet_upsample(&temp1[i], &temp2[i], WAVELET_TILE_SIZE * WAVELET_TILE_SIZE);
		}
	}
	for (i=0; i < WAVELET_TILE_SIZE*WAVELET_TILE_SIZE*WAVELET_TILE_SIZE; i++ ) {
		noise[i] -= temp2[i];
	}
	offset = WAVELET_TILE_SIZE/2;
	if ( (offset & 1) == 0 ) offset++;
	for (i=0,ix=0; ix < WAVELET_TILE_SIZE; ix++ ) {
		for (iy=0; iy < WAVELET_TILE_SIZE; iy++ ) {
			for (iz=0; iz < WAVELET_TILE_SIZE; iz++ ) {
				temp1[i++]=noise[ absmod(ix+offset,WAVELET_TILE_SIZE)
					+ absmod(iy+offset,WAVELET_TILE_SIZE)*WAVELET_TILE_SIZE
					+ absmod(iz+offset,WAVELET_TILE_SIZE)*WAVELET_TILE_SIZE*WAVELET_TILE_SIZE
					];
			}
		}
	}
	for (i=0; i < WAVELET_TILE_SIZE*WAVELET_TILE_SIZE*WAVELET_TILE_SIZE; i++ ) {
		noise[i] += temp1[i];
	}
	data->waveletTileData=noise;
	free(temp1);
	free(temp2);
}

float TCOD_noise_wavelet (TCOD_noise_t noise, float *f) {
	perlin_data_t *data=(perlin_data_t *)noise;
	float pf[3];
	int i;
	int p[3],c[3],mid[3],n=WAVELET_TILE_SIZE;
	float w[3][3],t,result=0.0f;
	if ( data->ndim > 3 ) return 0.0f; /* not supported */
	if (! data->waveletTileData ) TCOD_noise_wavelet_init(noise);
	for (i=0; i < data->ndim; i++ ) pf[i]=f[i]*WAVELET_SCALE;
	for (i=data->ndim; i < 3; i++ ) pf[i]=0.0f;
	for (i=0; i < 3; i++ ) {
		mid[i]=(int)ceil(pf[i]-0.5f);
		t=mid[i] - (pf[i]-0.5f);
		w[i][0]=t*t*0.5f;
		w[i][2]=(1.0f-t)*(1.0f-t)*0.5f;
		w[i][1]=1.0f - w[i][0]-w[i][2];
	}
	for (p[2]=-1; p[2]<=1; p[2]++) {
		for (p[1]=-1; p[1]<=1; p[1]++) {
			for (p[0]=-1; p[0]<=1; p[0]++) {
				float weight=1.0f;
				for (i=0;i<3;i++) {
					c[i]=absmod(mid[i]+p[i],n);
					weight *= w[i][p[i]+1];
				}
				result += weight * data->waveletTileData[ c[2]*n*n + c[1]*n + c[0] ];
			}
		}
	}
	return CLAMP(-1.0f,1.0f,result);
}

float TCOD_noise_fbm_wavelet(TCOD_noise_t noise, float *f, float octaves) {
	return TCOD_noise_fbm_int(noise,f,octaves,TCOD_noise_wavelet);
}

float TCOD_noise_turbulence_wavelet(TCOD_noise_t noise, float *f, float octaves) {
	return TCOD_noise_turbulence_int(noise,f,octaves,TCOD_noise_wavelet);
}

void TCOD_noise_set_type (TCOD_noise_t noise, TCOD_noise_type_t type) {
	((perlin_data_t *)noise)->noise_type = type;
}

float TCOD_noise_get_ex (TCOD_noise_t noise, float *f, TCOD_noise_type_t type) {
	switch (type) {
		case (TCOD_NOISE_PERLIN): return TCOD_noise_perlin(noise,f); break;
		case (TCOD_NOISE_SIMPLEX): return TCOD_noise_simplex(noise,f); break;
		case (TCOD_NOISE_WAVELET): return TCOD_noise_wavelet(noise,f); break;
		default:
			switch (((perlin_data_t *)noise)->noise_type) {
				case (TCOD_NOISE_PERLIN): return TCOD_noise_perlin(noise,f); break;
				case (TCOD_NOISE_SIMPLEX): return TCOD_noise_simplex(noise,f); break;
				case (TCOD_NOISE_WAVELET): return TCOD_noise_wavelet(noise,f); break;
				default: return TCOD_noise_simplex(noise,f); break;
			}
			break;
	}
}

float TCOD_noise_get_fbm_ex (TCOD_noise_t noise, float *f, float octaves, TCOD_noise_type_t type) {
	switch (type) {
		case (TCOD_NOISE_PERLIN): return TCOD_noise_fbm_perlin(noise,f,octaves); break;
		case (TCOD_NOISE_SIMPLEX): return TCOD_noise_fbm_simplex(noise,f,octaves); break;
		case (TCOD_NOISE_WAVELET): return TCOD_noise_fbm_wavelet(noise,f,octaves); break;
		default:
			switch (((perlin_data_t *)noise)->noise_type) {
				case (TCOD_NOISE_PERLIN): return TCOD_noise_fbm_perlin(noise,f,octaves); break;
				case (TCOD_NOISE_SIMPLEX): return TCOD_noise_fbm_simplex(noise,f,octaves); break;
				case (TCOD_NOISE_WAVELET): return TCOD_noise_fbm_wavelet(noise,f,octaves); break;
				default: return TCOD_noise_fbm_simplex(noise,f,octaves); break;
			}
			break;
	}
}

float TCOD_noise_get_turbulence_ex (TCOD_noise_t noise, float *f, float octaves, TCOD_noise_type_t type) {
	switch (type) {
		case (TCOD_NOISE_PERLIN): return TCOD_noise_turbulence_perlin(noise,f,octaves); break;
		case (TCOD_NOISE_SIMPLEX): return TCOD_noise_turbulence_simplex(noise,f,octaves); break;
		case (TCOD_NOISE_WAVELET): return TCOD_noise_turbulence_wavelet(noise,f,octaves); break;
		default:
			switch (((perlin_data_t *)noise)->noise_type) {
				case (TCOD_NOISE_PERLIN): return TCOD_noise_turbulence_perlin(noise,f,octaves); break;
				case (TCOD_NOISE_SIMPLEX): return TCOD_noise_turbulence_simplex(noise,f,octaves); break;
				case (TCOD_NOISE_WAVELET): return TCOD_noise_turbulence_wavelet(noise,f,octaves); break;
				default: return TCOD_noise_turbulence_simplex(noise,f,octaves); break;
			}
			break;
	}
}

float TCOD_noise_get (TCOD_noise_t noise, float *f) {
	switch (((perlin_data_t *)noise)->noise_type) {
		case (TCOD_NOISE_PERLIN): return TCOD_noise_perlin(noise,f); break;
		case (TCOD_NOISE_SIMPLEX): return TCOD_noise_simplex(noise,f); break;
		case (TCOD_NOISE_WAVELET): return TCOD_noise_wavelet(noise,f); break;
		default: return TCOD_noise_simplex(noise,f); break;
	}
}

float TCOD_noise_get_fbm (TCOD_noise_t noise, float *f, float octaves) {
	switch (((perlin_data_t *)noise)->noise_type) {
		case (TCOD_NOISE_PERLIN): return TCOD_noise_fbm_perlin(noise,f,octaves); break;
		case (TCOD_NOISE_SIMPLEX): return TCOD_noise_fbm_simplex(noise,f,octaves); break;
		case (TCOD_NOISE_WAVELET): return TCOD_noise_fbm_wavelet(noise,f,octaves); break;
		default: return TCOD_noise_fbm_simplex(noise,f,octaves); break;
	}
}

float TCOD_noise_get_turbulence (TCOD_noise_t noise, float *f, float octaves) {
	switch (((perlin_data_t *)noise)->noise_type) {
		case (TCOD_NOISE_PERLIN): return TCOD_noise_turbulence_perlin(noise,f,octaves); break;
		case (TCOD_NOISE_SIMPLEX): return TCOD_noise_turbulence_simplex(noise,f,octaves); break;
		case (TCOD_NOISE_WAVELET): return TCOD_noise_turbulence_wavelet(noise,f,octaves); break;
		default: return TCOD_noise_turbulence_simplex(noise,f,octaves); break;
	}
}

void TCOD_noise_delete(TCOD_noise_t noise) {
	free((perlin_data_t *)noise);
}
