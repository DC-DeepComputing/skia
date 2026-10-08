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
#include "include/ports/SkFontMgr_android.h"
#include "include/private/SkDebug.h"
#include "include/private/SkFixed.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkTHash.h"
#include "src/ports/SkFontMgr_android_parser.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/flags/CommandLineFlags.h"

#ifdef SK_TYPEFACE_FACTORY_FONTATIONS
#include "include/ports/SkFontScanner_Fontations.h"
#endif
#ifdef SK_TYPEFACE_FACTORY_FREETYPE
#include "include/ports/SkFontScanner_FreeType.h"
#endif

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

DECLARE_bool(verboseFontMgr)

int CountFallbacks(SkSpan<std::unique_ptr<FontFamily>> fontFamilies) {
    int countOfFallbackFonts = 0;
    for (auto&& family : fontFamilies) {
        if (family->fIsFallbackFont) {
            countOfFallbackFonts++;
        }
    }
    return countOfFallbackFonts;
}

//https://tools.ietf.org/html/rfc5234#appendix-B.1
static bool isALPHA(int c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

//https://tools.ietf.org/html/rfc5234#appendix-B.1
static bool isDIGIT(int c) {
    return ('0' <= c && c <= '9');
}

static void ValidateLoadedFonts(SkSpan<std::unique_ptr<FontFamily>> fontFamilies,
                                const char* firstExpectedFile,
                                skiatest::Reporter* reporter) {
    REPORTER_ASSERT(reporter, fontFamilies[0]->fNames.size() == 5);
    REPORTER_ASSERT(reporter, !strcmp(fontFamilies[0]->fNames[0].c_str(), "sans-serif"));
    REPORTER_ASSERT(reporter,
                    !strcmp(fontFamilies[0]->fFonts[0].fFileName.c_str(), firstExpectedFile));
    REPORTER_ASSERT(reporter, !fontFamilies[0]->fIsFallbackFont);

    // Check that the languages are all sane.
    for (auto&& fontFamily : fontFamilies) {
        for (auto&& lang : fontFamily->fLanguages) {
            const SkString& langString = lang.getTag();
            for (size_t i = 0; i < langString.size(); ++i) {
                int c = langString[i];
                REPORTER_ASSERT(reporter, isALPHA(c) || isDIGIT(c) || '-' == c);
            }
        }
    }

    // All file names in the test configuration files start with a capital letter.
    // This is not a general requirement, but it is true of all the test configuration data.
    // Verifying ensures the filenames have been read sanely and have not been 'sliced'.
    for (auto&& family : fontFamilies) {
        for (auto&& file : family->fFonts) {
            REPORTER_ASSERT(reporter, !file.fFileName.isEmpty() &&
                                      file.fFileName[0] >= 'A' &&
                                      file.fFileName[0] <= 'Z');
        }
    }
}

static void DumpFiles(const FontFamily& fontFamily) {
    for (auto&& ffi : fontFamily.fFonts) {
        SkDebugf("  file (%d) %s#%d", ffi.fWeight, ffi.fFileName.c_str(), ffi.fIndex);
        for (const auto& coordinate : ffi.fVariationDesignPosition) {
            SkDebugf(" @'%c%c%c%c'=%f",
                     (char)((coordinate.axis >> 24) & 0xFF),
                     (char)((coordinate.axis >> 16) & 0xFF),
                     (char)((coordinate.axis >> 8) & 0xFF),
                     (char)((coordinate.axis) & 0xFF),
                     coordinate.value);
        }
        SkDebugf("\n");
    }
}

static void DumpLoadedFonts(SkSpan<std::unique_ptr<FontFamily>> fontFamilies, const char* label) {
    if (!FLAGS_verboseFontMgr) {
        return;
    }

    SkDebugf("\n--- Dumping %s\n", label);
    size_t i = 0;
    for (auto&& family : fontFamilies) {
        SkDebugf("Family %zu:\n", i);
        switch(family->fVariant) {
            case kElegant_FontVariant: SkDebugf("  elegant\n"); break;
            case kCompact_FontVariant: SkDebugf("  compact\n"); break;
            default: break;
        }
        SkDebugf("  basePath %s\n", family->fBasePath.c_str());
        if (!family->fLanguages.empty()) {
            SkDebugf("  language");
            for (const auto& lang : family->fLanguages) {
                SkDebugf(" %s", lang.getTag().c_str());
            }
            SkDebugf("\n");
        }
        for (int j = 0; j < family->fNames.size(); ++j) {
            SkDebugf("  name %s\n", family->fNames[j].c_str());
        }
        DumpFiles(*family);
        for (const auto& [unused, fallbackFamily] : family->fallbackFamilies) {
            SkDebugf("  Fallback for: %s\n", fallbackFamily->fFallbackFor.c_str());
            DumpFiles(*fallbackFamily);
        }
        ++i;
    }
    SkDebugf("\n\n");
}

template <int N, typename T> static double test_parse_fixed_r(skiatest::Reporter* reporter,
                                                              double low, double high, double inc)
{
    double SK_FixedMax_double = nextafter(1 << (sizeof(T) * CHAR_BIT - N - 1), 0.0);
    double SK_FixedEpsilon_double = (1.0 / (1 << N));
    double maxError = 0;
    char buffer[64];
    for (double f = low; f < high; f += inc) {
        SkString s;
        // 'sprintf' formatting as expected depends on the current locale being "C".
        // We currently expect tests and tools to run in the "C" locale.
        snprintf(buffer, std::size(buffer), "%.20f", f);
        T fix;
        bool b = parse_fixed<N>(buffer, &fix);
        if (b) {
            double f2 = fix * SK_FixedEpsilon_double;
            double error = fabs(f - f2);
            REPORTER_ASSERT(reporter,  error <= SK_FixedEpsilon_double);
            maxError = std::max(maxError, error);
        } else {
            REPORTER_ASSERT(reporter, f < -SK_FixedMax_double || SK_FixedMax_double < f);
        }
    }

    //SkDebugf("maxError: %.20f\n", maxError);
    return maxError;
}

static void test_parse_fixed(skiatest::Reporter* reporter) {
    test_parse_fixed_r<27, int32_t>(reporter, -8.1, -7.9, 0.000001);
    test_parse_fixed_r<27, int32_t>(reporter, -0.1, 0.1, 0.000001);
    test_parse_fixed_r<27, int32_t>(reporter, 7.9, 8.1, 0.000001);
    test_parse_fixed_r<16, int32_t>(reporter, -0.125, 0.125, 1.0 / (1 << 19));
    test_parse_fixed_r<16, int32_t>(reporter, -32768.125, -32766.875, 1.0 / (1 << 17));
    test_parse_fixed_r<16, int32_t>(reporter, 32766.875, 32768.125, 1.0 / (1 << 17));
    test_parse_fixed_r<16, int32_t>(reporter, -1.1, 1.1, 0.0001);

    SkFixed fix;
    REPORTER_ASSERT(reporter, !parse_fixed<27>("-17.1", &fix));
    REPORTER_ASSERT(reporter, !parse_fixed<16>("32768", &fix));
    REPORTER_ASSERT(reporter, !parse_fixed<16>("", &fix));
    REPORTER_ASSERT(reporter, !parse_fixed<16>(".", &fix));
    REPORTER_ASSERT(reporter, !parse_fixed<16>("123.", &fix));
    REPORTER_ASSERT(reporter, !parse_fixed<16>("a", &fix));
    REPORTER_ASSERT(reporter, !parse_fixed<16>(".123a", &fix));
}

#ifdef SK_TYPEFACE_FACTORY_FONTATIONS
#define DEF_TEST_FONTATIONS(name, reporter) \
    DEF_TEST(name##Fontations, reporter) { name(reporter, SkFontScanner_Make_Fontations()); }
#else
#define DEF_TEST_FONTATIONS(name, reporter)
#endif

#define DEF_TEST_SCANNERS(name, reporter) \
    static void name(skiatest::Reporter*, std::unique_ptr<SkFontScanner>);                     \
    DEF_TEST(name, reporter) { name(reporter, SkFontScanner_Make_FreeType()); }                \
    DEF_TEST_FONTATIONS(name, reporter)                                                        \
    void name(skiatest::Reporter* reporter, std::unique_ptr<SkFontScanner> fs)

DEF_TEST_SCANNERS(FontMgrAndroidParser, reporter) {
    test_parse_fixed(reporter);

    bool resourcesMissing = false;

    std::vector<std::unique_ptr<FontFamily>> preV17FontFamilies;
    SkFontMgr_Android_Parser::GetCustomFontFamilies(preV17FontFamilies,
        SkString("/custom/font/path/"),
        GetResourcePath("android_fonts/pre_v17/system_fonts.xml").c_str(),
        GetResourcePath("android_fonts/pre_v17/fallback_fonts.xml").c_str());

    if (preV17FontFamilies.size() > 0) {
        REPORTER_ASSERT(reporter, preV17FontFamilies.size() == 14);
        REPORTER_ASSERT(reporter, CountFallbacks(preV17FontFamilies) == 10);

        DumpLoadedFonts(preV17FontFamilies, "pre version 17");
        ValidateLoadedFonts(preV17FontFamilies, "Roboto-Regular.ttf", reporter);
    } else {
        resourcesMissing = true;
    }


    std::vector<std::unique_ptr<FontFamily>> v17FontFamilies;
    SkFontMgr_Android_Parser::GetCustomFontFamilies(v17FontFamilies,
        SkString("/custom/font/path/"),
        GetResourcePath("android_fonts/v17/system_fonts.xml").c_str(),
        GetResourcePath("android_fonts/v17/fallback_fonts.xml").c_str(),
        GetResourcePath("android_fonts/v17").c_str());

    if (v17FontFamilies.size() > 0) {
        REPORTER_ASSERT(reporter, v17FontFamilies.size() == 56);
        REPORTER_ASSERT(reporter, CountFallbacks(v17FontFamilies) == 46);

        DumpLoadedFonts(v17FontFamilies, "version 17");
        ValidateLoadedFonts(v17FontFamilies, "Roboto-Regular.ttf", reporter);
    } else {
        resourcesMissing = true;
    }

    std::vector<std::unique_ptr<FontFamily>> v22FontFamilies;
    SkFontMgr_Android_Parser::GetCustomFontFamilies(v22FontFamilies,
        SkString("/custom/font/path/"),
        GetResourcePath("android_fonts/v22/fonts.xml").c_str(),
        nullptr);

    if (v22FontFamilies.size() > 0) {
        REPORTER_ASSERT(reporter, v22FontFamilies.size() == 54);
        REPORTER_ASSERT(reporter, CountFallbacks(v22FontFamilies) == 42);

        DumpLoadedFonts(v22FontFamilies, "version 22");
        ValidateLoadedFonts(v22FontFamilies, "Roboto-Thin.ttf", reporter);
    } else {
        resourcesMissing = true;
    }

    if (resourcesMissing) {
        SkDebugf("---- Resource files missing for FontConfigParser test\n");
    }
}

DEF_TEST_SCANNERS(FontMgrAndroidParserFamilyList, reporter) {
    bool resourcesMissing = false;
    std::vector<std::unique_ptr<FontFamily>> families;
    SkFontMgr_Android_Parser::GetCustomFontFamilies(
            families,
            SkString("/custom/font/path/"),
            GetResourcePath("android_fonts/family_list/fonts_family_list.xml").c_str(),
            nullptr);

    if (families.size() > 0) {
        REPORTER_ASSERT(reporter, families.size() == 2);

        REPORTER_ASSERT(reporter, families[0]->fNames.size() == 1);
        REPORTER_ASSERT(reporter, families[0]->fNames[0].equals("list-family"));
        REPORTER_ASSERT(reporter, families[0]->fFonts.size() == 1);
        REPORTER_ASSERT(reporter, families[0]->fFonts[0].fFileName.equals("ListFont.ttf"));
        REPORTER_ASSERT(reporter, !families[0]->fIsFallbackFont);

        REPORTER_ASSERT(reporter, families[1]->fNames.size() == 1);
        REPORTER_ASSERT(reporter, families[1]->fNames[0].equals("list-family"));
        REPORTER_ASSERT(reporter, families[1]->fFonts.size() == 1);
        REPORTER_ASSERT(reporter, families[1]->fFonts[0].fFileName.equals("ListFontFallback.ttf"));
        REPORTER_ASSERT(reporter, !families[1]->fIsFallbackFont);
    } else {
        resourcesMissing = true;
    }

    if (resourcesMissing) {
        SkDebugf("---- Resource files missing for FontConfigParser family-list test\n");
    }
}

DEF_TEST_SCANNERS(FontMgrAndroidParserFontsModification, reporter) {
    bool resourcesMissing = false;
    std::vector<std::unique_ptr<FontFamily>> families;
    SkFontMgr_Android_Parser::GetCustomFontFamilies(
            families,
            SkString("/custom/font/path/"),
            GetResourcePath("android_fonts/mod/fonts_modification.xml").c_str(),
            nullptr);

    if (families.size() > 0) {
        REPORTER_ASSERT(reporter, families.size() == 1);
        REPORTER_ASSERT(reporter, families[0]->fNames.size() == 1);
        REPORTER_ASSERT(reporter, families[0]->fNames[0].equals("mod-family"));
        REPORTER_ASSERT(reporter, families[0]->fFonts[0].fFileName.equals("ModFont.ttf"));
    } else {
        resourcesMissing = true;
    }

    if (resourcesMissing) {
        SkDebugf("---- Resource files missing for FontConfigParser fonts-modification test\n");
    }
}

DEF_TEST_SCANNERS(FontMgrAndroidParserAliasBugfix, reporter) {
    bool resourcesMissing = false;
    std::vector<std::unique_ptr<FontFamily>> families;
    SkFontMgr_Android_Parser::GetCustomFontFamilies(
            families,
            SkString("/custom/font/path/"),
            GetResourcePath("android_fonts/alias/fonts_alias_caps.xml").c_str(),
            nullptr);

    if (families.size() > 0) {
        REPORTER_ASSERT(reporter, families.size() == 1);
        REPORTER_ASSERT(reporter, families[0]->fNames.size() == 2);
        REPORTER_ASSERT(reporter, families[0]->fNames[0].equals("sans-serif-black"));
        REPORTER_ASSERT(reporter, families[0]->fNames[1].equals("test-alias"));
    } else {
        resourcesMissing = true;
    }

    if (resourcesMissing) {
        SkDebugf("---- Resource files missing for FontConfigParser alias bugfix test\n");
    }
}

DEF_TEST_SCANNERS(FontMgrAndroidLegacyMakeTypeface, reporter) {
    constexpr char fontsXmlFilename[] = "fonts/fonts.xml";
    SkString basePath = GetResourcePath("fonts/");
    SkString fontsXml = GetResourcePath(fontsXmlFilename);

    if (!sk_exists(fontsXml.c_str())) {
        ERRORF(reporter, "file missing: %s\n", fontsXmlFilename);
        return;
    }

    SkFontMgr_Android_CustomFonts custom;
    custom.fSystemFontUse = SkFontMgr_Android_CustomFonts::kOnlyCustom;
    custom.fBasePath = basePath.c_str();
    custom.fFontsXml = fontsXml.c_str();
    custom.fFallbackFontsXml = nullptr;
    custom.fIsolated = false;

    sk_sp<SkFontMgr> fm(SkFontMgr_New_Android(&custom, std::move(fs)));
    sk_sp<SkTypeface> t(fm->legacyMakeTypeface("non-existent-font", SkFontStyle()));
    REPORTER_ASSERT(reporter, nullptr == t);
}

static int bitmap_compare(const SkBitmap& ref, const SkBitmap& test) {
    int count = 0;
    for (int y = 0; y < test.height(); ++y) {
        for (int x = 0; x < test.width(); ++x) {
            SkColor testColor = test.getColor(x, y);
            SkColor refColor = ref.getColor(x, y);
            if (refColor != testColor) {
                ++count;
            }
        }
    }
    return count;
}

DEF_TEST_SCANNERS(FontMgrAndroidSystemVariableTypeface, reporter) {
    constexpr char fontsXmlFilename[] = "fonts/fonts.xml";
    SkString basePath = GetResourcePath("fonts/");
    SkString fontsXml = GetResourcePath(fontsXmlFilename);

    if (!sk_exists(fontsXml.c_str())) {
        ERRORF(reporter, "file missing: %s\n", fontsXmlFilename);
        return;
    }

    SkFontMgr_Android_CustomFonts custom;
    custom.fSystemFontUse = SkFontMgr_Android_CustomFonts::kOnlyCustom;
    custom.fBasePath = basePath.c_str();
    custom.fFontsXml = fontsXml.c_str();
    custom.fFallbackFontsXml = nullptr;
    custom.fIsolated = false;

    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_Android(&custom, std::move(fs)));
    // "sans-serif" in "fonts/fonts.xml" is "fonts/Distortable.ttf" with 9 weight entries.
    sk_sp<SkFontStyleSet> sansSerifSet(fontMgr->matchFamily("sans-serif"));
    REPORTER_ASSERT(reporter, sansSerifSet);
    REPORTER_ASSERT(reporter, sansSerifSet->count() == 9);

    constexpr float kExpectedWeights[9] = {
        0.5f, 0.65f, 0.85f, 1.0f, 1.2f, 1.4f, 1.6f, 1.8f, 2.0f
    };
    const SkFourByteTag wghtTag = SkSetFourByteTag('w', 'g', 'h', 't');
    for (int i = 0; i < sansSerifSet->count(); ++i) {
        SkFontStyle style;
        sansSerifSet->getStyle(i, &style, nullptr);
        REPORTER_ASSERT(reporter, style.weight() == (i + 1) * 100);

        sk_sp<SkTypeface> face(sansSerifSet->createTypeface(i));
        REPORTER_ASSERT(reporter, face);
        REPORTER_ASSERT(reporter, face->fontStyle().weight() == (i + 1) * 100);

        SkFontArguments::VariationPosition::Coordinate coord;
        int numCoords = face->getVariationDesignPosition({&coord, 1});
        REPORTER_ASSERT(reporter, numCoords == 1);
        if (numCoords == 1) {
            REPORTER_ASSERT(reporter, coord.axis == wghtTag);
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(coord.value, kExpectedWeights[i]));
        }
    }

    sk_sp<SkTypeface> typeface(fontMgr->legacyMakeTypeface("sans-serif", SkFontStyle()));

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
    paint.setAntiAlias(true);
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

        auto count = bitmap_compare(bitmapStream, bitmapClone);
        REPORTER_ASSERT(reporter, count == 0);
    }
}

