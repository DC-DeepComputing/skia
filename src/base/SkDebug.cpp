/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkAttributes.h"
#include "include/private/base/SkDebug.h"

#if defined(SK_BUILD_FOR_GOOGLE3)
void SkDebugfForDumpStackTrace(const char* data, void* unused) {
    SkDebugf("%s", data);
}
#endif
