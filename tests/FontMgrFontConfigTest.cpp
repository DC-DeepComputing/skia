/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/encode/SkPngEncoder.h"
#include "include/ports/SkFontMgr_fontconfig.h"

#ifdef SK_TYPEFACE_FACTORY_FONTATIONS
#include "include/ports/SkFontScanner_Fontations.h"
#endif
#ifdef SK_TYPEFACE_FACTORY_FREETYPE
#include "include/ports/SkFontScanner_FreeType.h"
#endif
#include "tests/Test.h"
#include "tools/Resources.h"

#include <fontconfig/fontconfig.h>

#include <array>
#include <memory>

namespace {

bool bitmap_compare(const SkBitmap& ref, const SkBitmap& test) {
    auto count = 0;
    for (int y = 0; y < test.height(); ++y) {
        for (int x = 0; x < test.width(); ++x) {
            SkColor testColor = test.getColor(x, y);
            SkColor refColor = ref.getColor(x, y);
            if (refColor != testColor) {
                ++count;
                if ((false)) {
                    SkDebugf("%d: (%d,%d) ", count, x, y);
                }
            }
        }
    }
    return (count == 0);
}

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

FcConfig* build_fontconfig_from_resources() {

    SkString path = GetResourcePath("");
    SkString content;
    content.append("<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                   "<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">"
                   "<fontconfig>\n"
                   "<dir>/fonts</dir>\n");
    content.append("<match target=\"font\">\n"
                   "   <edit name=\"embolden\" mode=\"assign\">\n"
                   "       <bool>true</bool>\n"
                   "   </edit>\n"
                   "</match>");
    content.append("</fontconfig>\n");
    FcConfig* fc_config = FcConfigCreate();
    FcConfigSetSysRoot(fc_config, reinterpret_cast<const FcChar8*>(path.c_str()));
    if (FcConfigParseAndLoadFromMemory(
                fc_config, reinterpret_cast<const FcChar8*>(content.c_str()),
                FcTrue) != FcTrue) {
        SkDebugf("FcConfigParseAndLoadFromMemory\n");
    }
    if (FcConfigBuildFonts(fc_config) != FcTrue) {
        SkDebugf("!FcConfigBuildFonts\n");
    }
    return fc_config;
}

[[maybe_unused]]
static void write_bitmap(const SkBitmap* bm, const char fileName[]) {
    SkFILEWStream file(fileName);
    SkAssertResult(file.isValid());
    SkAssertResult(SkPngEncoder::Encode(&file, bm->pixmap(), {}));
}

}  // namespace

DEF_TEST(FontMgrFontConfig, reporter) {
    FcConfig* config = build_fontconfig_with_fontfile("/fonts/Distortable.ttf");

    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_FontConfig(config, SkFontScanner_Make_FreeType()));
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

    std::unique_ptr<SkStreamAsset> distortableStream(
            GetResourceAsStream("fonts/Distortable.ttf"));
    if (!distortableStream) {
        return;
    }

    SkPoint point = SkPoint::Make(20.0f, 20.0f);
    SkFourByteTag tag = SkSetFourByteTag('w', 'g', 'h', 't');

    for (int i = 0; i < 10; ++i) {
        SkScalar styleValue =
            SkDoubleToScalar(0.5 + i * ((2.0 - 0.5) / 10));
        SkFontArguments::VariationPosition::Coordinate
            coordinates[] = {{tag, styleValue}};
        SkFontArguments::VariationPosition
            position = {coordinates, std::size(coordinates)};

        SkFont fontStream(
            fontMgr->makeFromStream(distortableStream->duplicate(),
                                    SkFontArguments().setVariationDesignPosition(position)),
            kTextSize);
        fontStream.setEdging(SkFont::Edging::kSubpixelAntiAlias);

        SkFont fontClone(
            typeface->makeClone(SkFontArguments().setVariationDesignPosition(position)), kTextSize);
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

void testAllBold(sk_sp<SkFontMgr> fontMgr, skiatest::Reporter* reporter) {
    constexpr float kTextSize = 20;
    constexpr char text[] = "abc";

    SkString filePath = GetResourcePath("fonts/Roboto-Regular.ttf");
    sk_sp<SkTypeface> dataTypeface(fontMgr->makeFromFile(filePath.c_str(), 0));
    if (!dataTypeface) {
        ERRORF(reporter, "Could not find data typeface. FcVersion: %d", FcGetVersion());
        return;
    }

    SkFont dataFont(dataTypeface, kTextSize);
    dataFont.setEmbolden(true);

    sk_sp<SkTypeface> matchTypeface(fontMgr->matchFamilyStyle("Roboto", SkFontStyle()));
    if (!matchTypeface) {
        ERRORF(reporter, "Could not find match typeface. FcVersion: %d", FcGetVersion());
        return;
    }
    SkFont matchFont(matchTypeface, kTextSize);

    SkBitmap bitmapData;
    bitmapData.allocN32Pixels(64, 64);
    SkCanvas canvasData(bitmapData);

    SkBitmap bitmapMatch;
    bitmapMatch.allocN32Pixels(64, 64);
    SkCanvas canvasMatch(bitmapMatch);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);

    canvasData.drawColor(SK_ColorGRAY);
    canvasData.drawString(text, 20.0f, 20.0f, dataFont, paint);
    if ((false)) {
        // In case we wonder what's been painted
        SkString dataPath = GetResourcePath("/fonts/data.png");
        write_bitmap(&bitmapData, dataPath.c_str());
    }

    canvasMatch.drawColor(SK_ColorGRAY);
    canvasMatch.drawString(text, 20.0f, 20.0f, matchFont, paint);
    if ((false)) {
        SkString matchPath = GetResourcePath("/fonts/match.png");
        write_bitmap(&bitmapMatch, matchPath.c_str());
    }

    bool success = bitmap_compare(bitmapData, bitmapMatch);
    REPORTER_ASSERT(reporter, success);
}

