/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdint>

template <int N, typename T> using V = T __attribute__((ext_vector_type(N)));

#if !defined(__clang__)
static_assert(false, "This only works on clang.");
#endif

#if defined(__SSSE3__)
#include <immintrin.h>
#endif

#if defined(__ARM_NEON)
// From section 5.5.5 of the ARM C Language Extensions (ACLE)
    #include <arm_neon.h>
#endif

using Q15 = V<8, uint16_t>;

// A pure C version of the neon intrinsic vqrdmulhq_s16;
static Q15 simulate_neon_vqrdmulhq_s16(Q15 a, Q15 b) {
    Q15 result;
    const int esize = 16;
    auto m = [](int16_t r, int16_t s) {
        const int64_t rounding = 1 << (esize - 1);
        int64_t product = 2LL * (int64_t)r * (int64_t)s + rounding;
        int64_t result = product >> esize;

        // Saturate the result
        if (int64_t limit =  (1LL << (esize - 1)) - 1; result > limit) { result = limit; }
        if (int64_t limit = -(1LL << (esize - 1))    ; result < limit) { result = limit; }
        return result;
    };
    for (int i = 0; i < 8; i++) {
        result[i] = m(a[i], b[i]);
    }
    return result;
}

struct Stats {
    int diff_16_bits[4] = {0, 0, 0, 0};
    int diff_8_bits = 0;
    int64_t total = 0;
};

static int16_t saturating_lerp(int16_t t, int16_t a, int16_t b) {
    Q15 qt(t);
    Q15 qa(a);
    Q15 qb(b);

    Q15 answer = simulate_neon_vqrdmulhq_s16(qt, qb - qa) + qa;
    return answer[0];
}

template <int logPixelScale, typename Lerp>
static Stats check_full_precision(Lerp lerp) {
    Stats stats;
    for (int t = 32767; t >= 0; t--)
    for (int a = 255; a >= 0; a--)
    for (int b = 255; b >= 0; b--) {
        int16_t scaledA = a << logPixelScale,
                scaledB = b << logPixelScale;
        int16_t toCheck = lerp(t, scaledA, scaledB);
        int scaledAnswer = (int)floor((t/32768.0f)*(scaledB - scaledA) + scaledA + 0.5f);
        int16_t half = 1 << (logPixelScale - 1);
        int16_t answer = (int16_t)floor((scaledAnswer + half) / (1 << logPixelScale));
        switch (abs(toCheck - scaledAnswer)) {
            case 0: break;
            case 1: stats.diff_16_bits[0]++; break;
            case 2: stats.diff_16_bits[1]++; break;
            case 3: stats.diff_16_bits[2]++; break;
            default: stats.diff_16_bits[3]++; break;
        }
        stats.diff_8_bits += ((toCheck + half) >> logPixelScale) != answer;
        stats.total++;
    }
    return stats;
}

int main() {
    Stats stats = check_full_precision<6>(saturating_lerp);
    if (stats.diff_16_bits != 0 || stats.diff_8_bits != 0) {
        printf("16-bit diffs: %d percent: %g 8-bit diffs: %d percent: %g\n",
               stats.diff_16_bits, (float)stats.diff_16_bits / stats.total,
               stats.diff_8_bits, (float)stats.diff_8_bits / stats.total );
        printf("total: %ld\n", stats.total);
    }
    printf("Done.");
    return 0;
}

