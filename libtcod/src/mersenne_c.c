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

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "libtcod.h"
#include "libtcod_int.h"

static TCOD_random_t instance=NULL;
static float rand_div=1.0f/(float)(0xffffffff);
static double rand_div_double = 1.0 / (double)(0xffffffff);

/* initialize the mersenne twister array */
static void mt_init(uint32 seed, uint32 mt[624] )
{
	int i;
    mt[0]= seed;
    for (i=1; i<624; i++) {
        mt[i] = (1812433253 * (mt[i-1] ^ (mt[i-1] >> 30)) + i);
    }
}

/* get the next random value from the mersenne twister array */
static uint32 mt_rand(uint32 mt[624], int *cur_mt)
{
#define MT_HIGH_BIT 0x80000000UL
#define MT_LOW_BITS 0x7fffffffUL
    uint32 y;

    if (*cur_mt == 624) {
    	/* our 624 sequence is finished. generate a new one */
        int i;

        for (i=0;i<623;i++) {
            y = ( mt[i] & MT_HIGH_BIT ) | ( mt[i+1] & MT_LOW_BITS );
            if ( y & 1 ) {
            	/* odd y */
	            mt[i] = mt[ (i + 397) % 624 ] ^ (y >> 1) ^ 2567483615UL;
			} else {
				/* even y */
	            mt[i] = mt[ (i + 397) % 624 ] ^ (y >> 1);
			}
        }
        y = ( mt[623] & MT_HIGH_BIT ) | ( mt[0] & MT_LOW_BITS );
        if ( y & 1 ) {
        	/* odd y */
	        mt[623] = mt[396] ^ (y >> 1) ^ 2567483615UL;
		} else {
	        mt[623] = mt[396] ^ (y >> 1);
		}

        *cur_mt = 0;
    }

    y = mt[(*cur_mt)++];
    y ^= (y >> 11);
    y ^= (y << 7) & 2636928640UL;
    y ^= (y << 15) & 4022730752UL;
    y ^= (y >> 18);
    return y;
}

/* get a random float between 0 and 1 */
static float frandom01(mersenne_data_t *r) {
	return (float)mt_rand(r->mt,&r->cur_mt)*rand_div;
}

/* string hashing function */
/* not used (yet)
static uint32 hash(const char *data,int len) {
	uint32 hash = 0;
	uint32 x;
	int i;
	for(i = 0; i < len; data++, i++) {
		hash = (hash << 4) + (*data);
		if((x = hash & 0xF0000000L) != 0) {
			hash ^= (x >> 24);
			hash &= ~x;
		}
	}
	return (hash & 0x7FFFFFFF);
}
*/

/* get a random number from the CMWC */
#define CMWC_GET_NUMBER(num) { unsigned long long t; uint32 x; r->cur=(r->cur+1)&4095; t=18782LL*r->Q[r->cur]+r->c; r->c=(t>>32); x=(uint32)(t+r->c); if (x < r->c) { x++; r->c++; } if((x+1)==0) { r->c++; x=0; } num = (uint32)(r->Q[r->cur] = 0xfffffffe - x); }

TCOD_random_t TCOD_random_new(TCOD_random_algo_t algo) {
    return TCOD_random_new_from_seed(algo,(uint32)time(0));
}

TCOD_random_t TCOD_random_get_instance(void) {
	if (! instance ) {
		instance=TCOD_random_new(TCOD_RNG_CMWC);
	}
	return instance;
}

TCOD_random_t TCOD_random_new_from_seed(TCOD_random_algo_t algo, uint32 seed) {
	mersenne_data_t *r = (mersenne_data_t *)calloc(sizeof(mersenne_data_t),1);
	/* Mersenne Twister */
	if (algo == TCOD_RNG_MT) {
        r->algo = TCOD_RNG_MT;
        r->cur_mt=624;
        mt_init(seed,r->mt);
	}
	/* Complementary-Multiply-With-Carry or Generalised Feedback Shift Register */
	else {
	    int i;
        /* fill the Q array with pseudorandom seeds */
        uint32 s = seed;
        for (i = 0; i < 4096; i++) r->Q[i] = s = (s * 1103515245) + 12345; /* glibc LCG */
        r->c = ((s * 1103515245) + 12345) % 809430660; /* this max value is recommended by George Marsaglia */
        r->cur = 0;
        r->algo = TCOD_RNG_CMWC;
	}
	r->distribution = TCOD_DISTRIBUTION_LINEAR;
	return (TCOD_random_t)r;
}


