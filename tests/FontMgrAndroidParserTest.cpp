/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
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
#include "include/private/SkSemaphore.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkTHash.h"
#include "src/ports/SkFontMgr_android_parser.h"
#include "src/utils/SkOSPath.h"
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
#include <thread>
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
    // "sans-serif" in "fonts/fonts.xml" is "fonts/Distortable.ttf"
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

    REPORTER_ASSERT(reporter, typeface->fontStyle() == SkFontStyle::Bold());
}

static void check_lazy_matches_eager(skiatest::Reporter* reporter,
                                     std::unique_ptr<SkFontScanner> (*makeScanner)()) {
    // Host builds have no /system/etc/fonts.xml, so a null config would compare two empty managers
    SkString basePath = GetResourcePath("fonts/");
    SkString fontsXml = GetResourcePath("fonts/fonts.xml");
    SkFontMgr_Android_CustomFonts custom;
    custom.fSystemFontUse = SkFontMgr_Android_CustomFonts::kOnlyCustom;
    custom.fBasePath = basePath.c_str();
    custom.fFontsXml = fontsXml.c_str();
    custom.fFallbackFontsXml = nullptr;
    custom.fIsolated = false;
    sk_sp<SkFontMgr> eager =
            SkFontMgr_New_Android(&custom, makeScanner(), SkFontMgr_Android_FontLoading::kEager);
    sk_sp<SkFontMgr> lazy =
            SkFontMgr_New_Android(&custom, makeScanner(), SkFontMgr_Android_FontLoading::kLazy);
    REPORTER_ASSERT(reporter, eager->countFamilies() > 0);
    REPORTER_ASSERT(reporter, eager->countFamilies() == lazy->countFamilies());
    for (int i = 0; i < eager->countFamilies(); ++i) {
        SkString eagerName, lazyName;
        eager->getFamilyName(i, &eagerName);
        lazy->getFamilyName(i, &lazyName);
        REPORTER_ASSERT(reporter, eagerName == lazyName);
        sk_sp<SkFontStyleSet> e = eager->createStyleSet(i);
        sk_sp<SkFontStyleSet> l = lazy->createStyleSet(i);
        REPORTER_ASSERT(reporter, e->count() > 0, "%s", eagerName.c_str());
        REPORTER_ASSERT(reporter, e->count() == l->count(), "%s", eagerName.c_str());
        for (int j = 0; j < std::min(e->count(), l->count()); ++j) {
            SkFontStyle es, ls;
            e->getStyle(j, &es, nullptr);
            l->getStyle(j, &ls, nullptr);
            REPORTER_ASSERT(reporter, es == ls, "%s[%d]", eagerName.c_str(), j);
            sk_sp<SkTypeface> et = e->createTypeface(j);
            sk_sp<SkTypeface> lt = l->createTypeface(j);
            REPORTER_ASSERT(reporter, lt && et->fontStyle() == lt->fontStyle());
            REPORTER_ASSERT(reporter, et->isFixedPitch() == lt->isFixedPitch());
        }
    }
    // Same resource name means the same file and index were picked
    static constexpr SkUnichar kChars[] = {'A',
                                           0x0905 /* Devanagari A */,
                                           0x4E2D /* CJK */,
                                           0x1F600 /* emoji */,
                                           0x0627 /* Arabic */,
                                           0x10FFFD /* not covered by any font */};
    const char* families[] = {nullptr, "sans-serif", "serif", "monospace"};
    const char* locales[] = {"en-US", "hi-IN", "ja-JP", "zh-Hans", "zh-Hant", "ko-KR"};
    for (const char* family : families) {
        for (const char* locale : locales) {
            for (SkUnichar c : kChars) {
                sk_sp<SkTypeface> et =
                        eager->matchFamilyStyleCharacter(family, SkFontStyle(), &locale, 1, c);
                sk_sp<SkTypeface> lt =
                        lazy->matchFamilyStyleCharacter(family, SkFontStyle(), &locale, 1, c);
                if (et && lt) {
                    SkString en, ln;
                    et->getFamilyName(&en);
                    lt->getFamilyName(&ln);
                    REPORTER_ASSERT(reporter, en == ln);
                } else {
                    REPORTER_ASSERT(reporter, !et && !lt);
                }
            }
        }
    }
}

static bool write_bytes_to_file(const SkString& path, const void* bytes, size_t size) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) {
        return false;
    }
    const bool ok = fwrite(bytes, 1, size, f) == size;
    fclose(f);
    return ok;
}

