/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkFontMgr_mac_ct.h"
#include "include/ports/SkTypeface_mac.h"
#include "src/core/SkZip.h"
#include "src/ports/SkTypeface_mac_symbols.h"
#include "src/utils/SkFloatUtils.h"
#include "tests/Test.h"

#include <stdarg.h>
#include <string>
#include <vector>

static void SkMaybeDebugf(const char* fmt, ...) SK_PRINTF_LIKE(1, 2);

static void SkMaybeDebugf(const char* format, ...) {
    if ((false)) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}

DEF_TEST(TypefaceMacVariation, reporter) {
    auto makeSystemFont = [](float size) -> CTFontRef {
        // kCTFontUIFontSystem, kCTFontUIFontMessage
        return CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, size, nullptr);
    };

    auto tagToString = [](SkFourByteTag tag) -> std::string {
      char buffer[5];
      buffer[0] = (tag & 0xff000000) >> 24;
      buffer[1] = (tag & 0xff0000) >> 16;
      buffer[2] = (tag & 0xff00) >> 8;
      buffer[3] = tag & 0xff;
      buffer[4] = 0;
      return std::string(buffer);
    };

    // This typeface has the issue.
    sk_sp<SkTypeface> typeface(SkMakeTypefaceFromCTFont(makeSystemFont(30)));

    // Since MakeFromFile creates at default size 12, these two are more comparable.
    // The first one has the issue and the second does not.
    //typeface = SkMakeTypefaceFromCTFont(makeSystemFont(12));
    //typeface = SkTypeface::MakeFromFile("/System/Library/Fonts/SFNS.ttf");

    // Setting the initial opsz <= min, the reported wght axis is strange, but draws the same?
    //typeface = SkMakeTypefaceFromCTFont(makeSystemFont(17));
    //typeface = SkMakeTypefaceFromCTFont(makeSystemFont(17.01));

    // Setting the initial opsz >= max, the requested variation doesn't take effect?
    //typeface = SkMakeTypefaceFromCTFont(makeSystemFont(95.9));
    //typeface = SkMakeTypefaceFromCTFont(makeSystemFont(96));

    if (!typeface) {
        REPORTER_ASSERT(reporter, typeface);
        return;
    }
    using Coordinate = SkFontArguments::VariationPosition::Coordinate;
    using Axis = SkFontParameters::Variation::Axis;

    const int originalPositionCount = typeface->getVariationDesignPosition({});
    std::vector<Coordinate> originalPosition(originalPositionCount);
    const int retrievedOriginalPositionCount =
        typeface->getVariationDesignPosition(originalPosition);
    if (!(retrievedOriginalPositionCount == originalPositionCount)) {
        REPORTER_ASSERT(reporter, retrievedOriginalPositionCount == originalPositionCount);
        return;
    }

    constexpr SkFourByteTag kGRADTag = SkSetFourByteTag('G', 'R', 'A', 'D');
    constexpr SkFourByteTag kWghtTag = SkSetFourByteTag('w', 'g', 'h', 't');
    constexpr SkFourByteTag kWdthTag = SkSetFourByteTag('w', 'd', 't', 'h');
    constexpr SkFourByteTag kOpszTag = SkSetFourByteTag('o', 'p', 's', 'z');

    SkMaybeDebugf("Original: ");
    for (auto& originalCoordinate : originalPosition) {
        SkMaybeDebugf("(%s: %f) ", tagToString(originalCoordinate.axis).c_str(),
                                   originalCoordinate.value);
    }
    SkMaybeDebugf("\n\n");

    const int originalAxisCount = typeface->getVariationDesignParameters({});
    std::vector<Axis> originalAxes(originalAxisCount);
    const int returnedOriginalAxisCount =
        typeface->getVariationDesignParameters(originalAxes);
    if (!(returnedOriginalAxisCount == originalAxisCount)) {
        REPORTER_ASSERT(reporter, returnedOriginalAxisCount == originalAxisCount);
        return;
    }

    for (bool omitOpsz : {false, true}) {
    for (SkFourByteTag axisToBump : { 0u, kOpszTag, kWdthTag, kGRADTag }) {
    for (float testCoordinate : {100, 300, 400, 500, 700, 900}) {
        std::vector<Coordinate> expectedPosition;
        std::vector<Coordinate> requestPosition;
        SkMaybeDebugf("Request : ");
        for (auto& originalCoordinate : originalPosition) {
            float requestValue = originalCoordinate.value;
            if (originalCoordinate.axis == kOpszTag && omitOpsz) {
                SkMaybeDebugf("#%s: %f# ", tagToString(originalCoordinate.axis).c_str(),
                                           requestValue);
            } else {
                if (originalCoordinate.axis == axisToBump) {
                    // CoreText floats for the variation coordinates have limited precision.
                    // 'opsz' sems to have more precision since it is set somewhat independently.
                    // Though in mocOS 14 this seems to have changed and it can be more rounded.
                    //requestValue = nextafter(requestValue, HUGE_VALF); // Does not work.
                    requestValue += requestValue / 1024.0f; // Expect at least 10 bits.
                }
                if (originalCoordinate.axis == kWghtTag) {
                    requestValue = testCoordinate;
                }
                SkMaybeDebugf("(%s: %f) ", tagToString(originalCoordinate.axis).c_str(),
                                           requestValue);
                requestPosition.push_back({originalCoordinate.axis, requestValue});
            }

            float expectedValue = requestValue;
            for (auto& originalAxis : originalAxes) {
                if (originalAxis.tag == originalCoordinate.axis) {
                    expectedValue = std::min(expectedValue, originalAxis.max);
                    expectedValue = std::max(expectedValue, originalAxis.min);
                }
            }
            expectedPosition.push_back({originalCoordinate.axis, expectedValue});
        }
        SkMaybeDebugf("\n");

        SkMaybeDebugf("Expected: ");
        for (auto& expectedCoordinate : expectedPosition) {
             SkMaybeDebugf("(%s: %f) ", tagToString(expectedCoordinate.axis).c_str(),
                                        expectedCoordinate.value);
        }
        SkMaybeDebugf("\n");

        SkFontArguments::VariationPosition variationPosition =
            { requestPosition.data(), (int)requestPosition.size() };
        sk_sp<SkTypeface> cloneTypeface(
            typeface->makeClone(SkFontArguments().setVariationDesignPosition(variationPosition)));

        const int cloneAxisCount = cloneTypeface->getVariationDesignPosition({});
        std::vector<Coordinate> clonePosition(cloneAxisCount);
        const int retrievedCloneAxisCount =
            cloneTypeface->getVariationDesignPosition(clonePosition);
        if (!(retrievedCloneAxisCount == cloneAxisCount)) {
            REPORTER_ASSERT(reporter, retrievedCloneAxisCount == cloneAxisCount);
            continue;
        }

        SkMaybeDebugf("Result  : ");
        for (auto& cloneCoordinate : clonePosition) {
             SkMaybeDebugf("(%s: %f) ", tagToString(cloneCoordinate.axis).c_str(),
                                        cloneCoordinate.value);
        }
        SkMaybeDebugf("\n");

        if (clonePosition.size() != expectedPosition.size()) {
            REPORTER_ASSERT(reporter, clonePosition.size() == expectedPosition.size());
            continue;
        }

        auto compareCoordinate = [](const Coordinate& a, const Coordinate& b) -> bool {
            return a.axis < b.axis;
        };
        std::sort(clonePosition.begin(), clonePosition.end(), compareCoordinate);
        std::sort(expectedPosition.begin(), expectedPosition.end(), compareCoordinate);
        for (const auto&& [clone, expected] : SkMakeZip(clonePosition, expectedPosition)) {
            REPORTER_ASSERT(reporter, clone.axis == expected.axis, "%s == %s",
                            tagToString(clone.axis).c_str(), tagToString(expected.axis).c_str());

            // Allow a lot of slop here, ignoring the bottom 6 of the 23 bits.
            // CoreText appears to round `opsz` a lot starting in macOS 14.
            const SkFloatingPoint<float, 64> cloneValue(clone.value), expectedValue(expected.value);
            REPORTER_ASSERT(reporter, cloneValue.AlmostEquals(expectedValue), "%s:%f == %s:%f",
                            tagToString(clone.axis).c_str(), clone.value,
                            tagToString(expected.axis).c_str(), expected.value);
        }

        SkMaybeDebugf("\n");
    }
    }
    }
}