int TCOD_random_get_i(TCOD_random_t mersenne, int min, int max) {
	mersenne_data_t *r;
	int delta;
	if (max==min) return min;
	else if (max < min) {
		int tmp=max;
		max=min;
		min=tmp;
	}
	if (!mersenne) mersenne=TCOD_random_get_instance();
	r=(mersenne_data_t *)mersenne;
	delta = max - min + 1;
	/* return a number from the Mersenne Twister */
	if (r->algo == TCOD_RNG_MT) return ( mt_rand(r->mt,&r->cur_mt)  % delta ) + min;
	/* or from the CMWC */
	else {
	    uint32 number;
	    CMWC_GET_NUMBER(number)
	    return number % delta + min;
	}
}

float TCOD_random_get_f(TCOD_random_t mersenne,float min, float max) {
	mersenne_data_t *r;
	float delta,f;
	if (max==min) return min;
	else if (max < min) {
		float tmp=max;
		max=min;
		min=tmp;
	}
	if (!mersenne) mersenne=TCOD_random_get_instance();
	r=(mersenne_data_t *)mersenne;
	delta = max - min;
	/* Mersenne Twister */
	if (r->algo == TCOD_RNG_MT) f = delta * frandom01(r);
	/* CMWC */
	else {
	    uint32 number;
	    CMWC_GET_NUMBER(number)
	    f = (float)(number) * rand_div * delta;
	}
	return min + f;
}

double TCOD_random_get_d(TCOD_random_t mersenne, double min, double max) {
	mersenne_data_t *r;
	double delta,f;
	if (max==min) return min;
	else if (max < min) {
		double tmp=max;
		max=min;
		min=tmp;
	}
	if (!mersenne) mersenne=TCOD_random_get_instance();
	r=(mersenne_data_t *)mersenne;
	delta = max - min;
	/* Mersenne Twister */
	if (r->algo == TCOD_RNG_MT) f = delta * (double)frandom01(r);
	/* CMWC */
	else {
	    uint32 number;
	    CMWC_GET_NUMBER(number)
	    f = (double)(number) * rand_div_double * delta;
	}
	return min + f;
}

void TCOD_random_delete(TCOD_random_t mersenne) {
	TCOD_IFNOT(mersenne != NULL) return;
	if ( mersenne == instance ) instance = NULL;
	free(mersenne);
}
TCOD_random_t TCOD_random_save(TCOD_random_t mersenne) {
	mersenne_data_t *ret=(mersenne_data_t *)malloc(sizeof(mersenne_data_t));
	if (!mersenne) mersenne=TCOD_random_get_instance();
	memcpy(ret,mersenne,sizeof(mersenne_data_t));
	return (TCOD_random_t)ret;
}

void TCOD_random_restore(TCOD_random_t mersenne, TCOD_random_t backup) {
	if (!mersenne) mersenne=TCOD_random_get_instance();
	memcpy(mersenne,backup,sizeof(mersenne_data_t));
}

/* Box-Muller transform (Gaussian distribution) */

double TCOD_random_get_gaussian_double (TCOD_random_t mersenne, double mean, double std_deviation) {
	double x1, x2, w, y1;
	static double y2;
	static bool again = false;
	double ret;
	if (again)
		ret = mean + y2 * std_deviation;
	else {
        mersenne_data_t *r = NULL;
		if (!mersenne) mersenne=TCOD_random_get_instance();
		r = (mersenne_data_t *)mersenne;
		/* MT */
		if (r->algo == TCOD_RNG_MT) {
			do {
				x1 = frandom01(r) * 2.0 - 1.0;
				x2 = frandom01(r) * 2.0 - 1.0;
				w = x1 * x1 + x2 * x2;
			} while (w >= 1.0);
		}
		/* CMWC */
		else {
			uint32 number;
			do {
				CMWC_GET_NUMBER(number)
				x1 = number * rand_div_double * 2.0 - 1.0;
				CMWC_GET_NUMBER(number)
				x2 = number * rand_div_double * 2.0 - 1.0;
				w = x1 * x1 + x2 * x2;
			} while (w >= 1.0);
		}
		w = sqrt((-2.0 * log(w)) / w);
		y1 = x1 * w;
		y2 = x2 * w;
		ret = mean + y1 * std_deviation;
	}
	again = !again;
	return ret;
}

float TCOD_random_get_gaussian_float (TCOD_random_t mersenne, float mean, float std_deviation) {
	return (float)TCOD_random_get_gaussian_double(mersenne, (double)mean, (double)std_deviation);
}

