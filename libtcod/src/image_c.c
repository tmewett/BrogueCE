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
#include <stdarg.h>
#include <math.h>
#include "libtcod.h"
#include "libtcod_int.h"

typedef struct {
	int width,height;
	float fwidth,fheight;
	TCOD_color_t *buf;
	bool dirty;
} mipmap_t;

typedef struct {
	void *sys_img;
	int nb_mipmaps;
	mipmap_t *mipmaps;
	TCOD_color_t key_color;
	bool has_key_color;
} image_data_t;

static int TCOD_image_get_mipmap_levels(int width, int height) {
	int curw=width;
	int curh=height;
	int nb_mipmap=0;
	while ( curw > 0 && curh > 0 ) {
		nb_mipmap++;
		curw >>= 1;
		curh >>= 1;
	}
	return nb_mipmap;
}

static void TCOD_image_generate_mip(image_data_t *img, int mip) {
	mipmap_t *orig=&img->mipmaps[0];
	mipmap_t *cur =&img->mipmaps[mip];
	int x,y;
	if (! cur->buf) {
		cur->buf=(TCOD_color_t *)calloc(sizeof(TCOD_color_t),cur->width*cur->height);
	}
	cur->dirty=false;
	for (x=0; x < cur->width; x++) {
		for (y=0; y < cur->height; y++) {
			int r=0,g=0,b=0, count=0;
			int sx,sy;
			TCOD_color_t *col;
			for (sx=(x << mip); sx < ((x+1)<<mip); sx ++) {
				for (sy=(y << mip); sy < ((y+1)<<mip); sy ++) {
					int offset=sx+sy*orig->width;
					count++;
					r+=orig->buf[offset].r;
					g+=orig->buf[offset].g;
					b+=orig->buf[offset].b;
				}
			}
			r /= count;
			g /= count;
			b /= count;
			col = &cur->buf[x+y*cur->width];
			col->r=r;
			col->g=g;
			col->b=b;
		}
	}
}

static void TCOD_image_init_mipmaps(image_data_t *img) {
	int w,h,i,x,y;
	float fw,fh;
	if (! img->sys_img ) return;
	TCOD_sys_get_image_size(img->sys_img,&w,&h);
	img->nb_mipmaps=TCOD_image_get_mipmap_levels(w,h);
	img->mipmaps = (mipmap_t *)calloc(sizeof(mipmap_t),img->nb_mipmaps);
	img->mipmaps[0].buf = (TCOD_color_t *)calloc(sizeof(TCOD_color_t),w*h);
	for (x=0; x < w; x++) {
		for (y=0;y < h; y++) {
			img->mipmaps[0].buf[x+y*w]=TCOD_sys_get_image_pixel(img->sys_img,x,y);
		}
	}
	fw=(float)w;
	fh=(float)h;
	for ( i=0; i < img->nb_mipmaps; i++) {
		img->mipmaps[i].width=w;
		img->mipmaps[i].height=h;
		img->mipmaps[i].fwidth=fw;
		img->mipmaps[i].fheight=fh;
		img->mipmaps[i].dirty=true;
		w >>= 1;
		h >>= 1;
		fw *= 0.5f;
		fh *= 0.5f;
	}
	img->mipmaps[0].dirty=false;
}

void TCOD_image_clear(TCOD_image_t image, TCOD_color_t color) {
	int i;
	image_data_t *img=(image_data_t *)image;
	if ( !img->mipmaps && !img->sys_img) return; /* no image data */
	if ( ! img->mipmaps ) {
		TCOD_image_init_mipmaps(img);
	}
	for (i=0; i< img->mipmaps[0].width*img->mipmaps[0].height; i++) {
		img->mipmaps[0].buf[i] = color;
	}
	for ( i=1; i < img->nb_mipmaps; i++) {
		img->mipmaps[i].dirty=true;
	}
}

