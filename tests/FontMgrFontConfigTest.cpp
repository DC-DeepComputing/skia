/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkFontConfigInterface.h"
#include "include/ports/SkFontMgr_FontConfigInterface.h"
#include "include/ports/SkFontMgr_fontconfig.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <fontconfig/fontconfig.h>

static bool bitmap_compare(const SkBitmap& ref, const SkBitmap& test) {
    for (int y = 0; y < test.height(); ++y) {
        for (int x = 0; x < test.width(); ++x) {
            SkColor testColor = test.getColor(x, y);
            SkColor refColor = ref.getColor(x, y);
            if (refColor != testColor) {
                return false;
            }
        }
    }
    return true;
}

// Builds a FcConfig with just the specified font at |path| in the list of
// fonts.
static FcConfig* load_resource_font(const char resourcePath[], const char path[]) {
    FcConfig* config = FcConfigCreate();

    // FontConfig may modify the passed path (make absolute or other).
    FcConfigSetSysRoot(config, reinterpret_cast<const FcChar8*>(resourcePath));
    // FontConfig will lexically compare paths against its version of the sysroot.
    SkString resourceFontPath(reinterpret_cast<const char*>(FcConfigGetSysRoot(config)));
    resourceFontPath += path;
    FcConfigAppFontAddFile(config, reinterpret_cast<const FcChar8*>(resourceFontPath.c_str()));

    FcConfigBuildFonts(config);
    return config;
}

DEF_TEST(FontMgrFontConfig_compareClone, reporter) {
    FcConfig* config = load_resource_font(GetResourcePath("").c_str(), "/fonts/Distortable.ttf");

    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_FontConfig(config));
    sk_sp<SkTypeface> typeface(fontMgr->legacyMakeTypeface("Distortable", SkFontStyle()));
    if (!typeface) {
        ERRORF(reporter, "Could not find typeface. FcVersion: %d", FcGetVersion());
        return;
    }

    SkBitmap bitmapStream;
    bitmapStream.allocN32Pixels(64, 64);
    SkCanvas canvasStream(bitmapStream);
    canvasStream.drawColor(SK_ColorWHITE);

    SkBitmap bitmapClone;
    bitmapClone.allocN32Pixels(64, 64);
    SkCanvas canvasClone(bitmapClone);
    canvasStream.drawColor(SK_ColorWHITE);

    SkPaint paint;
    paint.setColor(SK_ColorGRAY);

    constexpr float kTextSize = 20;

    std::unique_ptr<SkStreamAsset> distortableStream(GetResourceAsStream("fonts/Distortable.ttf"));
    if (!distortableStream) {
        return;
    }

    SkPoint point = SkPoint::Make(20.0f, 20.0f);
    SkFourByteTag tag = SkSetFourByteTag('w', 'g', 'h', 't');

    for (int i = 0; i < 10; ++i) {
        SkScalar styleValue = SkDoubleToScalar(0.5 + i * ((2.0 - 0.5) / 10));
        SkFontArguments::VariationPosition::Coordinate coordinates[] = {{tag, styleValue}};
        SkFontArguments::VariationPosition position = {coordinates, SK_ARRAY_COUNT(coordinates)};

        SkFont fontStream(
                fontMgr->makeFromStream(distortableStream->duplicate(),
                                        SkFontArguments().setVariationDesignPosition(position)),
                kTextSize);
        fontStream.setEdging(SkFont::Edging::kSubpixelAntiAlias);

        SkFont fontClone(
                typeface->makeClone(SkFontArguments().setVariationDesignPosition(position)),
                kTextSize);
        fontClone.setEdging(SkFont::Edging::kSubpixelAntiAlias);

        constexpr char text[] = "abc";

        canvasStream.drawColor(SK_ColorWHITE);
        canvasStream.drawString(text, point.fX, point.fY, fontStream, paint);

        canvasClone.drawColor(SK_ColorWHITE);
        canvasClone.drawString(text, point.fX, point.fY, fontClone, paint);

        bool success = bitmap_compare(bitmapStream, bitmapClone);
        REPORTER_ASSERT(reporter, success);
    }
}

// Returns a SkFontStyle with weight specified and an otherwise normal style.
static SkFontStyle weight_style(int weight) {
    return SkFontStyle(
            weight, SkFontStyle::Width::kNormal_Width, SkFontStyle::Slant::kUpright_Slant);
}

// Returns the value of the wght axes on the typeface if it can be found.
// Otherwise returns -1.
static int weight_coordinate(sk_sp<SkTypeface> typeface) {
    int axes = typeface->getVariationDesignPosition(nullptr, 0);
    if (axes <= 0) {
        // Typeface is not variable.
        return -1;
    }

    const SkFourByteTag wght = SkSetFourByteTag('w', 'g', 'h', 't');
    std::vector<SkFontArguments::VariationPosition::Coordinate> coordinates(axes);
    typeface->getVariationDesignPosition(coordinates.data(), axes);
    for (const SkFontArguments::VariationPosition::Coordinate& coord : coordinates) {
        if (coord.axis == wght) {
            return SkFloatToScalar(coord.value);
        }
    }

    return -1;
}

DEF_TEST(FontMgrFontConfig_legacyMakeTypeface, reporter) {
    SkDebugf("Start test\n");
    FcConfig* config = load_resource_font(GetResourcePath("").c_str(), "/fonts/Variable.ttf");
    // Make the config the global current because that's what the
    // FontConfigInterface expects.
    FcConfigSetCurrent(config);
    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_FCI(SkFontConfigInterface::RefGlobal()));

    // Request "Bold" weight.  Fontconfig equivalent 200. It's important that
    // this is not a named instance in the font.
    SkFontStyle style(weight_style(700));
    sk_sp<SkTypeface> typeface(fontMgr->legacyMakeTypeface("Variable", style));
    if (!typeface) {
        ERRORF(reporter, "Could not find typeface. FcVersion: %d", FcGetVersion());
        return;
    }

    // Ensure that variable typefaces still report bold.
    REPORTER_ASSERT(reporter, typeface->isBold());
    // Ensure that the SkFontStyle is set correctly.
    REPORTER_ASSERT(reporter, typeface->fontStyle().weight() == 700);
    // Ensure that the variant axis is set correctly.
    REPORTER_ASSERT(reporter, weight_coordinate(typeface) == 700);
}

DEF_TEST(FontMgrFontConfig_matchFamilyStyle, reporter) {
    FcConfig* config = load_resource_font(GetResourcePath("").c_str(), "/fonts/Variable.ttf");
    // Make the config the global current because that's what the
    // FontConfigInterface expects.
    FcConfigSetCurrent(config);
    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_FCI(SkFontConfigInterface::RefGlobal()));

    // Request "Extra Light" weight.  Fontconfig equivalent 40. It's important that
    // this is not a named instance in the font.
    SkFontStyle style(weight_style(200));
    sk_sp<SkTypeface> typeface(fontMgr->matchFamilyStyle("Variable", style));
    if (!typeface) {
        ERRORF(reporter, "Could not find typeface. FcVersion: %d", FcGetVersion());
        return;
    }

    // Thin is not bold.
    REPORTER_ASSERT(reporter, !typeface->isBold());
    // Ensure that the SkFontStyle is set correctly.
    REPORTER_ASSERT(reporter, typeface->fontStyle().weight() == 200);
    // Ensure that the variant axis is set correctly.
    REPORTER_ASSERT(reporter, weight_coordinate(typeface) == 200);
}
