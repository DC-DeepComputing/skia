/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/ports/SkFontMgr_empty.h"
#include "src/core/SkFontScanner.h"
#include "src/ports/SkFontMgr_custom.h"
#include "src/ports/SkEmptyFontLoader.h"

sk_sp<SkFontMgr> SkFontMgr_New_Custom_Empty_Fontations() {
    return sk_make_sp<SkFontMgr_Custom>(EmptyFontLoader());
}