TCOD_image_t TCOD_image_new(int width, int height) {
	int i;
	float fw,fh;
	image_data_t *ret=(image_data_t *)calloc(sizeof(image_data_t),1);
	ret->nb_mipmaps=TCOD_image_get_mipmap_levels(width,height);
	ret->mipmaps = (mipmap_t *)calloc(sizeof(mipmap_t),ret->nb_mipmaps);
	ret->mipmaps[0].buf = (TCOD_color_t *)calloc(sizeof(TCOD_color_t),width*height);

	for (i=0; i< width*height; i++) {
		ret->mipmaps[0].buf[i] = TCOD_black;
	}
	fw=(float)width;
	fh=(float)height;
	for ( i=0; i < ret->nb_mipmaps; i++) {
		ret->mipmaps[i].width=width;
		ret->mipmaps[i].height=height;
		ret->mipmaps[i].fwidth=fw;
		ret->mipmaps[i].fheight=fh;
		width >>= 1;
		height >>= 1;
		fw *= 0.5f;
		fh *= 0.5f;
	}
	return (TCOD_image_t)ret;
}

TCOD_image_t TCOD_image_load(const char *filename) {
	image_data_t *ret=(image_data_t *)calloc(sizeof(image_data_t),1);
	ret->sys_img=TCOD_sys_load_image(filename);
	return (TCOD_image_t)ret;
}

void TCOD_image_get_size(TCOD_image_t image, int *w,int *h) {
	image_data_t *img=(image_data_t *)image;
	if ( !img->mipmaps && !img->sys_img) return; /* no image data */
	if ( img->mipmaps ) {
		*w = img->mipmaps[0].width;
		*h = img->mipmaps[0].height;
	} else {
		TCOD_sys_get_image_size(img->sys_img,w,h);
	}
}

TCOD_color_t TCOD_image_get_pixel(TCOD_image_t image,int x, int y) {
	image_data_t *img=(image_data_t *)image;
	if ( !img->mipmaps && !img->sys_img) return TCOD_black; /* no image data */
	if ( img->mipmaps ) {
		if ( x >= 0 && x < img->mipmaps[0].width
			&& y >= 0 && y < img->mipmaps[0].height ) {
			return img->mipmaps[0].buf[x+y*img->mipmaps[0].width];
		} else {
			return TCOD_black;
		}
	} else {
		return TCOD_sys_get_image_pixel(img->sys_img,x,y);
	}
}

int TCOD_image_get_alpha(TCOD_image_t image,int x, int y) {
	image_data_t *img=(image_data_t *)image;
	if ( img->sys_img ) {
		return TCOD_sys_get_image_alpha(img->sys_img,x,y);
	} else return 255;
}

TCOD_color_t TCOD_image_get_mipmap_pixel(TCOD_image_t image,float x0,float y0, float x1, float y1) {
	int texel_xsize,texel_ysize, texel_size, texel_x,texel_y;
	int cur_size=1;
	int mip=0;
	image_data_t *img=(image_data_t *)image;
	if ( !img->mipmaps && !img->sys_img) return TCOD_black; /* no image data */
	if (!img->mipmaps) TCOD_image_init_mipmaps(img);
	texel_xsize=(int)(x1-x0);
	texel_ysize=(int)(y1-y0);
	texel_size=texel_xsize < texel_ysize ? texel_ysize : texel_xsize;
	while ( mip < img->nb_mipmaps-1 && cur_size < texel_size ) {
		mip++;
		cur_size <<= 1;
	}
	if ( mip > 0 ) mip --;
	texel_x=(int)(x0*(img->mipmaps[mip].width)/img->mipmaps[0].fwidth);
	texel_y=(int)(y0*(img->mipmaps[mip].height)/img->mipmaps[0].fheight);

	if (img->mipmaps[mip].buf == NULL || img->mipmaps[mip].dirty) {
		TCOD_image_generate_mip(img,mip);
	}
	if ( texel_x < 0 || texel_y < 0 || texel_x >= img->mipmaps[mip].width || texel_y >= img->mipmaps[mip].height )
		return TCOD_black;
	return img->mipmaps[mip].buf[texel_x+texel_y*img->mipmaps[mip].width];
}

void TCOD_image_put_pixel(TCOD_image_t image,int x, int y,TCOD_color_t col) {
	image_data_t *img=(image_data_t *)image;
	if ( !img->mipmaps && !img->sys_img) return; /* no image data */
	if ( ! img->mipmaps ) {
		TCOD_image_init_mipmaps(img);
	}
	if ( x >= 0 && x < img->mipmaps[0].width
		&& y >= 0 && y < img->mipmaps[0].height ) {
		int mip;
		img->mipmaps[0].buf[x+y*img->mipmaps[0].width] = col;
		for (mip=1; mip < img->nb_mipmaps; mip++) {
			img->mipmaps[mip].dirty=true;
		}
	}
}

