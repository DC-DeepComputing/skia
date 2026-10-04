/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_compute_SharedBlurSksl_DEFINED
#define skgpu_graphite_compute_SharedBlurSksl_DEFINED

#include "include/core/SkTileMode.h"

namespace skgpu::graphite {

// The SkSL below hard-codes these.
static_assert(static_cast<int>(SkTileMode::kClamp) == 0);
static_assert(static_cast<int>(SkTileMode::kRepeat) == 1);
static_assert(static_cast<int>(SkTileMode::kMirror) == 2);
static_assert(static_cast<int>(SkTileMode::kDecal) == 3);

constexpr char kSharedBlurSkSL[] = R"(
    const int CLAMP = 0;
    const int REPEAT = 1;
    const int MIRROR = 2;
    const int DECAL = 3;

    //Returns a modulo n (which must be positive) in [0, n). The result of % with negative
    //operands differs between backends, so a is made non-negative first
    int PositiveMod(int a, int n)
    {
        if (a < 0)
            a += n * ((n - 1 - a) / n);
        return a % n;
    }

    //Returns the source coordinate that p maps to along an axis of [lo, hi) with the tile
    //mode, or -1 if it maps to transparent black
    int TileCoord(int p, int lo, int hi, int mode)
    {
        if (p >= lo && p < hi)
            return p;
        if (mode == DECAL)
            return -1;
        if (mode == CLAMP)
            return p < lo ? lo : hi - 1;
        //One period (repeat) or reflection (mirror) maps every p within n of the edges, which
        //saves the integer division of the modulo below
        int n = hi - lo;
        int q;
        if (mode == REPEAT)
            q = p < lo ? p + n : p - n;
        else
            q = p < lo ? 2 * lo - 1 - p : 2 * hi - 1 - p;
        if (q >= lo && q < hi)
            return q;
        if (mode == REPEAT)
            return lo + PositiveMod(p - lo, n);
        int m = PositiveMod(p - lo, 2 * n);
        return lo + (m < n ? m : 2 * n - 1 - m);
    }

    uint2 PackHalf4(half4 v)
    {
        return uint2(packHalf2x16(v.xy), packHalf2x16(v.zw));
    }

    half4 UnpackHalf4(uint2 v)
    {
        return half4(unpackHalf2x16(v.x), unpackHalf2x16(v.y));
    }
)";

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_compute_SharedBlurSksl_DEFINED
