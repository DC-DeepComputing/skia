/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Swizzle_opts_DEFINED
#define Swizzle_opts_DEFINED

#include <cstddef>
#include <cstdint>

namespace SkOpts {
    // Swizzle input into some sort of 8888 pixel, {premul,unpremul} x {rgba,bgra}.
    typedef void (*Swizzle_8888_u32)(uint32_t*, const uint32_t*, int);
    extern Swizzle_8888_u32 RGBA_to_BGRA,          // i.e. just swap RB
                            RGBA_to_rgbA,          // i.e. just premultiply
                            RGBA_to_bgrA,          // i.e. swap RB and premultiply
                            inverted_CMYK_to_RGB1, // i.e. convert color space
                            inverted_CMYK_to_BGR1; // i.e. convert color space

    typedef void (*Swizzle_8888_u8)(uint32_t*, const uint8_t*, int);
    extern Swizzle_8888_u8 RGB_to_RGB1,     // i.e. insert an opaque alpha
                           RGB_to_BGR1,     // i.e. swap RB and insert an opaque alpha
                           gray_to_RGB1,    // i.e. expand to color channels + an opaque alpha
                           grayA_to_RGBA,   // i.e. expand to color channels
                           grayA_to_rgbA;   // i.e. expand to color channels and premultiply
}

#endif
