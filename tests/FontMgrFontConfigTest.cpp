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
#include "src/ports/SkFontConfigInterface_direct.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <fontconfig/fontconfig.h>

namespace {

bool bitmap_compare(const SkBitmap& ref, const SkBitmap& test) {
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
FcConfig* build_fontconfig_with_fontfile(const char* fontFilename) {
    FcConfig* config = FcConfigCreate();

    // FontConfig may modify the passed path (make absolute or other).
    FcConfigSetSysRoot(config, reinterpret_cast<const FcChar8*>(GetResourcePath("").c_str()));
    // FontConfig will lexically compare paths against its version of the sysroot.
    SkString fontFilePath(reinterpret_cast<const char*>(FcConfigGetSysRoot(config)));
    fontFilePath += fontFilename;
    FcConfigAppFontAddFile(config, reinterpret_cast<const FcChar8*>(fontFilePath.c_str()));

    FcConfigBuildFonts(config);
    return config;
}

bool fontmgr_understands_ft_named_instance_bits() {
    std::unique_ptr<SkStreamAsset> distortable(GetResourceAsStream("fonts/Distortable.ttf"));
    if (!distortable) {
        return false;
    }

    sk_sp<SkFontMgr> fm = SkFontMgr::RefDefault();
    SkFontArguments params;
    // The first named variation position in Distortable is 'Thin'.
    params.setCollectionIndex(0x00010000);
    sk_sp<SkTypeface> typeface = fm->makeFromStream(std::move(distortable), params);
    return !!typeface;
}

}  // namespace

DEF_TEST(FontMgrFontConfig_compareClone, reporter) {
    FcConfig* config = build_fontconfig_with_fontfile("/fonts/Distortable.ttf");

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

DEF_TEST(FontConfigInterface_MatchStyleNamedInstance, reporter) {
    if (!fontmgr_understands_ft_named_instance_bits()) {
        return;
    }

    FcConfig* config = build_fontconfig_with_fontfile("/fonts/NotoSansCJK-VF-subset.otf.ttc");
    sk_sp<SkFontConfigInterfaceDirect> fciDirect(new SkFontConfigInterfaceDirect(config));

    static constexpr const char* family_names[]{"Noto Sans CJK JP",
                                                "Noto Sans CJK HK",
                                                "Noto Sans CJK SC",
                                                "Noto Sans CJK TC",
                                                "Noto Sans CJK KR"};
    static constexpr const struct Test {
        int weight;
        bool highBitsExpectation;
    } tests[] {
        {100, false},
        {300, true },
        {350, true },
        {400, true },
        {500, true },
        {700, true },
        {900, true },
    };

    for (auto&& font_name : family_names) {
        for (auto&& [weight, highBitsExpectation] : tests) {
            SkFontStyle fontStyle(weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant);

            SkFontConfigInterface::FontIdentity resultIdentity;
            SkFontStyle resultStyle;
            SkString resultFamily;
            const bool r = fciDirect->matchFamilyName(
                    font_name, fontStyle, &resultIdentity, &resultFamily, &resultStyle);

            REPORTER_ASSERT(reporter, r, "Expecting to find a match result.");
            REPORTER_ASSERT(
                    reporter,
                    (resultIdentity.fTTCIndex >> 16 > 0) == highBitsExpectation,
                    "Expected to have the ttcIndex' upper 16 bits refer to a named instance.");

            // Intentionally go through manually creating the typeface so that SkFontStyle is
            // derived from data inside the font, not from the FcPattern that is the FontConfig
            // match result, see https://crbug.com/skia/12881
            sk_sp<SkTypeface> typeface(fciDirect->makeTypeface(resultIdentity).release());

            if (!typeface) {
                ERRORF(reporter, "Could not instantiate typeface, FcVersion: %d", FcGetVersion());
                return;
            }

            SkString family_from_typeface;
            typeface->getFamilyName(&family_from_typeface);

            REPORTER_ASSERT(reporter,
                            family_from_typeface == SkString(font_name),
                            "Matched font's family name should match the request.");

            SkFontStyle intrinsic_style = typeface->fontStyle();
            REPORTER_ASSERT(reporter,
                            intrinsic_style.weight() == weight,
                            "Matched font's weight should match request.");
            if (intrinsic_style.weight() != weight) {
                ERRORF(reporter,
                       "Matched font had weight: %d, expected %d, family: %s",
                       intrinsic_style.weight(),
                       weight,
                       family_from_typeface.c_str());
            }

            int numAxes = typeface->getVariationDesignPosition(nullptr, 0);
            std::vector<SkFontArguments::VariationPosition::Coordinate> coords;
            coords.resize(numAxes);
            typeface->getVariationDesignPosition(coords.data(), numAxes);

            REPORTER_ASSERT(reporter,
                            coords.size() == 1,
                            "The font must only have one axis, the weight axis.");

            REPORTER_ASSERT(reporter,
                            coords[0].axis == SkSetFourByteTag('w', 'g', 'h', 't'),
                            "The weight axis must be present and configured.");

            REPORTER_ASSERT(reporter,
                            static_cast<int>(coords[0].value) == weight,
                            "The weight axis must match the weight from the request.");
        }
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
    SkDebugf("Weight coordinate\n");
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
    FcConfig* config = build_fontconfig_with_fontfile("/fonts/Variable.ttf");
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

DEF_TEST(FontMgrFontConfig_matchFamilyStyle_namedInstance, reporter) {
    FcConfig* config = build_fontconfig_with_fontfile("/fonts/Variable.ttf");
    // Make the config the global current because that's what the
    // FontConfigInterface expects.
    FcConfigSetCurrent(config);
    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_FCI(SkFontConfigInterface::RefGlobal()));

    // Request one of the named instances in the variable font.
    SkFontStyle style(weight_style(100));
    sk_sp<SkTypeface> typeface(fontMgr->matchFamilyStyle("Variable", style));
    if (!typeface) {
        ERRORF(reporter, "Could not find typeface. FcVersion: %d", FcGetVersion());
        return;
    }

    // Thin is not bold.
    REPORTER_ASSERT(reporter, !typeface->isBold());
    // Ensure that the SkFontStyle is set correctly.
    REPORTER_ASSERT(reporter, typeface->fontStyle().weight() == 100);
    // Ensure that the variant axis is set correctly.
    REPORTER_ASSERT(reporter, weight_coordinate(typeface) == 100);
}

DEF_TEST(FontMgrFontConfig_matchFamilyStyle, reporter) {
    FcConfig* config = build_fontconfig_with_fontfile("/fonts/Variable.ttf");
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
    int weight = weight_coordinate(typeface);
    SkDebugf("Weight %d\n", weight);
    REPORTER_ASSERT(reporter, weight == 200);
}