void TCOD_image_delete_internal(TCOD_image_t image) {
	image_data_t *img=(image_data_t *)image;
	if ( img->mipmaps ) {
		int i;
		for ( i=0; i < img->nb_mipmaps; i++) {
			if ( img->mipmaps[i].buf ) free(img->mipmaps[i].buf);
		}
		free(img->mipmaps);
	}
	if ( img->sys_img ) {
		TCOD_sys_delete_bitmap(img->sys_img);
	}
}

void TCOD_image_delete(TCOD_image_t image) {
	TCOD_image_delete_internal(image);
	free(image);
}

bool TCOD_image_is_pixel_transparent(TCOD_image_t image, int x, int y) {
	image_data_t *img=(image_data_t *)image;
	TCOD_color_t col=TCOD_image_get_pixel(image,x,y);
	if ( img->has_key_color && img->key_color.r == col.r
		&& img->key_color.g == col.g && img->key_color.b == col.b ) {
		return true;
	}
	if ( TCOD_image_get_alpha(image,x,y) == 0 ) return true;
	return false;
}

void TCOD_image_blit(TCOD_image_t image, TCOD_console_t console, float x, float y,
	TCOD_bkgnd_flag_t bkgnd_flag, float scalex, float scaley, float angle) {
	int width,height;
	image_data_t *img=(image_data_t *)image;
	if ( scalex == 0.0f || scaley == 0.0f || bkgnd_flag == TCOD_BKGND_NONE ) return;
	TCOD_image_get_size(image,&width,&height);
	float rx = x - width * 0.5f;
	float ry = y - height * 0.5f; 
	if ( scalex == 1.0f && scaley == 1.0f && angle == 0.0f && rx == ((int)rx) && ry == ((int)ry)) {
		/* clip the image */
		int ix = (int)(x - width*0.5f);
		int iy = (int)(y - height*0.5f);
		int minx=MAX(ix,0);
		int miny=MAX(iy,0);
		int maxx=MIN(ix+width,TCOD_console_get_width(console));
		int maxy=MIN(iy+height,TCOD_console_get_height(console));
		int offx=0,offy=0;
		int cx,cy;
		if ( ix < 0 ) offx=-ix;
		if ( iy < 0 ) offy=-iy;
		for (cx=minx; cx < maxx; cx ++) {
			for (cy=miny; cy < maxy; cy ++) {
				TCOD_color_t col=TCOD_image_get_pixel(image,cx-minx+offx,cy-miny+offy);
				if ( !img->has_key_color || img->key_color.r != col.r
					|| img->key_color.g != col.g || img->key_color.b != col.b ) {
					TCOD_console_set_char_background(console,cx,cy,col,bkgnd_flag);
				}
			}
		}
	} else {
		float iw=width/2*scalex;
		float ih=height/2*scaley;
		/* get the coordinates of the image corners in the console */
		float newx_x = (float)cos(angle);
		float newx_y = -(float)sin(angle);
		float newy_x = newx_y;
		float newy_y = -newx_x;
		float x0,y0,x1,y1,x2,y2,x3,y3; /* image corners coordinates */
		int rx,ry,rw,rh; /* rectangular area in the console */
		int cx,cy;
		int minx,miny,maxx,maxy;
		float invscalex,invscaley;
		/* 0 = P - w/2 x' +h/2 y' */
		x0 = x-iw*newx_x+ih*newy_x;
		y0 = y-iw*newx_y+ih*newy_y;
		/* 1 = P + w/2 x' + h/2 y' */
		x1 = x+iw*newx_x+ih*newy_x;
		y1 = y+iw*newx_y+ih*newy_y;
		/* 2 = P + w/2 x' - h/2 y' */
		x2 = x+iw*newx_x-ih*newy_x;
		y2 = y+iw*newx_y-ih*newy_y;
		/* 3 = P - w/2 x' - h/2 y' */
		x3 = x-iw*newx_x-ih*newy_x;
		y3 = y-iw*newx_y-ih*newy_y;
		/* get the affected rectangular area in the console */
		rx=(int)(MIN(MIN(x0,x1),MIN(x2,x3)));
		ry=(int)(MIN(MIN(y0,y1),MIN(y2,y3)));
		rw=(int)(MAX(MAX(x0,x1),MAX(x2,x3))) - rx;
		rh=(int)(MAX(MAX(y0,y1),MAX(y2,y3))) - ry;
		/* clip it */
		minx=MAX(rx,0);
		miny=MAX(ry,0);
		maxx=MIN(rx+rw,TCOD_console_get_width(console));
		maxy=MIN(ry+rh,TCOD_console_get_height(console));
		invscalex=1.0f / scalex;
		invscaley=1.0f / scaley;
		for (cx=minx; cx < maxx; cx ++) {
			for (cy=miny; cy < maxy; cy ++) {
				float ix,iy;
				TCOD_color_t col;
				/* map the console pixel to the image world */
				ix = (iw+ (cx-x) * newx_x + (cy-y) *(-newy_x))*invscalex;
				iy = (ih + (cx-x) * (newx_y) - (cy-y)*newy_y)*invscaley;
				col = TCOD_image_get_pixel(image,(int)(ix),(int)(iy));
				if ( !img->has_key_color || img->key_color.r != col.r
					|| img->key_color.g != col.g || img->key_color.b != col.b ) {
					if ( scalex < 1.0f || scaley < 1.0f ) {
						col = TCOD_image_get_mipmap_pixel(image,ix,iy,ix+1.0f,iy+1.0f);
					}
					TCOD_console_set_char_background(console,cx,cy,col,bkgnd_flag);
				}
			}
		}
	}
}