int TCOD_random_get_gaussian_int (TCOD_random_t mersenne, int mean, int std_deviation) {
	double num = TCOD_random_get_gaussian_double(mersenne,(double)mean,(double)std_deviation);
	return (num >= 0.0 ? (int)(num + 0.5) : (int)(num - 0.5));
}

/* Box-Muller, ranges */

double TCOD_random_get_gaussian_double_range (TCOD_random_t mersenne, double min, double max) {
	double mean, std_deviation, ret;
	if (min > max) {
		double tmp = max;
		max = min;
		min = tmp;
	}
	mean = (min + max) / 2;
	std_deviation = (max - min) / 6.0; /* 6.0 is used because of the three-sigma rule */
	ret = TCOD_random_get_gaussian_double(mersenne, mean, std_deviation);
	return CLAMP(min,max,ret);
}

float TCOD_random_get_gaussian_float_range (TCOD_random_t mersenne, float min, float max) {
	if (min > max) {
		float tmp = max;
		max = min;
		min = tmp;
	}
	return (float)TCOD_random_get_gaussian_double_range (mersenne, (double)min, (double)max);
}

int TCOD_random_get_gaussian_int_range (TCOD_random_t mersenne, int min, int max) {
	double num;
	int ret;
	if (min > max) {
		int tmp = max;
		max = min;
		min = tmp;
	}
	num = TCOD_random_get_gaussian_double_range (mersenne, (double)min, (double)max);
	ret = (num >= 0.0 ? (int)(num + 0.5) : (int)(num - 0.5));
	return CLAMP(min,max,ret);
}

/* Box-Muller, ranges with a custom mean */

double TCOD_random_get_gaussian_double_range_custom (TCOD_random_t mersenne, double min, double max, double mean) {
	double d1, d2, std_deviation, ret;
	if (min > max) {
		double tmp = max;
		max = min;
		min = tmp;
	}
	d1 = max - mean;
	d2 = mean - min;
	std_deviation = MAX(d1,d2) / 3.0;
	ret = TCOD_random_get_gaussian_double(mersenne, mean, std_deviation);
	return CLAMP(min,max,ret);
}

float TCOD_random_get_gaussian_float_range_custom (TCOD_random_t mersenne, float min, float max, float mean) {
	if (min > max) {
		float tmp = max;
		max = min;
		min = tmp;
	}
	return (float)TCOD_random_get_gaussian_double_range_custom (mersenne, (double)min, (double)max, (double)mean);
}

int TCOD_random_get_gaussian_int_range_custom (TCOD_random_t mersenne, int min, int max, int mean) {
	double num;
	int ret;
	if (min > max) {
		int tmp = max;
		max = min;
		min = tmp;
	}
	num = TCOD_random_get_gaussian_double_range_custom (mersenne, (double)min, (double)max, (double)mean);
	ret = (num >= 0.0 ? (int)(num + 0.5) : (int)(num - 0.5));
	return CLAMP(min,max,ret);
}

/* Box-Muller, inverted distribution */

double TCOD_random_get_gaussian_double_inv (TCOD_random_t mersenne, double mean, double std_deviation) {
	double num = TCOD_random_get_gaussian_double(mersenne,mean,std_deviation);
	return (num >= mean ? num - (3 * std_deviation) : num + (3 * std_deviation));
}

float TCOD_random_get_gaussian_float_inv (TCOD_random_t mersenne, float mean, float std_deviation) {
	double num = TCOD_random_get_gaussian_double(mersenne,(double)mean,(double)std_deviation);
	return (num >= mean ? (float)(num - (3 * std_deviation)) : (float)(num + (3 * std_deviation)));
}

int TCOD_random_get_gaussian_int_inv (TCOD_random_t mersenne, int mean, int std_deviation) {
	double num = TCOD_random_get_gaussian_double(mersenne,(double)mean,(double)std_deviation);
	int inum = (num >= 0.0 ? (int)(num + 0.5) : (int)(num - 0.5));
	return (num >= mean ? inum - (3 * std_deviation) : inum + (3 * std_deviation));
}

/* Box-Muller, ranges, inverted distribution */

double TCOD_random_get_gaussian_double_range_inv (TCOD_random_t mersenne, double min, double max) {
	double mean, std_deviation, ret;
	if (min > max) {
		double tmp = max;
		max = min;
		min = tmp;
	}
	mean = (min + max) / 2.0;
	std_deviation = (max - min) / 6.0; /* 6.0 is used because of the three-sigma rule */
	ret = TCOD_random_get_gaussian_double_inv(mersenne, mean, std_deviation);
	return CLAMP(min,max,ret);
}

