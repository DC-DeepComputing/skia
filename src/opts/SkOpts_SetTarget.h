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

// Why not put the target string in a #define, and remove the boilerplate? Because GCC doesn't
// allow the target() option to be expanded by the preprocessor - it must be a literal string.
#if SK_OPTS_TARGET == SK_OPTS_TARGET_SSSE3
    #define SK_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL_SSSE3
    #if defined(__clang__)
        #pragma clang attribute push(__attribute__((target("ssse3"))), apply_to=function)
    #elif defined(__GNUC__)
        #pragma GCC push_options
        #pragma GCC target("ssse3")
    #endif

#elif SK_OPTS_TARGET == SK_OPTS_TARGET_AVX
    #define SK_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL_AVX
    #if defined(__clang__)
        #pragma clang attribute push(__attribute__((target("avx"))), apply_to=function)
    #elif defined(__GNUC__)
        #pragma GCC push_options
        #pragma GCC target("avx")
    #endif

#elif SK_OPTS_TARGET == SK_OPTS_TARGET_HSW
    #define SK_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL_AVX2
    #if defined(__clang__)
        #pragma clang attribute push(__attribute__((target("arch=haswell"))), apply_to=function)
    #elif defined(__GNUC__)
        #pragma GCC push_options
        #pragma GCC target("arch=haswell")
    #endif

#elif SK_OPTS_TARGET == SK_OPTS_TARGET_SKX
    #define SK_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL_SKX
    #if defined(__clang__)
        #pragma clang attribute push(__attribute__((target("arch=skylake-avx512"))), apply_to=function)
    #elif defined(__GNUC__)
        #pragma GCC push_options
        #pragma GCC target("arch=skylake-avx512")
    #endif

#else
    #error Unexpected value of SK_OPTS_TARGET

#endif