void TCOD_image_blit_rect(TCOD_image_t image, TCOD_console_t console, int x, int y,
	int w, int h, TCOD_bkgnd_flag_t bkgnd_flag) {
	int width,height;
	float scalex,scaley;
	TCOD_image_get_size(image,&width,&height);
	if ( w == -1 ) w=width;
	if ( h == -1 ) h=height;
	if ( w <= 0 || h <= 0 || bkgnd_flag == TCOD_BKGND_NONE ) return;
	scalex = (float)(w)/width;
	scaley = (float)(h)/height;
	TCOD_image_blit(image,console,x+w*0.5f,y+h*0.5f,bkgnd_flag,scalex,scaley,0.0f);
}

TCOD_image_t TCOD_image_from_console(TCOD_console_t console) {
	image_data_t *ret;
	void *bitmap=TCOD_sys_create_bitmap_for_console(console);
	ret=(image_data_t *)calloc(sizeof(image_data_t),1);
	ret->sys_img=bitmap;
	TCOD_image_refresh_console((TCOD_image_t)ret,console);
	return (TCOD_image_t)ret;
}

void TCOD_image_refresh_console(TCOD_image_t image, TCOD_console_t console) {
	image_data_t *img=(image_data_t *)image;
	TCOD_sys_console_to_bitmap(img->sys_img, TCOD_console_get_width(console), TCOD_console_get_height(console),
		TCOD_console_get_buf(console),NULL);
}

void TCOD_image_save(TCOD_image_t image, const char *filename) {
	image_data_t *img=(image_data_t *)image;
	void *bitmap=NULL;
	bool must_free=false;
	if ( img->sys_img ) {
		bitmap=img->sys_img;
	} else if ( img->mipmaps ){
		bitmap=TCOD_sys_create_bitmap(img->mipmaps[0].width, img->mipmaps[0].height, img->mipmaps[0].buf);
		must_free=true;
	}
	if (bitmap) {
		TCOD_sys_save_bitmap(bitmap, filename);
		if ( must_free ) {
			TCOD_sys_delete_bitmap(bitmap);
		}
	}
}

