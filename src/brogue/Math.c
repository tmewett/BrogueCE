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

void raninit( ranctx *x, uint64_t seed ) {
    u4 i;
    x->a = 0xf1ea5eed, x->b = x->c = x->d = (u4)seed;
    x->c ^= (u4)(seed >> 32);
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

long range(long n, short RNG) {
    unsigned long div;
    long r;

    div = RAND_MAX_COMBO/n;

    do {
        r = ranval(&(RNGState[RNG])) / div;
    } while (r >= n);

    return r;
}

// Get a random int between lowerBound and upperBound, inclusive, with uniform probability distribution

#ifdef AUDIT_RNG // debug version
long rand_range(long lowerBound, long upperBound) {
    int retval;
    char RNGMessage[100];
    if (upperBound <= lowerBound) {
        return lowerBound;
    }
    long interval = upperBound - lowerBound + 1;
    brogueAssert(interval > 1); // to verify that we didn't wrap around
    retval = lowerBound + range(interval, rogue.RNG);
    if (rogue.RNG == RNG_SUBSTANTIVE) {
        randomNumbersGenerated++;
        if (1) { //randomNumbersGenerated >= 1128397) {
            sprintf(RNGMessage, "\n#%lu, %ld to %ld: %ld", randomNumbersGenerated, lowerBound, upperBound, retval);
            RNGLog(RNGMessage);
        }
    }
    return retval;
}
#else // normal version
long rand_range(long lowerBound, long upperBound) {
    if (upperBound <= lowerBound) {
        return lowerBound;
    }
    if (rogue.RNG == RNG_SUBSTANTIVE) {
        randomNumbersGenerated++;
    }
    long interval = upperBound - lowerBound + 1;
    brogueAssert(interval > 1); // to verify that we didn't wrap around
    return lowerBound + range(interval, rogue.RNG);
}
#endif

uint64_t rand_64bits() {
    if (rogue.RNG == RNG_SUBSTANTIVE) {
        randomNumbersGenerated++;
    }
    uint64_t hi = ranval(&(RNGState[rogue.RNG]));
    uint64_t lo = ranval(&(RNGState[rogue.RNG]));
    return (hi << 32) | lo;
}

// seeds with the time if called with a parameter of 0; returns the seed regardless.
// All RNGs are seeded simultaneously and identically.
uint64_t seedRandomGenerator(uint64_t seed) {
    if (seed == 0) {
        seed = (uint64_t) time(NULL) - 1352700000;
    }
    raninit(&(RNGState[RNG_SUBSTANTIVE]), seed);
    raninit(&(RNGState[RNG_COSMETIC]), seed);
    return seed;
}


/* ----------------------------------------------------------------------
    Fixed-point arithmetic

For clarity, we'll use `fixpt` for numbers to be interpreted as
real numbers (a multiple of 1/FP_ONE) and `long` for numbers
to be interpreted as integers (a multiple of 1).
*/

// Converts an integer to fixed-point number.
fixpt fp(long n) {
    return n * FP_ONE;
}

// Converts a fixed-point number to integer, truncating toward 0.
long fp_trunc(fixpt x) {
    return x / FP_ONE;
}

// Rounds a fixed-point number to the nearest integer.
long fp_round(fixpt x) {
    return x >= 0 ? (x + FP_ONE/2) / FP_ONE : (x - FP_ONE/2) / FP_ONE;
}

// Returns the magnitude of x.
fixpt fp_abs(fixpt x) {
    return x >= 0 ? x : -x;
}

// Multiplies two fixed-point numbers. Last bit is rounded.
fixpt fp_mul(fixpt x, fixpt y) {
    //return (x * y) / FP_ONE; // truncated
    return (x<0)==(y<0) ? (x * y + FP_ONE/2) / FP_ONE : (x * y - FP_ONE/2) / FP_ONE; // rounded
}

// Divides x by y. Last bit is rounded.
fixpt fp_div(fixpt x, fixpt y) {
    if (x == 0) return 0;
    if (y == 0) return x > 0 ? LLONG_MAX : LLONG_MIN;
    //return (x * FP_ONE) / y; // truncated
    return (x<0)==(y<0) ? (x * FP_ONE + y/2) / y : (x * FP_ONE - y/2) / y; // rounded
}

// Divides n1 by n2, returning a fixpt.
fixpt fp_ratio(long n1, long n2) {
    // They are the same function, but with different signatures
    // to make it clear that input values don't represent fixed-point reals.
    return fp_div(n1, n2);
}

// Returns x rounded to the nearest multiple of 1/n
fixpt fp_quantize(fixpt x, int n) {
    return (((x * n + FP_ONE/2) / FP_ONE) * FP_ONE + n/2) / n;
}

// Returns the square root of x.
// Output is precise within 1 bit of the real value.
fixpt fp_sqrt(fixpt x) {
    // // Floating-point math is faster, but rounding error may differ between CPU generations.
    // return round(sqrt((double)x / FP_ONE) * FP_ONE);

    if (x == 0 || x == FP_ONE) return x;
    if (x < 0) return -fp_sqrt(-x);

    // Compute the square root with the Babylonian method.

    brogueAssert(x < INT64_MAX / (FP_ONE+1)); // above this we risk overflowing
    // initial estimate (the closer we can get, the fewer iterations)
    fixpt z = (x > FP_ONE*100 ? FP_ONE*20 : x < FP_ONE/100 ? FP_ONE/20 : FP_ONE);
    // converges in ~6 iterations (3 times faster than the bisection method)
    for (int i = 0; i < 32; i++) {
        fixpt t = z + (fp_div(x, z) - z) / 2;
        if (t == z) break;
        z = t;
    }
    return z;
}

// Fixed-point number with error
// x = val + err/FP_ONE
typedef struct fpe {
    fixpt val, err;
} fpe;

// Compares two numbers.
static int fp_cmp2(fpe x, fpe y) {
    fixpt t = (x.val * FP_ONE + x.err) - (y.val * FP_ONE + y.err);
    return t > 0 ? 1 : t < 0 ? -1 : 0;
}

// Averages two numbers.
static fpe fp_avg2(fpe x, fpe y) {
    fixpt t = ((x.val + y.val) * FP_ONE + (x.err + y.err))/2;
    return (fpe) { t / FP_ONE, t % FP_ONE };
}

// Multiplies two numbers.
static fpe fp_mul2(fpe x, fpe y) {
    fixpt t = x.val * y.val
            + (x.val * y.err + y.val * x.err + FP_ONE / 2) / FP_ONE
            + (x.err * y.err + FP_ONE * FP_ONE / 2) / (FP_ONE * FP_ONE);
    return (fpe) { t / FP_ONE, t % FP_ONE };
}

// Computes 1/x (ignoring the error term).
static fpe fp_inv2(fpe x) {
    fixpt t = (FP_ONE * FP_ONE * FP_ONE) / x.val;
    return (fpe) { t / FP_ONE, t % FP_ONE};
}

// Computes the precise square root.
static fpe fp_sqrt2(fpe x) {
    // start with the Babylonian method to get the square root with some rounding error
    fixpt root = fp_sqrt(x.val);
    // use the bisection method to get the full precision
    fpe mid, lo = (fpe) {root-2, 0}, hi = (fpe) {root+2, 0};
    for (int i = 0; i < 64; i++) {
        mid = fp_avg2(lo, hi);
        int cmp = fp_cmp2(fp_mul2(mid, mid), x);
        if (cmp == 0) return mid; // spot on!
        if (cmp > 0) hi = mid; else lo = mid;
        if (lo.val * FP_ONE + lo.err + 1 >= hi.val * FP_ONE + hi.err) break;
    }
    return lo;
}

// Returns x to the power of y (fixed-point).
fixpt fp_pow(fixpt x, fixpt y) {
    // // Floating-point math is faster, but rounding error may differ between CPU generations.
    // return round(pow((double)x / FP_ONE, (double)y / FP_ONE) * FP_ONE);

    if (y == 0 || x == FP_ONE) return FP_ONE;
    if (x == 0 || y == FP_ONE) return x;

    // let's work with positive numbers, we'll set the sign at the end
    fpe f = (fpe) {fp_abs(x), 0};
    if (y < 0) y = -y, f = fp_inv2(f);

    // Exponentiation by squaring

    fixpt n = y / FP_ONE;
    fpe z = (fpe) {FP_ONE, 0}, t = f;
    for (int i = 0; i < 64; i++) {
        if (n & 1) z = fp_mul2(z, t);
        n /= 2;
        if (!n) break;
        t = fp_mul2(t, t);
    }

    // Fractional part

    n = y % FP_ONE;
    t = f;
    for (int i = 0; n && i < 64; i++) {
        t = fp_sqrt2(t);
        if (t.val == FP_ONE && t.err == 0) break;
        n *= 2;
        if (n >= FP_ONE) {
            n -= FP_ONE;
            z = fp_mul2(z, t);
        }
    }

    fixpt out = z.val + fp_round(z.err);
    return (x < 0) && ((y / FP_ONE) & 1) ? -out : out;
}

// Returns x to the power of n (integer).
fixpt fp_ipow(fixpt x, int n) {
    if (n == 2) return fp_mul(x, x); // shortcut
    return fp_pow(x, n*FP_ONE);
}