DEF_TEST_SCANNERS(FontMgrAndroidSystemFallbackFor, reporter) {
    constexpr char fontsXmlFilename[] = "fonts/fonts.xml";
    SkString basePath = GetResourcePath("fonts/");
    SkString fontsXml = GetResourcePath(fontsXmlFilename);

    if (!sk_exists(fontsXml.c_str())) {
        ERRORF(reporter, "file missing: %s\n", fontsXmlFilename);
        return;
    }

    SkFontMgr_Android_CustomFonts custom;
    custom.fSystemFontUse = SkFontMgr_Android_CustomFonts::kOnlyCustom;
    custom.fBasePath = basePath.c_str();
    custom.fFontsXml = fontsXml.c_str();
    custom.fFallbackFontsXml = nullptr;
    custom.fIsolated = false;

    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_Android(&custom, std::move(fs)));
    // "sans-serif" in "fonts/fonts.xml" is "fonts/Distortable.ttf", which doesn't have a '!'
    // but "TestTTC" has a bold font which does have '!' and is marked as fallback for "sans-serif"
    // and should take precedence over the same font marked as normal weight next to it.
    sk_sp<SkTypeface> typeface(fontMgr->matchFamilyStyleCharacter(
        "sans-serif", SkFontStyle(), nullptr, 0, '!'));

    REPORTER_ASSERT(reporter, typeface);
    if (typeface) {
        REPORTER_ASSERT(reporter, typeface->fontStyle() == SkFontStyle::Bold());
    }
}

