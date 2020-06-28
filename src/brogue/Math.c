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

fixpt fp_round(fixpt x) {
    long long div = x / FP_FACTOR, rem = x % FP_FACTOR;
    int sign = (x >= 0) - (x < 0);

    if (rem >= FP_FACTOR / 2 || rem <= -FP_FACTOR / 2) {
        return div + sign;
    } else {
        return div;
    }
}

// Returns the bit position of the most significant bit of x, where the unit
// bit has position 1. Returns 0 if x=0.
static int msbpos(unsigned long long x) {
    if (x == 0) return 0;
    int n = 0;
    do {
        n += 1;
    } while (x >>= 1);
    return n;
}

static fixpt fp_exp2(int n) {
    return (n >= 0 ? FP_FACTOR << n : FP_FACTOR >> -n);
}

// Calculates sqrt(u) using the bisection method to find the root of
// f(x) = x^2 - u.
fixpt fp_sqrt(fixpt u) {

    if (u < 0) return -fp_sqrt(-u);
    if (u == 0 || u == FP_FACTOR) return u;

    // Find the unique k such that 2^(k-1) <= u < 2^k
    // FP_BASE is the msbpos-1 of FP_FACTOR ("one")
    int k = msbpos(u) - FP_BASE;

    fixpt x = 0, fx, upper, lower;
    // Since 2^(k-1) <= u < 2^k, we have 2^(ceil(k/2)-1) <= sqrt(u) < 2^ceil(k/2).
    // First ineq. from sqrt(u) >= 2^[(k-1)/2] = 2^[k/2 + 1/2 - 1] >= 2^(ceil(k/2) - 1)
    // To calculate ceil(k/2), do k/2 but add 1 to k if positive.
    upper = fp_exp2((k + (k > 0))/2);
    lower = upper / 2;

    while (upper != lower + 1) {
        x = (upper + lower) / 2;
        fx = FP_MUL(x, x) - u;

        if (fx == 0) {
            break;
        } else if (fx > 0) {
            upper = x;
        } else {
            lower = x;
        }
    }

    return x;
}

// Returns base to the power of expn
fixpt fp_pow(fixpt base, int expn) {
    if (base == 0) return 0;

    if (expn < 0) {
        base = FP_DIV(FP_FACTOR, base);
        expn = -expn;
    }

    fixpt res = FP_FACTOR, err = 0;
    while (expn--) {
        res = res * base + (err * base) / FP_FACTOR;
        err = res % FP_FACTOR;
        res /= FP_FACTOR;
    }

    return res + fp_round(err);
}
