/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkFeatures.h"
#include "src/core/SkCpu.h"
#include "src/core/SkOptsTargets.h"
#include "src/core/SkSwizzlePriv.h"

#define SK_OPTS_TARGET SK_OPTS_TARGET_DEFAULT
#include "src/opts/SkOpts_SetTarget.h"

// Only define different default functions if they are not one of the pre-existing architecture
// specific implementations.
#if !defined(SK_CPU_X86) || \
    ((SK_CPU_SSE_LEVEL != SK_CPU_SSE_LEVEL_SSSE3) && (SK_CPU_SSE_LEVEL != SK_CPU_SSE_LEVEL_AVX2))
#include "src/opts/SkSwizzler_opts.h"  // IWYU pragma: keep
#endif

#include "src/opts/SkOpts_RestoreTarget.h"

namespace SkOpts {
    Swizzle_8888_u32 RGBA_to_BGRA = SK_OPTS_NS::RGBA_to_BGRA;
    Swizzle_8888_u32 RGBA_to_rgbA = SK_OPTS_NS::RGBA_to_rgbA;
    Swizzle_8888_u32 RGBA_to_bgrA = SK_OPTS_NS::RGBA_to_bgrA;
    Swizzle_8888_u32 rgbA_to_RGBA = SK_OPTS_NS::rgbA_to_RGBA;
    Swizzle_8888_u32 rgbA_to_BGRA = SK_OPTS_NS::rgbA_to_BGRA;
    Swizzle_8888_u32 inverted_CMYK_to_RGB1 = SK_OPTS_NS::inverted_CMYK_to_RGB1;
    Swizzle_8888_u32 inverted_CMYK_to_BGR1 = SK_OPTS_NS::inverted_CMYK_to_BGR1;
    Swizzle_8888_u8 RGB_to_RGB1 = SK_OPTS_NS::RGB_to_RGB1;
    Swizzle_8888_u8 RGB_to_BGR1 = SK_OPTS_NS::RGB_to_BGR1;
    Swizzle_8888_u8 gray_to_RGB1 = SK_OPTS_NS::gray_to_RGB1;
    Swizzle_8888_u8 grayA_to_RGBA = SK_OPTS_NS::grayA_to_RGBA;
    Swizzle_8888_u8 grayA_to_rgbA = SK_OPTS_NS::grayA_to_rgbA;

    void Init_Swizzler_ssse3();
    void Init_Swizzler_hsw();

    static bool init() {
    #if defined(SK_ENABLE_OPTIMIZE_SIZE)
        // All Init_foo functions are omitted when optimizing for size
    #elif defined(SK_CPU_X86)
        #if SK_CPU_SSE_LEVEL < SK_CPU_SSE_LEVEL_SSSE3
            if (SkCpu::Supports(SkCpu::SSSE3)) { Init_Swizzler_ssse3(); }
        #endif

        #if SK_CPU_SSE_LEVEL < SK_CPU_SSE_LEVEL_AVX2
            if (SkCpu::Supports(SkCpu::HSW)) { Init_Swizzler_hsw(); }
        #endif
    #endif
      return true;
    }

    void Init_Swizzler() {
        [[maybe_unused]] static bool gInitialized = init();
    }
}  // namespace SkOpts
