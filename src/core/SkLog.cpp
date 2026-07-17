/*
 * Copyright 2026 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkLog.h"

#include <cstdarg>


#if defined(__clang__)
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-format-attr-call"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
#endif

#if !defined(SkLog)


void SkLog(SkLogPriority priority, const char format[], ...) {
    va_list args;
    va_start(args, format);
    SkLogVAList(priority, format, args);
    va_end(args);
}
#endif
