/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkFontMgr_android.h"

#if defined(SK_BUILD_FOR_ANDROID) && defined(SK_FONTMGR_ANDROID_AVAILABLE) && \
        defined(SK_TYPEFACE_FACTORY_FREETYPE)
#include "include/ports/SkFontScanner_FreeType.h"

class FontMgrAndroidLazyBench : public Benchmark {
    bool fLazy;
    SkUnichar fChar;
    const char* fLocale;
    SkFontStyle fStyle;
    SkString fName;

public:
    FontMgrAndroidLazyBench(
            bool lazy, SkUnichar c, const char* locale, SkFontStyle style, const char* name)
            : fLazy(lazy), fChar(c), fLocale(locale), fStyle(style) {
        fName.printf("fontmgr_android_%s_%s_%s",
                     lazy ? "lazy" : "eager",
                     locale ? locale : "untagged",
                     name);
    }

    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }

protected:
    const char* onGetName() override { return fName.c_str(); }

    // Cold latency: manager construction plus the first fallback match, as on app start
    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            auto loading = fLazy ? SkFontMgr_Android_FontLoading::kLazy
                                 : SkFontMgr_Android_FontLoading::kEager;
            sk_sp<SkFontMgr> mgr =
                    SkFontMgr_New_Android(nullptr, SkFontScanner_Make_FreeType(), loading);
            sk_sp<SkTypeface> tf =
                    mgr->matchFamilyStyleCharacter(nullptr, fStyle, &fLocale, 1, fChar);
        }
    }
};

// CJK
DEF_BENCH(return new FontMgrAndroidLazyBench(true, 0x4E2D, "zh-Hans", SkFontStyle(), "cjk_normal");)
DEF_BENCH(
        return new FontMgrAndroidLazyBench(false, 0x4E2D, "zh-Hans", SkFontStyle(), "cjk_normal");)
DEF_BENCH(return new FontMgrAndroidLazyBench(
                         true, 0x4E2D, "zh-Hans", SkFontStyle::Bold(), "cjk_bold");)
DEF_BENCH(return new FontMgrAndroidLazyBench(
                         false, 0x4E2D, "zh-Hans", SkFontStyle::Bold(), "cjk_bold");)

// CJK untagged
DEF_BENCH(return new FontMgrAndroidLazyBench(
                         true, 0x4E2D, nullptr, SkFontStyle(), "cjk_untagged_normal");)
DEF_BENCH(return new FontMgrAndroidLazyBench(
                         false, 0x4E2D, nullptr, SkFontStyle(), "cjk_untagged_normal");)

// Emoji
DEF_BENCH(return new FontMgrAndroidLazyBench(true, 0x1F600, nullptr, SkFontStyle(), "emoji");)
DEF_BENCH(return new FontMgrAndroidLazyBench(false, 0x1F600, nullptr, SkFontStyle(), "emoji");)

#endif
