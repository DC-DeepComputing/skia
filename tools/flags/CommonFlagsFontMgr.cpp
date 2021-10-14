/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkFontMgrPriv.h"
#include "tools/flags/CommonFlagsFontMgr.h"
#include "tools/fonts/TestFontMgr.h"

#if defined(SK_BUILD_FOR_WIN)
#include "include/ports/SkTypeface_win.h"
#endif

namespace CommonFlags {

/**
 *  Turn on portable (--nonativeFonts) or GDI font rendering (--gdi).
 */
static DEFINE_bool(nativeFonts,
                   true,
                   "If true, use native font manager and rendering. "
                   "If false, fonts will draw as portably as possible.");
#if defined(SK_BUILD_FOR_WIN)
static DEFINE_bool(gdi, false, "Use GDI instead of DirectWrite for font rendering.");
#endif

void SetDefaultFontMgr() {
    if (!FLAGS_nativeFonts) {
        gSkFontMgr_DefaultFactory = &ToolUtils::MakePortableFontMgr;
    }
#if defined(SK_BUILD_FOR_WIN)
    if (FLAGS_gdi) {
        gSkFontMgr_DefaultFactory = &SkFontMgr_New_GDI;
    }
#endif
}

}