float TCOD_random_get_gaussian_float_range_inv (TCOD_random_t mersenne, float min, float max) {
	float ret = (float)TCOD_random_get_gaussian_double_range_inv(mersenne, (double)min, (double)max);
	return CLAMP(min,max,ret);
}

int TCOD_random_get_gaussian_int_range_inv (TCOD_random_t mersenne, int min, int max) {
	double num = TCOD_random_get_gaussian_double_range_inv(mersenne, (double)min, (double)max);
	int ret = (num >= 0.0 ? (int)(num + 0.5) : (int)(num - 0.5));
	return CLAMP(min,max,ret);
}

/* Box-Muller, ranges with a custom mean, inverted distribution */

double TCOD_random_get_gaussian_double_range_custom_inv (TCOD_random_t mersenne, double min, double max, double mean) {
	double d1, d2, std_deviation, ret;
	if (min > max) {
		double tmp = max;
		max = min;
		min = tmp;
	}
	d1 = max - mean;
	d2 = mean - min;
	std_deviation = MAX(d1,d2) / 3.0;
	ret = TCOD_random_get_gaussian_double_inv(mersenne, mean, std_deviation);
	return CLAMP(min,max,ret);
}

float TCOD_random_get_gaussian_float_range_custom_inv (TCOD_random_t mersenne, float min, float max, float mean) {
	float ret = (float)TCOD_random_get_gaussian_double_range_custom_inv(mersenne, (double)min, (double)max, (double)mean);
	return CLAMP(min,max,ret);
}

int TCOD_random_get_gaussian_int_range_custom_inv (TCOD_random_t mersenne, int min, int max, int mean) {
	double num = TCOD_random_get_gaussian_double_range_custom_inv(mersenne, (double)min, (double)max, (double)mean);
	int ret = (num >= 0.0 ? (int)(num + 0.5) : (int)(num - 0.5));
	return CLAMP(min,max,ret);
}

void TCOD_random_set_distribution (TCOD_random_t mersenne, TCOD_distribution_t distribution) {
    mersenne_data_t *r = NULL;
	if (!mersenne) mersenne=TCOD_random_get_instance();
	r = (mersenne_data_t *)mersenne;
	r->distribution = distribution;
}

int TCOD_random_get_int (TCOD_random_t mersenne, int min, int max) {
	if (!mersenne) mersenne=TCOD_random_get_instance();
	switch (((mersenne_data_t *)mersenne)->distribution) {
		case TCOD_DISTRIBUTION_LINEAR: return TCOD_random_get_i(mersenne, min, max); break;
		case TCOD_DISTRIBUTION_GAUSSIAN: return TCOD_random_get_gaussian_int(mersenne, min, max); break;
		case TCOD_DISTRIBUTION_GAUSSIAN_INVERSE: return TCOD_random_get_gaussian_int_inv(mersenne, min, max); break;
		case TCOD_DISTRIBUTION_GAUSSIAN_RANGE: return TCOD_random_get_gaussian_int_range(mersenne, min, max); break;
		case TCOD_DISTRIBUTION_GAUSSIAN_RANGE_INVERSE: return TCOD_random_get_gaussian_int_range_inv(mersenne, min, max); break;
		default: return TCOD_random_get_i(mersenne, min, max); break;
	}
}

float TCOD_random_get_float (TCOD_random_t mersenne, float min, float max) {
	if (!mersenne) mersenne=TCOD_random_get_instance();
	switch (((mersenne_data_t *)mersenne)->distribution) {
		case TCOD_DISTRIBUTION_LINEAR: return TCOD_random_get_f(mersenne, min, max); break;
		case TCOD_DISTRIBUTION_GAUSSIAN: return TCOD_random_get_gaussian_float(mersenne, min, max); break;
		case TCOD_DISTRIBUTION_GAUSSIAN_INVERSE: return TCOD_random_get_gaussian_float_inv(mersenne, min, max); break;
		case TCOD_DISTRIBUTION_GAUSSIAN_RANGE: return TCOD_random_get_gaussian_float_range(mersenne, min, max); break;
		case TCOD_DISTRIBUTION_GAUSSIAN_RANGE_INVERSE: return TCOD_random_get_gaussian_float_range_inv(mersenne, min, max); break;
		default: return TCOD_random_get_f(mersenne, min, max); break;
	}
}