#if defined(SK_TYPEFACE_FACTORY_FREETYPE)
DEF_TEST(FontMgrFontConfig_FreeType_AllBold, reporter) {

    FcConfig* config = build_fontconfig_from_resources();
    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_FontConfig(config, SkFontScanner_Make_FreeType()));

    testAllBold(fontMgr, reporter);
}
#endif

#if defined(SK_TYPEFACE_FACTORY_FONTATIONS)
DEF_TEST(FontMgrFontConfig_Fontations_AllBold, reporter) {

    FcConfig* config = build_fontconfig_from_resources();
    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_FontConfig(config, SkFontScanner_Make_Fontations()));

    testAllBold(fontMgr, reporter);
}
#endif

#if defined(SK_TYPEFACE_FACTORY_FREETYPE) && defined(SK_TYPEFACE_FACTORY_FONTATIONS)
// The results may not match but it's still interesting to run sometimes
DEF_TEST_DISABLED(FontMgrFontConfig_MatchFonts, reporter) {
    FcConfig* config = build_fontconfig_from_resources();
    sk_sp<SkFontMgr> freeTypeFontMgr(
            SkFontMgr_New_FontConfig(config, SkFontScanner_Make_FreeType()));
    sk_sp<SkFontMgr> fontationsFontMgr(
            SkFontMgr_New_FontConfig(config, SkFontScanner_Make_Fontations()));

    constexpr float kTextSize = 20;
    constexpr char text[] = "abc";

    sk_sp<SkTypeface> dataTypeface(freeTypeFontMgr->matchFamilyStyle("Roboto", SkFontStyle()));
    if (!dataTypeface) {
        ERRORF(reporter, "Could not find data typeface. FcVersion: %d", FcGetVersion());
        return;
    }
    SkFont dataFont(dataTypeface, kTextSize);

    sk_sp<SkTypeface> matchTypeface(fontationsFontMgr->matchFamilyStyle("Roboto", SkFontStyle()));
    if (!matchTypeface) {
        ERRORF(reporter, "Could not find match typeface. FcVersion: %d", FcGetVersion());
        return;
    }
    SkFont matchFont(matchTypeface, kTextSize);

    SkBitmap bitmapData;
    bitmapData.allocN32Pixels(64, 64);
    SkCanvas canvasData(bitmapData);

    SkBitmap bitmapMatch;
    bitmapMatch.allocN32Pixels(64, 64);
    SkCanvas canvasMatch(bitmapMatch);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);

    canvasData.drawColor(SK_ColorGRAY);
    canvasData.drawString(text, 20.0f, 20.0f, dataFont, paint);
    if ((false)) {
        // In case we wonder what's been painted
        SkString dataPath = GetResourcePath("/fonts/data.png");
        write_bitmap(&bitmapData, dataPath.c_str());
    }

    canvasMatch.drawColor(SK_ColorGRAY);
    canvasMatch.drawString(text, 20.0f, 20.0f, matchFont, paint);
    if ((false)) {
        SkString matchPath = GetResourcePath("/fonts/match.png");
        write_bitmap(&bitmapMatch, matchPath.c_str());
    }

    bool success = bitmap_compare(bitmapData, bitmapMatch);
    REPORTER_ASSERT(reporter, success);
}
#endif

