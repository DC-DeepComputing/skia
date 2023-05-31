/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"

#if defined(SK_CPU_X86) && !defined(SK_ENABLE_OPTIMIZE_SIZE)

//
// Switch to generating SSSE3 code
//
#define SK_OLD_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL
#undef SK_CPU_SSE_LEVEL
#define SK_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL_SSSE3

#if defined(__clang__)
    #pragma clang attribute push(__attribute__((target("ssse3"))), apply_to=function)
#elif defined(__GNUC__)
    #pragma GCC push_options
    #pragma GCC target("ssse3")
#endif

#define SK_OPTS_NS ssse3
#include "src/opts/SkBitmapProcState_opts.h"
#include "src/opts/SkBlitMask_opts.h"
#include "src/opts/SkSwizzler_opts.h"

//
// Restore compilation target
//
#undef SK_CPU_SSE_LEVEL
#define SK_CPU_SSE_LEVEL SK_OLD_CPU_SSE_LEVEL
#undef SK_OLD_CPU_SSE_LEVEL

#if defined(__clang__)
    #pragma clang attribute pop
#elif defined(__GNUC__)
    #pragma GCC pop_options
#endif

namespace SkOpts {
    void Init_ssse3() {
        blit_mask_d32_a8 = ssse3::blit_mask_d32_a8;

        RGBA_to_BGRA          = ssse3::RGBA_to_BGRA;
        RGBA_to_rgbA          = ssse3::RGBA_to_rgbA;
        RGBA_to_bgrA          = ssse3::RGBA_to_bgrA;
        RGB_to_RGB1           = ssse3::RGB_to_RGB1;
        RGB_to_BGR1           = ssse3::RGB_to_BGR1;
        gray_to_RGB1          = ssse3::gray_to_RGB1;
        grayA_to_RGBA         = ssse3::grayA_to_RGBA;
        grayA_to_rgbA         = ssse3::grayA_to_rgbA;
        inverted_CMYK_to_RGB1 = ssse3::inverted_CMYK_to_RGB1;
        inverted_CMYK_to_BGR1 = ssse3::inverted_CMYK_to_BGR1;

        S32_alpha_D32_filter_DX  = ssse3::S32_alpha_D32_filter_DX;
    }
}  // namespace SkOpts

#endif // SK_CPU_X86 && !SK_ENABLE_OPTIMIZE_SIZE