DEF_TEST_SCANNERS(FontMgrAndroidMissingFilesAndLifetime, reporter) {
    SkString basePath = GetResourcePath("fonts/");
    SkString v22FontsXml = GetResourcePath("android_fonts/v22/fonts.xml");
    if (!sk_exists(v22FontsXml.c_str())) {
        ERRORF(reporter, "file missing: android_fonts/v22/fonts.xml\n");
        return;
    }

    // Test missing font file filtering and partial family loading using v22/fonts.xml
    // pointed at resources/fonts/ (where Roboto-Regular.ttf and Distortable.ttf exist,
    // while other Roboto weights and Noto* fonts do not exist).
    SkFontMgr_Android_CustomFonts custom;
    custom.fSystemFontUse = SkFontMgr_Android_CustomFonts::kOnlyCustom;
    custom.fBasePath = basePath.c_str();
    custom.fFontsXml = v22FontsXml.c_str();
    custom.fFallbackFontsXml = nullptr;
    custom.fIsolated = false;

    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_Android(&custom, std::move(fs)));
    REPORTER_ASSERT(reporter, fontMgr);

    // "serif" has only missing NotoSerif-*.ttf files, so it should be skipped.
    sk_sp<SkFontStyleSet> serifSet(fontMgr->matchFamily("serif"));
    REPORTER_ASSERT(reporter, serifSet && serifSet->count() == 0);
    REPORTER_ASSERT(reporter, fontMgr->matchFamilyStyle("serif", SkFontStyle()) == nullptr);

    // "sans-serif" lists 12 Roboto files, of which only Roboto-Regular.ttf exists on disk.
    sk_sp<SkFontStyleSet> sansSet(fontMgr->matchFamily("sans-serif"));
    REPORTER_ASSERT(reporter, sansSet && sansSet->count() == 1);

    // "var" lists 9 variable-font instances of Distortable.ttf, all of which exist.
    sk_sp<SkFontStyleSet> varSet(fontMgr->matchFamily("var"));
    REPORTER_ASSERT(reporter, varSet && varSet->count() == 9);

    // Verify that an SkFontStyleSet outliving SkFontMgr remains safe to use.
    fontMgr.reset();
    sk_sp<SkTypeface> varFace(varSet->createTypeface(0));
    REPORTER_ASSERT(reporter, varFace);
    sk_sp<SkTypeface> matchedFace(varSet->matchStyle(SkFontStyle::Bold()));
    REPORTER_ASSERT(reporter, matchedFace);
}