namespace {

struct PatternElem {
    const char* fValue;
    FcValueBinding fBinding;
};

static void populate_pattern(FcPattern* pat, const char* object, SkSpan<const PatternElem> elements) {
    for (const auto& elem : elements) {
        FcValue v;
        v.type = FcTypeString;
        v.u.s = reinterpret_cast<const FcChar8*>(elem.fValue);
        if (elem.fBinding == FcValueBindingWeak) {
            FcPatternAddWeak(pat, object, v, FcTrue);
        } else {
            FcPatternAdd(pat, object, v, FcTrue);
        }
    }
}

static void verify_pattern(skiatest::Reporter* reporter,
                           FcPattern* pat,
                           const char* object,
                           SkSpan<const PatternElem> expected,
                           const char* testName) {
    int index = 0;
    while (true) {
        FcValue value;
        FcValueBinding binding;
        FcResult result = FcPatternGetWithBinding(pat, object, index, &value, &binding);
        if (result != FcResultMatch) {
            REPORTER_ASSERT(reporter, index == static_cast<int>(expected.size()),
                            "%s: Expected %zu elements but found %d",
                            testName, expected.size(), index);
            break;
        }
        if (index < static_cast<int>(expected.size())) {
            REPORTER_ASSERT(reporter, value.type == FcTypeString,
                            "%s: Unexpected value type at index %d", testName, index);
            REPORTER_ASSERT(reporter, !strcmp(reinterpret_cast<const char*>(value.u.s), expected[index].fValue),
                            "%s: Mismatch at %d: got '%s', expected '%s'",
                            testName, index, reinterpret_cast<const char*>(value.u.s), expected[index].fValue);
            REPORTER_ASSERT(reporter, binding == expected[index].fBinding,
                            "%s: Binding mismatch at %d: got %d, expected %d",
                            testName, index, binding, expected[index].fBinding);
        }
        ++index;
    }
}

static FcConfig* build_fontconfig_with_custom_aliases() {
    SkString path = GetResourcePath("");
    SkString content;
    content.append("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                   "<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">\n"
                   "<fontconfig>\n"
                   "<dir>/fonts</dir>\n"
                   // Alias: "CustomGenericSans" -> prefer "Roboto", default fallback "DejaVu Sans"
                   "<alias>\n"
                   "  <family>CustomGenericSans</family>\n"
                   "  <prefer><family>Roboto</family></prefer>\n"
                   "  <default><family>DejaVu Sans</family></default>\n"
                   "</alias>\n"
                   // Alias: "CustomPreferDejaVu" -> prefer "DejaVu Sans", default fallback "Roboto"
                   "<alias>\n"
                   "  <family>CustomPreferDejaVu</family>\n"
                   "  <prefer><family>DejaVu Sans</family></prefer>\n"
                   "  <default><family>Roboto</family></default>\n"
                   "</alias>\n"
                   // Alias: "CustomFallbackOnly" -> only default fallback "Roboto"
                   "<alias>\n"
                   "  <family>CustomFallbackOnly</family>\n"
                   "  <default><family>Roboto</family></default>\n"
                   "</alias>\n"
                   "</fontconfig>\n");

    FcConfig* fc_config = FcConfigCreate();
    FcConfigSetSysRoot(fc_config, reinterpret_cast<const FcChar8*>(path.c_str()));
    if (FcConfigParseAndLoadFromMemory(
                fc_config, reinterpret_cast<const FcChar8*>(content.c_str()),
                FcTrue) != FcTrue) {
        SkDebugf("FcConfigParseAndLoadFromMemory failed\n");
    }
    if (FcConfigBuildFonts(fc_config) != FcTrue) {
        SkDebugf("!FcConfigBuildFonts failed\n");
    }
    return fc_config;
}

}  // namespace