void TCOD_image_set_key_color(TCOD_image_t image, TCOD_color_t key_color) {
	image_data_t *img=(image_data_t *)image;
	img->has_key_color=true;
	img->key_color=key_color;
}
void TCOD_image_invert(TCOD_image_t image) {
	int i,mip;
	int width,height;
	image_data_t *img=(image_data_t *)image;
	if ( !img->mipmaps && !img->sys_img) return; /* no image data */
	if ( ! img->mipmaps ) {
		TCOD_image_init_mipmaps(img);
	}
	TCOD_image_get_size(image,&width,&height);
	for (i=0; i< width*height; i++) {
		TCOD_color_t col=img->mipmaps[0].buf[i];
		col.r=255-col.r;
		col.g=255-col.g;
		col.b=255-col.b;
		img->mipmaps[0].buf[i] = col;
	}
	for (mip=1; mip < img->nb_mipmaps; mip++) {
		img->mipmaps[mip].dirty=true;
	}
}

void TCOD_image_hflip(TCOD_image_t image) {
	int px,py;
	int width,height;
	TCOD_image_get_size(image,&width,&height);
	for (py = 0; py < height; py++ ) {
		for (px = 0; px < width/2; px++ ) {
			TCOD_color_t col1=TCOD_image_get_pixel(image,px,py);
			TCOD_color_t col2=TCOD_image_get_pixel(image,width-1-px,py);
			TCOD_image_put_pixel(image,px,py,col2);
			TCOD_image_put_pixel(image,width-1-px,py,col1);
		}
	}
}

void TCOD_image_vflip(TCOD_image_t image) {
	int px,py;
	int width,height;
	TCOD_image_get_size(image,&width,&height);
	for (px = 0; px < width; px++ ) {
		for (py = 0; py < height/2; py++ ) {
			TCOD_color_t col1=TCOD_image_get_pixel(image,px,py);
			TCOD_color_t col2=TCOD_image_get_pixel(image,px,height-1-py);
			TCOD_image_put_pixel(image,px,py,col2);
			TCOD_image_put_pixel(image,px,height-1-py,col1);
		}
	}
}

void TCOD_image_rotate90(TCOD_image_t image, int numRotations) {
	int px,py;
	int width,height;
	numRotations = numRotations % 4;
	if (numRotations == 0 ) return;
	if ( numRotations < 0 ) numRotations += 4;
	TCOD_image_get_size(image,&width,&height);
	if (numRotations == 1) {
		/* rotate 90 degrees */
		TCOD_image_t newImg=TCOD_image_new(height,width);
		image_data_t *img=(image_data_t *)image;
		image_data_t *img2=(image_data_t *)newImg;
		for (px = 0; px < width; px++ ) {
			for (py = 0; py < height; py++ ) {
				TCOD_color_t col1=TCOD_image_get_pixel(image,px,py);
				TCOD_image_put_pixel(newImg,height-1-py,px,col1);
			}
		}
		TCOD_image_delete_internal(image);
		/* update img with the new image content */
		img->mipmaps = img2->mipmaps;
		img->sys_img=NULL;
		img->nb_mipmaps=img2->nb_mipmaps;
		free(img2);
	} else if ( numRotations == 2 ) {
		/* rotate 180 degrees */
		int maxy=height/2 + ((height & 1) == 1? 1 : 0 );
		for (px = 0; px < width; px++ ) {
			for (py = 0; py < maxy; py++ ) {
				if ( py != height-1-py || px < width/2 ) {
					TCOD_color_t col1=TCOD_image_get_pixel(image,px,py);
					TCOD_color_t col2=TCOD_image_get_pixel(image,width-1-px,height-1-py);
					TCOD_image_put_pixel(image,px,py,col2);
					TCOD_image_put_pixel(image,width-1-px,height-1-py,col1);
				}
			}
		}
	} else if (numRotations == 3) {
		/* rotate 270 degrees */
		TCOD_image_t newImg=TCOD_image_new(height,width);
		image_data_t *img=(image_data_t *)image;
		image_data_t *img2=(image_data_t *)newImg;
		for (px = 0; px < width; px++ ) {
			for (py = 0; py < height; py++ ) {
				TCOD_color_t col1=TCOD_image_get_pixel(image,px,py);
				TCOD_image_put_pixel(newImg,py,width-1-px,col1);
			}
		}
		TCOD_image_delete_internal(image);
		/* update img with the new image content */
		img->mipmaps = img2->mipmaps;
		img->sys_img=NULL;
		img->nb_mipmaps=img2->nb_mipmaps;
		free(img2);
	}
}

