/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_ANDROID)

#include "include/core/SkFontMgr.h"
#include "include/ports/SkFontMgr_android_ndk.h"

sk_sp<SkFontMgr> SkFontMgr::Factory() {
    return SkFontMgr_New_Android_NDK(false);
}

#endif//defined(SK_BUILD_FOR_ANDROID)