DEF_TEST(FontMgrFontConfig_RemoveWeakPatterns, reporter) {
    constexpr FcValueBinding S = FcValueBindingStrong;
    constexpr FcValueBinding W = FcValueBindingWeak;

    struct TestCase {
        const char* name;
        const char* object;
        std::vector<PatternElem> input;
        std::vector<PatternElem> expected;
    };

    TestCase testCases[] = {
        // 1. Motivating CL bug case:
        // [ "sans-serif" (S), "Noto Sans" (S), "DejaVu Sans" (W), "CustomCorpSans" (S), "Fallback" (W) ]
        // Weak binding between strong elements is pruned, and trailing weak fallback is pruned.
        {
            "CL_Motivating_Bug_Interleaved_Weak_Before_Strong",
            FC_FAMILY,
            { {"sans-serif", S}, {"Noto Sans", S}, {"DejaVu Sans", W}, {"CustomCorpSans", S}, {"Fallback", W} },
            { {"sans-serif", S}, {"Noto Sans", S}, {"CustomCorpSans", S} }
        },
        // 2. Interleaved strong and weak starting with strong:
        // [ S, W, S, W, S ] -> [ S, S, S ]
        {
            "Interleaved_Strong_Weak",
            FC_FAMILY,
            { {"s0", S}, {"w1", W}, {"s2", S}, {"w3", W}, {"s4", S} },
            { {"s0", S}, {"s2", S}, {"s4", S} }
        },
        // 3. Multiple consecutive weak bindings after strong:
        // [ S, W, W, S, W, W ] -> [ S, S ]
        {
            "Multiple_Consecutive_Weak_After_Strong",
            FC_FAMILY,
            { {"s0", S}, {"w1", W}, {"w2", W}, {"s3", S}, {"w4", W}, {"w5", W} },
            { {"s0", S}, {"s3", S} }
        },
        // 4. Leading weak bindings (e.g. from <prefer> aliases) followed by strong and trailing weak:
        // [ W, W, S, W, S, W ] -> [ W, W, S, S ]
        {
            "Leading_Weak_Prefer_Aliases_Preserved",
            FC_FAMILY,
            { {"pref1", W}, {"pref2", W}, {"requested", S}, {"default1", W}, {"custom_strong", S}, {"default2", W} },
            { {"pref1", W}, {"pref2", W}, {"requested", S}, {"custom_strong", S} }
        },
        // 5. Weak binding at index 0 followed only by strong bindings:
        // [ W, S, S ] -> [ W, S, S ]
        {
            "Weak_At_Index_0_Followed_By_Strong",
            FC_FAMILY,
            { {"w0", W}, {"s1", S}, {"s2", S} },
            { {"w0", W}, {"s1", S}, {"s2", S} }
        },
        // 6. Patterns with ONLY weak bindings (no strong binding in pattern):
        // [ W, W, W ] -> [ W, W, W ]
        {
            "Only_Weak_Bindings",
            FC_FAMILY,
            { {"w0", W}, {"w1", W}, {"w2", W} },
            { {"w0", W}, {"w1", W}, {"w2", W} }
        },
        // 7. Patterns with ONLY strong bindings:
        // [ S, S, S ] -> [ S, S, S ]
        {
            "Only_Strong_Bindings",
            FC_FAMILY,
            { {"s0", S}, {"s1", S}, {"s2", S} },
            { {"s0", S}, {"s1", S}, {"s2", S} }
        },
        // 8. Single weak element:
        // [ W ] -> [ W ]
        {
            "Single_Weak_Element",
            FC_FAMILY,
            { {"w0", W} },
            { {"w0", W} }
        },
        // 9. Single strong element:
        // [ S ] -> [ S ]
        {
            "Single_Strong_Element",
            FC_FAMILY,
            { {"s0", S} },
            { {"s0", S} }
        },
        // 10. Empty pattern:
        // [ ] -> [ ]
        {
            "Empty_Pattern",
            FC_FAMILY,
            {},
            {}
        },
        // 11. Property other than FC_FAMILY (e.g. FC_POSTSCRIPT_NAME):
        {
            "Postscript_Name_Property",
            FC_POSTSCRIPT_NAME,
            { {"ps0", S}, {"ps1", W}, {"ps2", S}, {"ps3", W} },
            { {"ps0", S}, {"ps2", S} }
        }
    };

    for (const auto& tc : testCases) {
        FcPattern* pat = FcPatternCreate();
        populate_pattern(pat, tc.object, SkSpan(tc.input.data(), tc.input.size()));

        SkFontMgr_FontConfig_remove_weak(pat, tc.object);

        verify_pattern(reporter, pat, tc.object, SkSpan(tc.expected.data(), tc.expected.size()), tc.name);
        FcPatternDestroy(pat);
    }
}