DEF_TEST_SCANNERS(FontMgrAndroidFallbackLangAndVariant, reporter) {
    SkString basePath = GetResourcePath("fonts/");
    SkString tmpDir = skiatest::GetTmpDir();
    if (tmpDir.isEmpty()) {
        return;
    }

    SkString customXmlPath = SkOSPath::Join(tmpDir.c_str(), "skia_android_fallback_test.xml");
    // Place compact fallback before elegant fallback for "en", and separate "ja" (weight 400)
    // and "zh-Hans" (weight 700) fallbacks to test language and variant pre-filtering.
    constexpr char kCustomXml[] =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<familyset version=\"21\">\n"
        "    <family name=\"sans-serif\">\n"
        "        <font weight=\"400\" style=\"normal\">Distortable.ttf</font>\n"
        "    </family>\n"
        "    <family name=\"missing-family\">\n"
        "        <font weight=\"400\" style=\"normal\">NonExistentFont.ttf</font>\n"
        "    </family>\n"
        "    <family lang=\"ja\">\n"
        "        <font weight=\"400\" style=\"normal\" index=\"0\">test.ttc</font>\n"
        "    </family>\n"
        "    <family lang=\"zh-Hans\">\n"
        "        <font weight=\"700\" style=\"normal\" index=\"1\">test.ttc</font>\n"
        "    </family>\n"
        "    <family lang=\"en\" variant=\"compact\">\n"
        "        <font weight=\"400\" style=\"normal\" index=\"0\">test.ttc</font>\n"
        "    </family>\n"
        "    <family lang=\"en\" variant=\"elegant\">\n"
        "        <font weight=\"700\" style=\"normal\" index=\"1\">test.ttc</font>\n"
        "    </family>\n"
        "</familyset>\n";
    {
        SkFILEWStream xmlStream(customXmlPath.c_str());
        if (xmlStream.isValid()) {
            xmlStream.write(kCustomXml, sizeof(kCustomXml) - 1);
            xmlStream.flush();
        }
    }
    if (!sk_exists(customXmlPath.c_str())) {
        return;
    }

    SkFontMgr_Android_CustomFonts custom;
    custom.fSystemFontUse = SkFontMgr_Android_CustomFonts::kOnlyCustom;
    custom.fBasePath = basePath.c_str();
    custom.fFontsXml = customXmlPath.c_str();
    custom.fFallbackFontsXml = nullptr;
    custom.fIsolated = false;

    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_Android(&custom, std::move(fs)));

    // "missing-family" should be filtered out by HasExistingFontFile.
    REPORTER_ASSERT(reporter,
                    fontMgr->matchFamilyStyle("missing-family", SkFontStyle()) == nullptr);

    // Requesting '!' with "ja" should match the Japanese fallback (index 0, weight 400).
    const char* jaBcp47[] = {"ja"};
    sk_sp<SkTypeface> jaFace(fontMgr->matchFamilyStyleCharacter(
        "sans-serif", SkFontStyle(), jaBcp47, 1, '!'));
    REPORTER_ASSERT(reporter, jaFace);
    if (jaFace) {
        REPORTER_ASSERT(reporter, jaFace->fontStyle() == SkFontStyle::Normal());
    }

    // Requesting '!' with "zh-Hans-CN" should walk parent tag to "zh-Hans" and match
    // the Simplified Chinese fallback (index 1, weight 700).
    const char* zhBcp47[] = {"zh-Hans-CN"};
    sk_sp<SkTypeface> zhFace(fontMgr->matchFamilyStyleCharacter(
        "sans-serif", SkFontStyle(), zhBcp47, 1, '!'));
    REPORTER_ASSERT(reporter, zhFace);
    if (zhFace) {
        REPORTER_ASSERT(reporter, zhFace->fontStyle() == SkFontStyle::Bold());
    }

    // Requesting '!' with "en" tries elegant=true before elegant=false, so it should skip
    // the compact "en" fallback and match the elegant "en" fallback (index 1, weight 700).
    const char* enBcp47[] = {"en"};
    sk_sp<SkTypeface> enFace(fontMgr->matchFamilyStyleCharacter(
        "sans-serif", SkFontStyle(), enBcp47, 1, '!'));
    REPORTER_ASSERT(reporter, enFace);
    if (enFace) {
        REPORTER_ASSERT(reporter, enFace->fontStyle() == SkFontStyle::Bold());
    }

    remove(customXmlPath.c_str());
}