void TCOD_image_scale(TCOD_image_t image, int neww, int newh) {
	image_data_t *img=(image_data_t *)image;
	int px,py;
	int width,height;
	image_data_t *newimg;
	TCOD_image_get_size(image,&width,&height);
	if ( neww==width && newh==height ) return;
	if ( neww == 0 || newh == 0 ) return;
	newimg=(image_data_t *)TCOD_image_new(neww,newh);

	if ( neww < width && newh < height ) {
		/* scale down image, using supersampling */
		for (py = 0; py < newh; py++ ) {
			float y0 = (float)(py) * height / newh;
			float y0floor = (float)floor(y0);
			float y0weight = 1.0f - (y0 - y0floor);
			int iy0 = (int)y0floor;

			float y1 = (float)(py+1) * height / newh;
			float y1floor = (float)floor(y1-0.00001);
			float y1weight = (y1 - y1floor);
			int iy1 = (int)y1floor;

			for (px = 0; px < neww; px++ ) {
			    TCOD_color_t col;
				float x0 = (float)(px) * width / neww;
				float x0floor = (float)floor(x0);
				float x0weight = 1.0f - (x0 - x0floor);
				int ix0 = (int)x0floor;

				float x1 = (float)(px+1) * width / neww;
				float x1floor = (float)floor(x1- 0.00001);
				float x1weight = (x1 - x1floor);
				int ix1 = (int)x1floor;

				float r=0,g=0,b=0,sumweight=0.0f;
				int srcx,srcy;
				/* left & right fractional edges */
				for (srcy=(int)(y0+1); srcy < (int)y1; srcy++) {
					TCOD_color_t col_left=TCOD_image_get_pixel(image,ix0,srcy);
					TCOD_color_t col_right=TCOD_image_get_pixel(image,ix1,srcy);
					r += col_left.r * x0weight + col_right.r * x1weight;
					g += col_left.g * x0weight + col_right.g * x1weight;
					b += col_left.b * x0weight + col_right.b * x1weight;
					sumweight += x0weight+x1weight;
				}
				/* top & bottom fractional edges */
				for (srcx = (int)(x0+1); srcx < (int)x1; srcx++) {
					TCOD_color_t col_top=TCOD_image_get_pixel(image,srcx,iy0);
					TCOD_color_t col_bottom=TCOD_image_get_pixel(image,srcx,iy1);
					r += col_top.r * y0weight + col_bottom.r * y1weight;
					g += col_top.g * y0weight + col_bottom.g * y1weight;
					b += col_top.b * y0weight + col_bottom.b * y1weight;
					sumweight += y0weight+y1weight;
				}
				/* center */
				for (srcy=(int)(y0+1); srcy < (int)y1; srcy++) {
					for (srcx = (int)(x0+1); srcx < (int)x1; srcx++) {
						TCOD_color_t col=TCOD_image_get_pixel(image,srcx,srcy);
						r += col.r;
						g += col.g;
						b += col.b;
						sumweight += 1.0f;
					}
				}
				/* corners */
				col=TCOD_image_get_pixel(image,ix0,iy0);
				r += col.r * (x0weight * y0weight);
				g += col.g * (x0weight * y0weight);
				b += col.b * (x0weight * y0weight);
				sumweight += x0weight * y0weight;
				col=TCOD_image_get_pixel(image,ix0,iy1);
				r += col.r * (x0weight * y1weight);
				g += col.g * (x0weight * y1weight);
				b += col.b * (x0weight * y1weight);
				sumweight += x0weight * y1weight;
				col=TCOD_image_get_pixel(image,ix1,iy1);
				r += col.r * (x1weight * y1weight);
				g += col.g * (x1weight * y1weight);
				b += col.b * (x1weight * y1weight);
				sumweight += x1weight * y1weight;
				col=TCOD_image_get_pixel(image,ix1,iy0);
				r += col.r * (x1weight * y0weight);
				g += col.g * (x1weight * y0weight);
				b += col.b * (x1weight * y0weight);
				sumweight += x1weight * y0weight;
				sumweight = 1.0f / sumweight;
				r = r*sumweight + 0.5f;
				g = g*sumweight + 0.5f;
				b = b*sumweight + 0.5f;
				col.r=(int)r;
				col.g=(int)g;
				col.b=(int)b;
				TCOD_image_put_pixel(newimg,px,py,col);
			}
		}
	} else {
		/* scale up image, using nearest neightbor */
		for (py = 0; py < newh; py++ ) {
			int srcy = py * height / newh;
			for (px = 0; px < neww; px++ ) {
				int srcx = px * width / neww;
				TCOD_color_t col=TCOD_image_get_pixel(image,srcx,srcy);
				TCOD_image_put_pixel(newimg,px,py,col);
			}
		}
	}

	/* destroy old image */
	TCOD_image_delete_internal(image);
	/* update img with the new image content */
	img->mipmaps = newimg->mipmaps;
	img->sys_img=NULL;
	img->nb_mipmaps=newimg->nb_mipmaps;
	free(newimg);
}