#if defined(SK_TYPEFACE_FACTORY_FREETYPE)
DEF_TEST(FontMgrFontConfig_AliasesAndSubstitution, reporter) {
    FcConfig* config = build_fontconfig_with_custom_aliases();
    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_FontConfig(config, SkFontScanner_Make_FreeType()));
    if (!fontMgr) {
        ERRORF(reporter, "Failed to create fontMgr");
        return;
    }

    // 1. matchFamily for "CustomGenericSans" should match the preferred font "Roboto", not "DejaVu Sans"
    {
        sk_sp<SkFontStyleSet> styleSet = fontMgr->matchFamily("CustomGenericSans");
        REPORTER_ASSERT(reporter, styleSet != nullptr);
        REPORTER_ASSERT(reporter, styleSet->count() > 0);

        sk_sp<SkTypeface> tf = styleSet->matchStyle(SkFontStyle::Normal());
        REPORTER_ASSERT(reporter, tf != nullptr);
        if (tf) {
            SkString name;
            tf->getFamilyName(&name);
            REPORTER_ASSERT(reporter, name.equals("Roboto"),
                            "Expected 'Roboto', got '%s'", name.c_str());
        }
    }

    // 2. matchFamily for "CustomPreferDejaVu" should match "DejaVu Sans"
    {
        sk_sp<SkFontStyleSet> styleSet = fontMgr->matchFamily("CustomPreferDejaVu");
        REPORTER_ASSERT(reporter, styleSet != nullptr);
        REPORTER_ASSERT(reporter, styleSet->count() > 0);

        sk_sp<SkTypeface> tf = styleSet->matchStyle(SkFontStyle::Normal());
        REPORTER_ASSERT(reporter, tf != nullptr);
        if (tf) {
            SkString name;
            tf->getFamilyName(&name);
            REPORTER_ASSERT(reporter, name.equals("DejaVu Sans"),
                            "Expected 'DejaVu Sans', got '%s'", name.c_str());
        }
    }

    // 3. matchFamily for a non-existent family should NOT match weak default fallbacks
    {
        sk_sp<SkFontStyleSet> styleSet = fontMgr->matchFamily("CompletelyUnknownNonExistentFont");
        if (styleSet) {
            REPORTER_ASSERT(reporter, styleSet->count() == 0,
                            "Non-existent family should not match fallback fonts in matchFamily");
        }
    }

    // 4. matchFamily for "CustomFallbackOnly" should NOT match default fallback via matchFamily
    // because default fallbacks (weak bindings after the requested family) are pruned by remove_weak.
    {
        sk_sp<SkFontStyleSet> styleSet = fontMgr->matchFamily("CustomFallbackOnly");
        if (styleSet) {
            REPORTER_ASSERT(reporter, styleSet->count() == 0,
                            "CustomFallbackOnly should not match weak default fallback in matchFamily");
        }
    }

    // 5. Test StyleSet style matching and family name preservation
    {
        sk_sp<SkFontStyleSet> robotoSet = fontMgr->matchFamily("Roboto");
        REPORTER_ASSERT(reporter, robotoSet != nullptr);
        REPORTER_ASSERT(reporter, robotoSet->count() > 0);

        // Test matching bold style on StyleSet
        sk_sp<SkTypeface> boldTf = robotoSet->matchStyle(SkFontStyle::Bold());
        REPORTER_ASSERT(reporter, boldTf != nullptr);
        if (boldTf) {
            SkString famName;
            boldTf->getFamilyName(&famName);
            REPORTER_ASSERT(reporter, famName.equals("Roboto"),
                            "Expected family 'Roboto', got '%s'", famName.c_str());
        }

        // Test matching italic style on StyleSet
        sk_sp<SkTypeface> italicTf = robotoSet->matchStyle(SkFontStyle::Italic());
        REPORTER_ASSERT(reporter, italicTf != nullptr);
    }

    // 6. Test matchFamilyStyle direct lookup
    {
        sk_sp<SkTypeface> tf = fontMgr->matchFamilyStyle("Roboto", SkFontStyle::Normal());
        REPORTER_ASSERT(reporter, tf != nullptr);
        if (tf) {
            SkString name;
            tf->getFamilyName(&name);
            REPORTER_ASSERT(reporter, name.equals("Roboto"));
        }

        sk_sp<SkTypeface> nonExistentTf = fontMgr->matchFamilyStyle("NonExistentFamily123", SkFontStyle::Normal());
        REPORTER_ASSERT(reporter, nonExistentTf == nullptr);
    }
}
#endif