double TCOD_random_get_double (TCOD_random_t mersenne, double min, double max) {
	if (!mersenne) mersenne=TCOD_random_get_instance();
	switch (((mersenne_data_t *)mersenne)->distribution) {
		case TCOD_DISTRIBUTION_LINEAR: return TCOD_random_get_d(mersenne, min, max); break;
		case TCOD_DISTRIBUTION_GAUSSIAN: return TCOD_random_get_gaussian_double(mersenne, min, max); break;
		case TCOD_DISTRIBUTION_GAUSSIAN_INVERSE: return TCOD_random_get_gaussian_double_inv(mersenne, min, max); break;
		case TCOD_DISTRIBUTION_GAUSSIAN_RANGE: return TCOD_random_get_gaussian_double_range(mersenne, min, max); break;
		case TCOD_DISTRIBUTION_GAUSSIAN_RANGE_INVERSE: return TCOD_random_get_gaussian_double_range_inv(mersenne, min, max); break;
		default: return TCOD_random_get_d(mersenne, min, max); break;
	}
}

int TCOD_random_get_int_mean (TCOD_random_t mersenne, int min, int max, int mean) {
	if (!mersenne) mersenne=TCOD_random_get_instance();
	switch (((mersenne_data_t *)mersenne)->distribution) {
		case TCOD_DISTRIBUTION_GAUSSIAN_INVERSE:
		case TCOD_DISTRIBUTION_GAUSSIAN_RANGE_INVERSE: return TCOD_random_get_gaussian_int_range_custom_inv(mersenne, min, max, mean); break;
		default: return TCOD_random_get_gaussian_int_range_custom(mersenne, min, max, mean); break;
	}
}

float TCOD_random_get_float_mean (TCOD_random_t mersenne, float min, float max, float mean) {
	if (!mersenne) mersenne=TCOD_random_get_instance();
	switch (((mersenne_data_t *)mersenne)->distribution) {
		case TCOD_DISTRIBUTION_GAUSSIAN_INVERSE:
		case TCOD_DISTRIBUTION_GAUSSIAN_RANGE_INVERSE: return TCOD_random_get_gaussian_float_range_custom_inv(mersenne, min, max, mean); break;
		default: return TCOD_random_get_gaussian_float_range_custom(mersenne, min, max, mean); break;
	}
}

double TCOD_random_get_double_mean (TCOD_random_t mersenne, double min, double max, double mean) {
	if (!mersenne) mersenne=TCOD_random_get_instance();
	switch (((mersenne_data_t *)mersenne)->distribution) {
		case TCOD_DISTRIBUTION_GAUSSIAN_INVERSE:
		case TCOD_DISTRIBUTION_GAUSSIAN_RANGE_INVERSE: return TCOD_random_get_gaussian_double_range_custom_inv(mersenne, min, max, mean); break;
		default: return TCOD_random_get_gaussian_double_range_custom(mersenne, min, max, mean); break;
	}
}

TCOD_dice_t TCOD_random_dice_new (const char * s) {
	TCOD_dice_t d = { 1, 1, 1.0f, 0.0f };
	char * ptr = (char *)s;
	char tmp[128];
	size_t l;
	/* get multiplier */
	if ((l = strcspn(ptr,"*x")) < strlen(ptr)) {
		strcpy(tmp,ptr);
		tmp[l] = '\0';
		d.multiplier = (float)atof(tmp);
		ptr += l + 1;
	}
	/* get rolls */
	l = strcspn(ptr,"dD");
	strcpy(tmp,ptr);
	tmp[l] = '\0';
	d.nb_rolls = atoi(tmp);
	ptr += l + 1;
	/* get faces */
	l = strcspn(ptr,"-+");
	strcpy(tmp,ptr);
	tmp[l] = '\0';
	d.nb_faces = atoi(tmp);
	ptr += l;
	/* get addsub */
	if (strlen(ptr) > 0) {
		int sign = (*ptr == '+') ? 1 : (-1);
		ptr++;
		d.addsub = (float)(atof(ptr) * sign);
	}
	return d;
}

int TCOD_random_dice_roll (TCOD_random_t mersenne, TCOD_dice_t dice) {
	int rolls;
	int result = 0;
	for (rolls = 0; rolls < dice.nb_rolls; rolls++)
		result += TCOD_random_get_i(mersenne,1,dice.nb_faces);
	return (int)((result + dice.addsub) * dice.multiplier);
}

int TCOD_random_dice_roll_s (TCOD_random_t mersenne, const char * s) {
	return TCOD_random_dice_roll(mersenne,TCOD_random_dice_new(s));
}
