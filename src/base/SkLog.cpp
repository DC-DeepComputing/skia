/*
 * Copyright 2026 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/private/base/SkLog.h"

#include <cstdarg>

#include "include/private/base/SkAssert.h" // IWYU pragma: keep
#include "include/private/base/SkAttributes.h" // IWYU pragma: keep

void SkLog(SkLogPriority priority, const char format[], ...) {
    va_list args;
    va_start(args, format);
    vSkLog(priority, format, args);
    va_end(args);
}
