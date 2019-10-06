/*
 *  RogueMain.c
 *  Brogue
 *
 *  Created by Brian Walker on 12/26/08.
 *  Copyright 2012. All rights reserved.
 *
 *  This file is part of Brogue.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <math.h>
#include <time.h>
#include <limits.h>
#include <stdint.h> // C99

#include "Rogue.h"
#include "IncludeGlobals.h"


    // Random number generation

short randClump(randomRange theRange) {
    return randClumpedRange(theRange.lowerBound, theRange.upperBound, theRange.clumpFactor);
}

// Get a random int between lowerBound and upperBound, inclusive, with probability distribution
// affected by clumpFactor.
short randClumpedRange(short lowerBound, short upperBound, short clumpFactor) {
    if (upperBound <= lowerBound) {
        return lowerBound;
    }
    if (clumpFactor <= 1) {
        return rand_range(lowerBound, upperBound);
    }

    short i, total = 0, numSides = (upperBound - lowerBound) / clumpFactor;

    for(i=0; i < (upperBound - lowerBound) % clumpFactor; i++) {
        total += rand_range(0, numSides + 1);
    }

    for(; i< clumpFactor; i++) {
        total += rand_range(0, numSides);
    }

    return (total + lowerBound);
}

// Get a random int between lowerBound and upperBound, inclusive
boolean rand_percent(short percent) {
    return (rand_range(0, 99) < clamp(percent, 0, 100));
}

void shuffleList(short *list, short listLength) {
    short i, r, buf;
    for (i=0; i<listLength; i++) {
        r = rand_range(0, listLength-1);
        if (i != r) {
            buf = list[r];
            list[r] = list[i];
            list[i] = buf;
        }
    }
}

void fillSequentialList(short *list, short listLength) {
    short i;
    for (i=0; i<listLength; i++) {
        list[i] = i;
    }
}

//typedef unsigned long int  u4;
typedef uint32_t u4;
typedef struct ranctx { u4 a; u4 b; u4 c; u4 d; } ranctx;

static ranctx RNGState[2];

#define rot(x,k) (((x)<<(k))|((x)>>(32-(k))))
u4 ranval( ranctx *x ) {
    u4 e = x->a - rot(x->b, 27);
    x->a = x->b ^ rot(x->c, 17);
    x->b = x->c + x->d;
    x->c = x->d + e;
    x->d = e + x->a;
    return x->d;
}

void raninit( ranctx *x, u4 seed ) {
    u4 i;
    x->a = 0xf1ea5eed, x->b = x->c = x->d = seed;
    for (i=0; i<20; ++i) {
        (void)ranval(x);
    }
}

/* ----------------------------------------------------------------------
 range

 returns a number between 0 and N-1
 without any bias.

 */

#define RAND_MAX_COMBO ((unsigned long) UINT32_MAX)

int range(int n, short RNG) {
    unsigned long div;
    int r;

    div = RAND_MAX_COMBO/n;

    do {
        r = ranval(&(RNGState[RNG])) / div;
    } while (r >= n);

    return r;
}

// Get a random int between lowerBound and upperBound, inclusive, with uniform probability distribution

#ifdef AUDIT_RNG // debug version
int rand_range(int lowerBound, int upperBound) {
    int retval;
    char RNGMessage[100];

    brogueAssert(lowerBound <= INT_MAX && upperBound <= INT_MAX);

    if (upperBound <= lowerBound) {
        return lowerBound;
    }
    retval = lowerBound + range(upperBound-lowerBound+1, rogue.RNG);
    if (rogue.RNG == RNG_SUBSTANTIVE) {
        randomNumbersGenerated++;
        if (1) { //randomNumbersGenerated >= 1128397) {
            sprintf(RNGMessage, "\n#%lu, %i to %i: %i", randomNumbersGenerated, lowerBound, upperBound, retval);
            RNGLog(RNGMessage);
        }
    }
    return retval;
}
#else // normal version
int rand_range(int lowerBound, int upperBound) {
    brogueAssert(lowerBound <= INT_MAX && upperBound <= INT_MAX);
    if (upperBound <= lowerBound) {
        return lowerBound;
    }
    if (rogue.RNG == RNG_SUBSTANTIVE) {
        randomNumbersGenerated++;
    }
    return lowerBound + range(upperBound-lowerBound+1, rogue.RNG);
}
#endif

// seeds with the time if called with a parameter of 0; returns the seed regardless.
// All RNGs are seeded simultaneously and identically.
unsigned long seedRandomGenerator(unsigned long seed) {
    if (seed == 0) {
        seed = (unsigned long) time(NULL) - 1352700000;
    }
    raninit(&(RNGState[RNG_SUBSTANTIVE]), seed);
    raninit(&(RNGState[RNG_COSMETIC]), seed);
    return seed;
}


    // Fixed-point arithmetic

long long fp_round(long long x) {
    long long div = x / FP_FACTOR, rem = x % FP_FACTOR;
    int sign = (x >= 0) - (x < 0);

    if (rem >= FP_FACTOR / 2 || rem <= -FP_FACTOR / 2) {
        return div + sign;
    } else {
        return div;
    }
}

// As of v1.7.5, Brogue uses this open-source fixed-point square root function
// by Mads A. Elvheim.
// Original file is available here:
// https://gist.github.com/Madsy/1088393/ee0e6c2ca940c25149a08d525d6e713b68636773
// clz() function is taken from a comment by "ruslan-cray" on the same page.

/* Computing the number of leading zeros in a word. */
static int32_t clz(uint32_t x)
{
    int n;

    /* See "Hacker's Delight" book for more details */
    if (x == 0) return 32;
    n = 0;
    if (x <= 0x0000FFFF) {n = n +16; x = x <<16;}
    if (x <= 0x00FFFFFF) {n = n + 8; x = x << 8;}
    if (x <= 0x0FFFFFFF) {n = n + 4; x = x << 4;}
    if (x <= 0x3FFFFFFF) {n = n + 2; x = x << 2;}
    if (x <= 0x7FFFFFFF) {n = n + 1;}

    return n;
}

unsigned long long fp_sqrt(unsigned long long val)
{
    unsigned long long x, v;
    int bitpos;

    if(!val)
        return val;

    /* clz = count-leading-zeros. bitpos is the position of the most significant bit,
        relative to "1" or 1 << FP_BASE */
    bitpos = FP_BASE - clz(val);

    /* Calculate our first estimate.
        We use the identity 2^a * 2^a = 2^(2*a) or:
         sqrt(2^a) = 2^(a/2)
    */
    if(bitpos > 0) /* val > 1 */
        x = (FP_FACTOR)<<(bitpos >> 1u);
    else if(bitpos < 0) /* 0 < val < 1 */
        x = (FP_FACTOR)<<((unsigned)(-bitpos) << 1u);
    else /* val == 1 */
        x = (FP_FACTOR);

    /* We need to scale val with FP_BASE due to the division.
       Also val /= 2, hence the subtraction of one*/
    v = val << (FP_BASE - 1u);

    /* The actual iteration */
    x = (x >> 1u) + v/x;
    x = (x >> 1u) + v/x;
    x = (x >> 1u) + v/x;
    x = (x >> 1u) + v/x;
    return x;
}
