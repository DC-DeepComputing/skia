/*
 * Copyright 2026 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkLog.h"

#include <cstdarg>

#if defined(UNSAFE_BUFFERS_BUILD) && defined(__clang__)
// TODO(crbug.com/40285824): Remove this and convert code to safer constructs.
#pragma allow_unsafe_buffers
#endif

#if !defined(SkLog)
void SkLog(SkLogPriority priority, const char format[], ...) {
    va_list args;
    va_start(args, format);
    SkLogVAList(priority, format, args);
    va_end(args);
}
#endif
