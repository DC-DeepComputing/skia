/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_multi_DEFINED
#define SkFontMgr_multi_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"

class SkFontMgr;

/** Create a font manager which searches each fontmgr in turn. */
SK_API sk_sp<SkFontMgr> SkFontMgr_New_Multi(SkSpan<const sk_sp<SkFontMgr>> fontmgrs);

#endif // SkFontMgr_multi_DEFINED
