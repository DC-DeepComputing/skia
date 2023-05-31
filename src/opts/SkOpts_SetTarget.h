/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Include guards are intentionally omitted

#if !defined(SK_OPTS_TARGET)
#error Define SK_OPTS_TARGET before including SkOpts_SetTarget
#endif

#if defined(SK_OLD_CPU_SSE_LEVEL)
#error Include SkOpts_RestoreTarget before re-including SkOpts_SetTarget
#endif

#define SK_OLD_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL
#undef SK_CPU_SSE_LEVEL

#if SK_OPTS_TARGET == SK_OPTS_TARGET_SSSE3
    #define SK_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL_SSSE3
    #define SK_OPTS_TARGET_STR "ssse3"

#elif SK_OPTS_TARGET == SK_OPTS_TARGET_AVX
    #define SK_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL_AVX
    #define SK_OPTS_TARGET_STR "avx"

#elif SK_OPTS_TARGET == SK_OPTS_TARGET_HSW
    #define SK_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL_AVX2
    #define SK_OPTS_TARGET_STR "arch=haswell"

#elif SK_OPTS_TARGET == SK_OPTS_TARGET_SKX
    #define SK_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL_SKX
    #define SK_OPTS_TARGET_STR "arch=skylake-avx512"

#else
    #error Unexpected value of SK_OPTS_TARGET

#endif

#if defined(__clang__)
    #pragma clang attribute push(__attribute__((target(SK_OPTS_TARGET_STR))), apply_to=function)
#elif defined(__GNUC__)
    #pragma GCC push_options
    #pragma GCC target(SK_OPTS_TARGET_STR)
#endif

#undef SK_OPTS_TARGET_STR