static void check_lazy_auto_style_and_missing_file(skiatest::Reporter* reporter,
                                                   std::unique_ptr<SkFontScanner> (*makeScanner)(),
                                                   const char* prefix) {
    SkString baseDir = skiatest::GetTmpDir();
    REPORTER_ASSERT(reporter, !baseDir.isEmpty());
    if (baseDir.isEmpty()) {
        return;
    }
    if (!baseDir.endsWith("/")) {
        baseDir.append("/");
    }

    sk_sp<SkData> ttcData = GetResourceAsData("fonts/test.ttc");
    sk_sp<SkData> cbdtData = GetResourceAsData("fonts/cbdt.ttf");
    REPORTER_ASSERT(reporter, ttcData && cbdtData);
    if (!ttcData || !cbdtData) {
        return;
    }

    // Per-scanner file names keep parallel dm test workers from unlinking each other's fixtures
    const SkString primaryFile = SkStringPrintf("%sPrimary.ttc", prefix);
    const SkString secondaryFile = SkStringPrintf("%sSecondary.ttc", prefix);
    const SkString bitmapFile = SkStringPrintf("%sColorBitmap.ttf", prefix);
    const SkString primaryPath = SkOSPath::Join(baseDir.c_str(), primaryFile.c_str());
    const SkString secondaryPath = SkOSPath::Join(baseDir.c_str(), secondaryFile.c_str());
    const SkString bitmapPath = SkOSPath::Join(baseDir.c_str(), bitmapFile.c_str());
    const SkString xmlPath =
            SkOSPath::Join(baseDir.c_str(), SkStringPrintf("%slazy_edge.xml", prefix).c_str());
    const SkString jbXmlPath =
            SkOSPath::Join(baseDir.c_str(), SkStringPrintf("%sjb_nameless.xml", prefix).c_str());

    // Omitting weight/style exercises apply_xml_style's kAuto/fWeight==0 path; ColorBitmap.ttf
    // exercises scanStyle's eager fallback when a font lacks scalable outlines
    const SkString xml = SkStringPrintf(
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
            "<familyset version=\"22\">\n"
            "  <family name=\"sans-serif\"><font index=\"0\">%s</font></family>\n"
            "  <family name=\"ttc-auto\">\n"
            "    <font index=\"0\">%s</font>\n"
            "    <font index=\"1\">%s</font>\n"
            "  </family>\n"
            "  <family><font index=\"1\">%s</font></family>\n"
            "  <family><font>%s</font></family>\n"
            "</familyset>\n",
            primaryFile.c_str(),
            secondaryFile.c_str(),
            secondaryFile.c_str(),
            secondaryFile.c_str(),
            bitmapFile.c_str());
    // Legacy JB <family> without <nameset> parses with fIsFallbackFont==false and empty fNames
    const SkString jbXml = SkStringPrintf(
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
            "<familyset>\n"
            "  <family><fileset><file>%s</file></fileset></family>\n"
            "</familyset>\n",
            bitmapFile.c_str());

    struct Cleanup {
        std::vector<SkString> paths;
        ~Cleanup() {
            for (const SkString& p : paths) {
                std::remove(p.c_str());
            }
        }
    } cleanup{{primaryPath, secondaryPath, bitmapPath, xmlPath, jbXmlPath}};

    REPORTER_ASSERT(reporter, write_bytes_to_file(primaryPath, ttcData->data(), ttcData->size()));
    REPORTER_ASSERT(reporter, write_bytes_to_file(secondaryPath, ttcData->data(), ttcData->size()));
    REPORTER_ASSERT(reporter, write_bytes_to_file(bitmapPath, cbdtData->data(), cbdtData->size()));
    REPORTER_ASSERT(reporter, write_bytes_to_file(xmlPath, xml.c_str(), xml.size()));
    REPORTER_ASSERT(reporter, write_bytes_to_file(jbXmlPath, jbXml.c_str(), jbXml.size()));

    SkFontMgr_Android_CustomFonts custom;
    custom.fSystemFontUse = SkFontMgr_Android_CustomFonts::kOnlyCustom;
    custom.fBasePath = baseDir.c_str();
    custom.fFontsXml = xmlPath.c_str();
    custom.fFallbackFontsXml = nullptr;
    custom.fIsolated = false;

    sk_sp<SkFontMgr> eager =
            SkFontMgr_New_Android(&custom, makeScanner(), SkFontMgr_Android_FontLoading::kEager);
    sk_sp<SkFontMgr> lazy =
            SkFontMgr_New_Android(&custom, makeScanner(), SkFontMgr_Android_FontLoading::kLazy);
    sk_sp<SkFontStyleSet> eagerAuto = eager->matchFamily("ttc-auto");
    sk_sp<SkFontStyleSet> lazyAuto = lazy->matchFamily("ttc-auto");
    REPORTER_ASSERT(reporter, eagerAuto && lazyAuto && eagerAuto->count() == 2);
    REPORTER_ASSERT(reporter, lazyAuto->count() == 2);
    for (int i = 0; i < 2; ++i) {
        SkFontStyle es, ls;
        eagerAuto->getStyle(i, &es, nullptr);
        lazyAuto->getStyle(i, &ls, nullptr);
        REPORTER_ASSERT(reporter, es == ls);
    }
    SkFontStyle s0, s1;
    lazyAuto->getStyle(0, &s0, nullptr);
    lazyAuto->getStyle(1, &s1, nullptr);
    REPORTER_ASSERT(reporter, s0 == SkFontStyle::Normal() && s1 == SkFontStyle::Bold());

    // Unlink Primary.ttc after kLazy construction so first materialization fails inside SkOnce
    sk_sp<SkFontMgr> brokenLazy =
            SkFontMgr_New_Android(&custom, makeScanner(), SkFontMgr_Android_FontLoading::kLazy);
    REPORTER_ASSERT(reporter, std::remove(primaryPath.c_str()) == 0);

    // Materialize face 0 of Secondary.ttc, unlink Secondary.ttc, and verify face 1 reuses the
    // shared stream mapping rather than re-opening the file per TTC entry
    REPORTER_ASSERT(reporter, lazyAuto->createTypeface(0) != nullptr);
    REPORTER_ASSERT(reporter, std::remove(secondaryPath.c_str()) == 0);
    sk_sp<SkTypeface> sharedTtcFace1 = lazyAuto->createTypeface(1);
    REPORTER_ASSERT(reporter, sharedTtcFace1 && sharedTtcFace1->fontStyle() == SkFontStyle::Bold());

    sk_sp<SkFontStyleSet> sansSet = brokenLazy->matchFamily("sans-serif");
    REPORTER_ASSERT(reporter, sansSet && sansSet->count() == 1);
    SkFontStyle brokenStyle;
    sansSet->getStyle(0, &brokenStyle, nullptr);
    REPORTER_ASSERT(reporter, brokenStyle == SkFontStyle::Normal());
    REPORTER_ASSERT(reporter, sansSet->createTypeface(0) == nullptr);
    REPORTER_ASSERT(reporter, sansSet->createTypeface(0) == nullptr);
    REPORTER_ASSERT(reporter, brokenLazy->matchFamilyStyle("sans-serif", SkFontStyle()) == nullptr);
    REPORTER_ASSERT(reporter,
                    brokenLazy->legacyMakeTypeface("sans-serif", SkFontStyle()) == nullptr);
    REPORTER_ASSERT(reporter, brokenLazy->legacyMakeTypeface(nullptr, SkFontStyle()) != nullptr);

    // Restore Secondary.ttc before brokenLazy resolves fallback character '!' from it
    REPORTER_ASSERT(reporter, write_bytes_to_file(secondaryPath, ttcData->data(), ttcData->size()));
    sk_sp<SkTypeface> fallback =
            brokenLazy->matchFamilyStyleCharacter("sans-serif", SkFontStyle(), nullptr, 0, '!');
    REPORTER_ASSERT(reporter, fallback && fallback->fontStyle() == SkFontStyle::Bold());

    SkFontMgr_Android_CustomFonts jbCustom = custom;
    jbCustom.fFontsXml = jbXmlPath.c_str();
    sk_sp<SkFontMgr> jbEager =
            SkFontMgr_New_Android(&jbCustom, makeScanner(), SkFontMgr_Android_FontLoading::kEager);
    sk_sp<SkFontMgr> jbLazy =
            SkFontMgr_New_Android(&jbCustom, makeScanner(), SkFontMgr_Android_FontLoading::kLazy);
    sk_sp<SkTypeface> jbEagerTf = jbEager->legacyMakeTypeface(nullptr, SkFontStyle());
    sk_sp<SkTypeface> jbLazyTf = jbLazy->legacyMakeTypeface(nullptr, SkFontStyle());
    REPORTER_ASSERT(reporter, jbEagerTf && jbLazyTf);
    if (jbEagerTf && jbLazyTf) {
        SkString en, ln;
        jbEagerTf->getFamilyName(&en);
        jbLazyTf->getFamilyName(&ln);
        REPORTER_ASSERT(reporter, !en.isEmpty() && en == ln);
    }
}

