/*
 Copyright (C) 1997-2001 Id Software, Inc.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 */

// MT19937 PRNG
#include "q_shared.h"

#define N 624
#define M 397

static uint32 mt_state[N];
static uint32 mt_index;

/**
 * Seed PRNG with initial value
 */
void init_genrand(uint32 seed) {
    int i;

    mt_index = N;
    mt_state[0] = seed;
    for (i = 1; i < N; i++) {
        mt_state[i] = seed = 1812433253 * (seed ^ seed >> 30) + i;
    }
}

/**
 * Generate random integer in range [0, 2^32)
 */
uint32 genrand_int32(void) {
    uint32 x, y;
    int i;

    if (mt_index >= N) {
        mt_index = 0;

#define STEP(j, k) do {                 \
        x  = mt_state[i] & 0x80000000;  \
        x |= mt_state[j] & 0x7FFFFFFF;  \
        y  = x >> 1;                    \
        y ^= 0x9908B0DF & -(x & 1);     \
        mt_state[i] = mt_state[k] ^ y;  \
    } while (0)

        for (i = 0; i < N - M; i++) {
            STEP(i + 1, i + M);
        }
        for (; i < N - 1; i++) {
            STEP(i + 1, i - N + M);
        }
        STEP(0, M - 1);
    }

    y = mt_state[mt_index++];
    y ^= y >> 11;
    y ^= y << 7 & 0x9D2C5680;
    y ^= y << 15 & 0xEFC60000;
    y ^= y >> 18;

    return y;
}

/**
 * Generate random integer in range [0, n) avoiding modulo bias
 */
uint32 genrand_uniform(uint32 n) {
    uint32 r, m;

    if (n < 2) {
        return 0;
    }
    m = -n % n; // m = 2^32 mod n
    do {
        r = genrand_int32();
    } while (r < m);

    return r % n;
}