/* distance between two colors */
int rgbdist(const TCOD_color_t *c1,const TCOD_color_t *c2) {
	int dr=(int)(c1->r)-c2->r;
	int dg=(int)(c1->g)-c2->g;
	int db=(int)(c1->b)-c2->b;
	return dr*dr+dg*dg+db*db;
}

void getPattern(TCOD_color_t desired[4], TCOD_color_t palette[2], int *nbCols, int *ascii) {
	/* adapted from Jeff Lait's code posted on r.g.r.d */
	int flag=0;
	/*
		pixels have following flag values :
			X 1
			2 4
		flag indicates which pixels uses foreground color (palette[1])
	*/
	static int flagToAscii[8] = {
		0,
		TCOD_CHAR_SUBP_NE,TCOD_CHAR_SUBP_SW,-TCOD_CHAR_SUBP_DIAG,TCOD_CHAR_SUBP_SE,
		TCOD_CHAR_SUBP_E,-TCOD_CHAR_SUBP_N,-TCOD_CHAR_SUBP_NW
	};
	int weight[2] = { 0, 0 };
	int i;

	/* First colour trivial. */
	palette[0] = desired[0];

	/* Ignore all duplicates... */
	for (i = 1; i < 4; i++) {
		if (desired[i].r != palette[0].r || desired[i].g != palette[0].g || desired[i].b != palette[0].b)
		break;
	}

	/* All the same. */
	if (i == 4) {
		*nbCols=1;
		return;
	}
	weight[0] = i;

	/* Found a second colour... */
	palette[1] = desired[i];
	weight[1] = 1;
	flag |= 1<<(i-1);
	*nbCols = 2;
	/* remaining colours */
	i++;
	while (i< 4) {
		if (desired[i].r == palette[0].r && desired[i].g == palette[0].g && desired[i].b == palette[0].b) {
			weight[0]++;
		} else if (desired[i].r == palette[1].r && desired[i].g == palette[1].g && desired[i].b == palette[1].b)  {
			flag |= 1<<(i-1);
			weight[1]++;
		} else {
			/* Bah, too many colours, */
			/* merge the two nearest */
			int dist0i=rgbdist(&desired[i], &palette[0]);
			int dist1i=rgbdist(&desired[i], &palette[1]);
			int dist01=rgbdist(&palette[0],&palette[1]);
			if ( dist0i < dist1i ) {
				if ( dist0i <= dist01 ) {
					/* merge 0 and i */
					palette[0]=TCOD_color_lerp(desired[i],palette[0],weight[0]/(1.0f+weight[0]));
					weight[0]++;
				} else {
					/* merge 0 and 1 */
					palette[0]=TCOD_color_lerp(palette[0],palette[1],(float)(weight[1])/(weight[0]+weight[1]));
					weight[0]++;
					palette[1]=desired[i];
					flag=1<<(i-1);
				}
			} else {
				if ( dist1i <= dist01 ) {
					/* merge 1 and i */
					palette[1]=TCOD_color_lerp(desired[i],palette[1],weight[1]/(1.0f+weight[1]));
					weight[1]++;
					flag|=1<<(i-1);
				} else {
					/* merge 0 and 1 */
					palette[0]=TCOD_color_lerp(palette[0],palette[1],(float)(weight[1])/(weight[0]+weight[1]));
					weight[0]++;
					palette[1]=desired[i];
					flag=1<<(i-1);
				}
			}
		}
		i++;
	}
	*ascii=flagToAscii[flag];
}