static void check_lazy_concurrent_access(skiatest::Reporter* reporter,
                                         std::unique_ptr<SkFontScanner> (*makeScanner)()) {
    SkString basePath = GetResourcePath("fonts/");
    SkString fontsXml = GetResourcePath("fonts/fonts.xml");
    SkFontMgr_Android_CustomFonts custom;
    custom.fSystemFontUse = SkFontMgr_Android_CustomFonts::kOnlyCustom;
    custom.fBasePath = basePath.c_str();
    custom.fFontsXml = fontsXml.c_str();
    custom.fFallbackFontsXml = nullptr;
    custom.fIsolated = false;

    sk_sp<SkFontMgr> lazy =
            SkFontMgr_New_Android(&custom, makeScanner(), SkFontMgr_Android_FontLoading::kLazy);
    const int families = lazy->countFamilies();
    REPORTER_ASSERT(reporter, families > 0);

    constexpr int kThreads = 8;
    SkSemaphore ready;
    SkSemaphore start;
    struct Results {
        sk_sp<SkTypeface> sansNormal;
        sk_sp<SkTypeface> sansBold;
        sk_sp<SkTypeface> bangFallback;
    } results[kThreads];

    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&, t] {
            ready.signal();
            start.wait();
            results[t].sansNormal = lazy->matchFamilyStyle("sans-serif", SkFontStyle::Normal());
            results[t].sansBold = lazy->matchFamilyStyle("sans-serif", SkFontStyle::Bold());
            results[t].bangFallback =
                    lazy->matchFamilyStyleCharacter("sans-serif", SkFontStyle(), nullptr, 0, '!');
            for (int i = 0; i < families; ++i) {
                sk_sp<SkFontStyleSet> set = lazy->createStyleSet(i);
                for (int j = 0; j < set->count(); ++j) {
                    (void)set->createTypeface(j);
                }
            }
        });
    }
    for (int t = 0; t < kThreads; ++t) {
        ready.wait();
    }
    start.signal(kThreads);
    for (std::thread& th : threads) {
        th.join();
    }

    REPORTER_ASSERT(reporter, results[0].sansNormal && results[0].sansBold);
    REPORTER_ASSERT(reporter, results[0].bangFallback);
    for (int t = 1; t < kThreads; ++t) {
        REPORTER_ASSERT(reporter, results[t].sansNormal.get() == results[0].sansNormal.get());
        REPORTER_ASSERT(reporter, results[t].sansBold.get() == results[0].sansBold.get());
        REPORTER_ASSERT(reporter, results[t].bangFallback.get() == results[0].bangFallback.get());
    }
}

