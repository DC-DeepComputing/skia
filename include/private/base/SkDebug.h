/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDebug_DEFINED
#define SkDebug_DEFINED

#include <cstdarg>
#include "include/private/base/SkAPI.h"
#include "include/private/base/SkAttributes.h"
#include "include/private/base/SkLoadUserConfig.h" // IWYU pragma: keep
#include "include/private/base/SkLogPriority.h"

#if !defined(SkDebugf)
    void vSkDebugf(SkLogPriority priority, const char format[], va_list args) SK_PRINTF_LIKE(2, 0);
    void SK_SPI SkDebugf(SkLogPriority priority, const char format[], ...) SK_PRINTF_LIKE(2, 3);
    // This legacy function should be removed once all SkDebug call sites are replaced with the
    // new SKIA_LOG_PRIORITY macros.
    void SK_SPI SkDebugf(const char format[], ...)SK_PRINTF_LIKE(1, 2);
#endif

#if defined(SK_DEBUG)
    #define SkDEBUGCODE(...)  __VA_ARGS__
    #define SkDEBUGF(...)     SkDebugf(SkLogPriority::kDebug, __VA_ARGS__)
#else
    #define SkDEBUGCODE(...)
    #define SkDEBUGF(...)
#endif

#endif
