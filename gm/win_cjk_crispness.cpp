/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// DO NOT SUBMIT: experiment for https://github.com/flutter/flutter/issues/194931
// Renders 14px CJK text with DirectWrite in several rendering modes next to GDI ClearType

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypeface.h"
#include "tools/fonts/FontToolUtils.h"

#if defined(SK_BUILD_FOR_WIN)
#include "include/ports/SkTypeface_win.h"
#endif

namespace {

constexpr int kWidth = 1000;
constexpr int kLabelWidth = 300;
constexpr int kRowHeight = 46;
constexpr int kRows = 12;
constexpr int kHeight = kRows * kRowHeight + 10;

constexpr const char* kLine1 = "你好，世界 Hello World 0123456789";
constexpr const char* kLine2 =
        "在 RenderObject 树和 Layer 树的协作下，只有标记为 dirty 的节点才会参与重绘。";

sk_sp<SkTypeface> match_cjk(const sk_sp<SkFontMgr>& mgr, SkString* family) {
    if (!mgr) {
        return nullptr;
    }
    for (const char* name : {"Microsoft YaHei", "Microsoft YaHei UI", "SimSun", "Noto Sans SC"}) {
        if (sk_sp<SkTypeface> tf = mgr->matchFamilyStyle(name, SkFontStyle())) {
            *family = name;
            return tf;
        }
    }
    return nullptr;
}

struct Variant {
    const char* fName;
    bool fGdi;
    SkFont::Edging fEdging;
    SkFontHinting fHinting;
    bool fSubpixel;
};

// kNormal is the current DirectWrite behavior (Flutter's kSlight collapses to it on main);
// kSlight and kFull are remapped by this CL to GDI_NATURAL and GDI_CLASSIC
constexpr Variant kVariants[kRows] = {
    {"DW A8 current, subpx on", false, SkFont::Edging::kAntiAlias, SkFontHinting::kNormal, true},
    {"DW A8 current, subpx off", false, SkFont::Edging::kAntiAlias, SkFontHinting::kNormal, false},
    {"DW A8 no gridfit, subpx on", false, SkFont::Edging::kAntiAlias, SkFontHinting::kNone, true},
    {"DW A8 no gridfit, subpx off", false, SkFont::Edging::kAntiAlias, SkFontHinting::kNone, false},
    {"DW A8 GDI_NATURAL, subpx on", false, SkFont::Edging::kAntiAlias, SkFontHinting::kSlight, true},
    {"DW A8 GDI_NATURAL, subpx off", false, SkFont::Edging::kAntiAlias, SkFontHinting::kSlight,
     false},
    {"DW A8 GDI_CLASSIC, subpx on", false, SkFont::Edging::kAntiAlias, SkFontHinting::kFull, true},
    {"DW A8 GDI_CLASSIC, subpx off", false, SkFont::Edging::kAntiAlias, SkFontHinting::kFull,
     false},
    {"DW LCD current, subpx off", false, SkFont::Edging::kSubpixelAntiAlias,
     SkFontHinting::kNormal, false},
    {"DW LCD GDI_CLASSIC, subpx off", false, SkFont::Edging::kSubpixelAntiAlias,
     SkFontHinting::kFull, false},
    {"GDI A8, subpx off", true, SkFont::Edging::kAntiAlias, SkFontHinting::kNormal, false},
    {"GDI LCD (ClearType), subpx off", true, SkFont::Edging::kSubpixelAntiAlias,
     SkFontHinting::kNormal, false},
};

}  // namespace

DEF_SIMPLE_GM_BG(win_cjk_crispness, canvas, kWidth, kHeight, SK_ColorWHITE) {
    sk_sp<SkFontMgr> dwMgr;
    sk_sp<SkFontMgr> gdiMgr;
#if defined(SK_BUILD_FOR_WIN) && defined(SK_FONTMGR_DIRECTWRITE_AVAILABLE)
    dwMgr = SkFontMgr_New_DirectWrite();
#endif
#if defined(SK_BUILD_FOR_WIN) && defined(SK_FONTMGR_GDI_AVAILABLE)
    gdiMgr = SkFontMgr_New_GDI();
#endif
    SkString dwFamily("none");
    SkString gdiFamily("none");
    sk_sp<SkTypeface> dwFace = match_cjk(dwMgr, &dwFamily);
    sk_sp<SkTypeface> gdiFace = match_cjk(gdiMgr, &gdiFamily);

    // LCD text needs a surface with a known pixel geometry; the GM canvas has none
    SkSurfaceProps props(0, kRGB_H_SkPixelGeometry);
    sk_sp<SkSurface> surface =
            SkSurfaces::Raster(SkImageInfo::MakeN32Premul(kWidth, kHeight), &props);
    SkCanvas* c = surface->getCanvas();
    c->clear(SK_ColorWHITE);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    SkFont labelFont = ToolUtils::DefaultPortableFont();
    labelFont.setSize(11);

    for (int i = 0; i < kRows; ++i) {
        const Variant& v = kVariants[i];
        const SkScalar top = SkIntToScalar(i * kRowHeight);
        const sk_sp<SkTypeface>& face = v.fGdi ? gdiFace : dwFace;
        SkString label(v.fName);
        label.appendf(" [%s]", v.fGdi ? gdiFamily.c_str() : dwFamily.c_str());
        c->drawString(label.c_str(), 4, top + 18, labelFont, paint);
        if (!face) {
            continue;
        }
        SkFont font(face, 14);
        font.setEdging(v.fEdging);
        font.setHinting(v.fHinting);
        font.setSubpixel(v.fSubpixel);
        c->drawString(kLine1, kLabelWidth, top + 18, font, paint);
        c->drawString(kLine2, kLabelWidth, top + 38, font, paint);
    }

    canvas->drawImage(surface->makeImageSnapshot(), 0, 0);
}