#if defined(SK_TYPEFACE_FACTORY_FREETYPE)
DEF_TEST(FontMgrAndroid_LazyParity_FreeType, reporter) {
    check_lazy_matches_eager(reporter, SkFontScanner_Make_FreeType);
}
DEF_TEST(FontMgrAndroid_LazyAutoStyleAndMissingFile_FreeType, reporter) {
    check_lazy_auto_style_and_missing_file(reporter, SkFontScanner_Make_FreeType, "ft_");
}
DEF_TEST(FontMgrAndroid_LazyConcurrentAccess_FreeType, reporter) {
    check_lazy_concurrent_access(reporter, SkFontScanner_Make_FreeType);
}
#endif

#if defined(SK_TYPEFACE_FACTORY_FONTATIONS)
DEF_TEST(FontMgrAndroid_LazyParity_Fontations, reporter) {
    check_lazy_matches_eager(reporter, SkFontScanner_Make_Fontations);
}
DEF_TEST(FontMgrAndroid_LazyAutoStyleAndMissingFile_Fontations, reporter) {
    check_lazy_auto_style_and_missing_file(reporter, SkFontScanner_Make_Fontations, "fn_");
}
DEF_TEST(FontMgrAndroid_LazyConcurrentAccess_Fontations, reporter) {
    check_lazy_concurrent_access(reporter, SkFontScanner_Make_Fontations);
}
#endif
