/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkContextOptions_DEFINED
#define SkContextOptions_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkAPI.h"

#include <cstddef>

struct SK_API SkContextOptions {
    SkContextOptions() = default;

    // Historical default value
    int fTypefaceCacheCountLimit = 1024;
    size_t fResourceCacheTotalByteLimit = 32 * 1024 * 1024;
    // 0 means no separate single-allocation cap beyond the total resource cache budget
    size_t fResourceCacheSingleAllocationByteLimit = 0;

    //  SK_DEFAULT_FONT_CACHE_COUNT_LIMIT and SK_DEFAULT_FONT_CACHE_LIMIT can be set using -D on
    //  your compiler commandline, or by using the defines in SkUserConfig.h
    int fFontCacheCountLimit = 2048;
    size_t fFontCacheLimit = 2 * 1024 * 1024;
};

#endif  // SkContextOptions_DEFINED