// Verifies Apple SF Symbols virtual typeface creation, PUA codepoint mapping,
// glyph metrics/IDs, and font manager family/character fallback integration.
DEF_TEST(TypefaceMac_SFSymbols, reporter) {
    // 1. Direct virtual typeface API: verify symbol discovery and PUA codepoint boundary checks.
    REPORTER_ASSERT(reporter, SkGetSFSymbolsCount() > 0);
    REPORTER_ASSERT(reporter, SkIsSFSymbolsCodepoint(kSFSymbolsBaseCodepoint));
    REPORTER_ASSERT(reporter, !SkIsSFSymbolsCodepoint(0x41)); // Standard ASCII 'A' must not match
    REPORTER_ASSERT(reporter, SkGetSFSymbolName(kSFSymbolsBaseCodepoint) != nullptr);

    // Verify virtual typeface instantiation, naming, and glyph ID lookup.
    sk_sp<SkTypeface> tf = SkMakeTypeface_SFSymbols(SkFontStyle::Normal());
    REPORTER_ASSERT(reporter, tf != nullptr);
    if (tf) {
        SkString family;
        tf->getFamilyName(&family);
        REPORTER_ASSERT(reporter, family.equals("SF Symbols"));

        SkString psName;
        tf->getPostScriptName(&psName);
        REPORTER_ASSERT(reporter, psName.equals("SF-Symbols"));

        // Total glyphs: missing glyph (0) + system symbols count
        REPORTER_ASSERT(reporter, tf->countGlyphs() == (int)SkGetSFSymbolsCount() + 1);

        // First symbol maps to glyph ID 1 (0 is reserved for .notdef)
        SkGlyphID glyph = tf->unicharToGlyph(kSFSymbolsBaseCodepoint);
        REPORTER_ASSERT(reporter, glyph == 1);

        // Unmapped codepoints resolve to missing glyph (0)
        SkGlyphID missingGlyph = tf->unicharToGlyph(0x41);
        REPORTER_ASSERT(reporter, missingGlyph == 0);
    }

    // 2. SkFontMgr integration: verify family matching and character fallback routing.
    sk_sp<SkFontMgr> fm = SkFontMgr_New_CoreText(nullptr);
    REPORTER_ASSERT(reporter, fm != nullptr);

#if defined(SK_ENABLE_SF_SYMBOLS_FALLBACK)
    // Verify direct family style query resolves to SF Symbols virtual typeface.
    sk_sp<SkTypeface> matched = fm->matchFamilyStyle("SF Symbols", SkFontStyle::Normal());
    REPORTER_ASSERT(reporter, matched != nullptr);

    // Verify fallback routing: requesting a character in Plane 16 PUA routes to SF Symbols.
    sk_sp<SkTypeface> fallback = fm->matchFamilyStyleCharacter(
            nullptr, SkFontStyle::Normal(), nullptr, 0, kSFSymbolsBaseCodepoint);
    REPORTER_ASSERT(reporter, fallback != nullptr);
    if (fallback) {
        SkString fallbackFamily;
        fallback->getFamilyName(&fallbackFamily);
        REPORTER_ASSERT(reporter, fallbackFamily.equals("SF Symbols"));
    }
#endif
}

