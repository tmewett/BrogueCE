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
#include "GlobalsBase.h"


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

// Test a random roll with a success chance of percent out of 100
boolean rand_percent(short percent) {
    return (rand_range(0, 99) < clamp(percent, 0, 100));
}

void shuffleList(short *list, short listLength) {
    // See https://en.wikipedia.org/wiki/Fisher-Yates_shuffle
    // A "fair" shuffle requires choosing only items between the
    // current index and the end of the array to swap with the
    // current item.
    for (int i = 0; i < listLength - 1; i++) {
        // We can skip the last item, since it could only swap
        // with itself.
        int r = rand_range(i, listLength - 1);
        if (i != r) {
            short buf = list[r];
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
static u4 ranval( ranctx *x ) {
    u4 e = x->a - rot(x->b, 27);
    x->a = x->b ^ rot(x->c, 17);
    x->b = x->c + x->d;
    x->c = x->d + e;
    x->d = e + x->a;
    return x->d;
}

static void raninit( ranctx *x, uint64_t seed ) {
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

static long range(long n, short RNG) {
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

    static const fixpt SQUARE_ROOTS[] = { // values were computed by the code that follows
        0,      65536,  92682,  113511, 131073, 146543, 160529, 173392, 185363, 196608, 207243, 217359, 227023, 236293, 245213, 253819,
        262145, 270211, 278045, 285665, 293086, 300323, 307391, 314299, 321059, 327680, 334169, 340535, 346784, 352923, 358955, 364889,
        370727, 376475, 382137, 387717, 393216, 398640, 403991, 409273, 414487, 419635, 424721, 429749, 434717, 439629, 444487, 449293,
        454047, 458752, 463409, 468021, 472587, 477109, 481589, 486028, 490427, 494786, 499107, 503391, 507639, 511853, 516031, 520175,
        524289, 528369, 532417, 536435, 540423, 544383, 548313, 552217, 556091, 559939, 563762, 567559, 571329, 575077, 578799, 582497,
        586171, 589824, 593453, 597061, 600647, 604213, 607755, 611279, 614783, 618265, 621729, 625173, 628599, 632007, 635395, 638765,
        642119, 645455, 648773, 652075, 655360, 658629, 661881, 665117, 668339, 671545, 674735, 677909, 681071, 684215, 687347, 690465,
        693567, 696657, 699733, 702795, 705845, 708881, 711903, 714913, 717911, 720896, 723869, 726829, 729779, 732715, 735639, 738553
    };

    if (u < 0) return -fp_sqrt(-u);

    if ((u & (127LL << FP_BASE)) == u) {
        // u is an integer between 0 and 127
        return SQUARE_ROOTS[u >> FP_BASE];
    }

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