void TCOD_image_blit_2x(TCOD_image_t image, TCOD_console_t con, int dx, int dy, int sx, int sy, int w, int h) {
	TCOD_color_t grid[4];
	TCOD_color_t cols[2];
	int nbCols;
	int width,height,ascii,cx,cy;
	TCOD_console_data_t *dat = con ? (TCOD_console_data_t *)(con) : TCOD_ctx.root;
	image_data_t *img=(image_data_t *)image;
	int maxx,maxy;
	TCOD_IFNOT(image != NULL && dat != NULL) return;

	TCOD_image_get_size(image,&width,&height);
	if ( w == -1 ) w=width;
	if ( h == -1 ) h=height;

	/* check that the sx,sy/w,h rectangle is inside the image */
	TCOD_ASSERT(sx >= 0 && sy >= 0 && sx+w <= width && sy+h <= height);
	TCOD_IFNOT(w > 0 && h > 0) return;

	sx=MAX(0,sx);
	sy=MAX(0,sy);
	w = MIN(w,width-sx);
	h = MIN(h,height-sy);

	maxx=dx+w/2 <= dat->w ? w : (dat->w-dx)*2;
	maxy=dy+h/2 <= dat->h ? h : (dat->h-dy)*2;
	/* check that the image is not blitted outside the console */
	TCOD_IFNOT(dx+maxx/2 >= 0 && dy+maxy/2 >= 0 && dx < dat->w && dy < dat->h) return;
	maxx+=sx;
	maxy+=sy;

	for (cx=sx; cx < maxx; cx += 2) {
		for (cy=sy; cy < maxy; cy += 2) {
			/* get the 2x2 super pixel colors from the image */
			int conx=dx+(cx-sx)/2;
			int cony=dy+(cy-sy)/2;
			TCOD_color_t consoleBack=TCOD_console_get_char_background(con,conx,cony);
			grid[0]=TCOD_image_get_pixel(image,cx,cy);
			if ( img->has_key_color && grid[0].r == img->key_color.r  && grid[0].g == img->key_color.g && grid[0].b == img->key_color.b)
				grid[0]=consoleBack;
			if ( cx < maxx-1 ) {
				grid[1]=TCOD_image_get_pixel(image,cx+1,cy);
				if ( img->has_key_color && grid[1].r == img->key_color.r  && grid[1].g == img->key_color.g && grid[1].b == img->key_color.b)
					grid[1]=consoleBack;
			} else grid[1]=consoleBack;
			if ( cy < maxy-1 ) {
				grid[2]=TCOD_image_get_pixel(image,cx,cy+1);
				if ( img->has_key_color && grid[2].r == img->key_color.r  && grid[2].g == img->key_color.g && grid[2].b == img->key_color.b)
					grid[2]=consoleBack;
			} else grid[2]=consoleBack;
			if ( cx < maxx-1 && cy < maxy-1 ) {
				grid[3]=TCOD_image_get_pixel(image,cx+1,cy+1);
				if ( img->has_key_color && grid[3].r == img->key_color.r  && grid[3].g == img->key_color.g && grid[3].b == img->key_color.b)
					grid[3]=consoleBack;
			} else grid[3]=consoleBack;
			/* analyse color, posterize, get pattern */
			getPattern(grid,cols,&nbCols,&ascii);
			if ( nbCols == 1 ) {
				/* single color */
				TCOD_console_set_char_background(con,conx,cony,cols[0],TCOD_BKGND_SET);
				TCOD_console_set_char(con,conx,cony,' ');
			} else {
				if ( ascii >= 0 ) {
					TCOD_console_set_default_background(con,cols[0]);
					TCOD_console_set_default_foreground(con,cols[1]);
					TCOD_console_put_char(con,conx,cony,ascii,TCOD_BKGND_SET);
				} else {
					/* negative ascii code means we need to invert back/fore colors */
					TCOD_console_set_default_background(con,cols[1]);
					TCOD_console_set_default_foreground(con,cols[0]);
					TCOD_console_put_char(con,conx,cony,-ascii,TCOD_BKGND_SET);
				}
			}
		}
	}
}



