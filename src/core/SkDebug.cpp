/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkDebug.h"

#include <cstdarg>

#include "include/private/SkAssert.h" // IWYU pragma: keep
#include "include/private/SkAttributes.h" // IWYU pragma: keep
#include "include/private/SkLog.h"

#ifdef UNSAFE_BUFFERS_BUILD
// TODO(crbug.com/40285824): Remove this and convert code to safer constructs.
#pragma allow_unsafe_buffers
#endif

#if defined(SK_BUILD_FOR_GOOGLE3)
void SkDebugfForDumpStackTrace(const char* data, void* unused) {
    SkDebugf("%s", data);
}
#endif

#if !defined(SkDebugf)
void SkDebugf(const char format[], ...) {
    va_list args;
    va_start(args, format);
    SkLogVAList(SkLogPriority::kDebug, format, args);
    va_end(args);
}
#endif
