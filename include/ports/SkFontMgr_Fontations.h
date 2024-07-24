/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_fontations_DEFINED
#define SkFontMgr_fontations_DEFINED

#include "SkFontMgr_android.h"
#include "include/core/SkRefCnt.h"

class SkFontMgr;

/** Create a font manager instantiating fonts using the Rust Fontations backend.
 * This font manager does not support matching fonts, only instantiation.
 */
SK_API sk_sp<SkFontMgr> SkFontMgr_New_Fontations_Empty();
SK_API sk_sp<SkFontMgr> SkFontMgr_New_Fontations(const SkFontMgr_Android_CustomFonts* custom);
#endif // #ifndef SkFontMgr_fontations_DEFINED