// Verifies optical sizing detection for Apple system UI fonts:
// System fonts under 20.0 pt use Text design variants (.SFNS-Text / .SFNS-TextBold),
// while 20.0 pt and above use Display design variants (.SFNS-Display / .SFNS-DisplayBold).
DEF_TEST(TypefaceMac_OpticalSizing, reporter) {
    // Under 20.0 pt -> .SFNS-Text
    CTFontRef fontText = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, 16.0, nullptr);
    if (fontText) {
        sk_sp<SkTypeface> tfText = SkMakeTypefaceFromCTFont(fontText);
        CFRelease(fontText);
        REPORTER_ASSERT(reporter, tfText != nullptr);
        if (tfText) {
            SkString psText;
            tfText->getPostScriptName(&psText);
            REPORTER_ASSERT(reporter, psText.equals(".SFNS-Text"));
        }
    }

    // 20.0 pt and above -> .SFNS-Display
    CTFontRef fontDisplay = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, 28.0, nullptr);
    if (fontDisplay) {
        sk_sp<SkTypeface> tfDisplay = SkMakeTypefaceFromCTFont(fontDisplay);
        CFRelease(fontDisplay);
        REPORTER_ASSERT(reporter, tfDisplay != nullptr);
        if (tfDisplay) {
            SkString psDisplay;
            tfDisplay->getPostScriptName(&psDisplay);
            REPORTER_ASSERT(reporter, psDisplay.equals(".SFNS-Display"));
        }
    }

    // Emphasized (Bold) under 20.0 pt -> .SFNS-TextBold
    CTFontRef fontBoldText = CTFontCreateUIFontForLanguage(kCTFontUIFontEmphasizedSystem, 16.0, nullptr);
    if (fontBoldText) {
        sk_sp<SkTypeface> tfBoldText = SkMakeTypefaceFromCTFont(fontBoldText);
        CFRelease(fontBoldText);
        REPORTER_ASSERT(reporter, tfBoldText != nullptr);
        if (tfBoldText) {
            SkString psBoldText;
            tfBoldText->getPostScriptName(&psBoldText);
            REPORTER_ASSERT(reporter, psBoldText.equals(".SFNS-TextBold"));
        }
    }

    // Emphasized (Bold) 20.0 pt and above -> .SFNS-DisplayBold
    CTFontRef fontBoldDisplay = CTFontCreateUIFontForLanguage(kCTFontUIFontEmphasizedSystem, 28.0, nullptr);
    if (fontBoldDisplay) {
        sk_sp<SkTypeface> tfBoldDisplay = SkMakeTypefaceFromCTFont(fontBoldDisplay);
        CFRelease(fontBoldDisplay);
        REPORTER_ASSERT(reporter, tfBoldDisplay != nullptr);
        if (tfBoldDisplay) {
            SkString psBoldDisplay;
            tfBoldDisplay->getPostScriptName(&psBoldDisplay);
            REPORTER_ASSERT(reporter, psBoldDisplay.equals(".SFNS-DisplayBold"));
        }
    }
}
