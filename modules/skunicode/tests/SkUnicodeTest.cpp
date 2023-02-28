/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImageEncoder.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkBitSet.h"
#include "src/utils/SkOSPath.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <string.h>
#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <thread>

#include "modules/skunicode/include/SkUnicode.h"

namespace {
    SkString toString(SkUnicode::CodeUnitFlags flags) {
        SkString result;
        if ((flags & SkUnicode::kPartOfWhiteSpaceBreak) != 0) result.append("PartOfWhiteSpaceBreak+");
        if ((flags & SkUnicode::kGraphemeStart) != 0) result.append("GraphemeStart+");
        if ((flags & SkUnicode::kSoftLineBreakBefore) != 0) result.append("SoftLineBreakBefore+");
        if ((flags & SkUnicode::kHardLineBreakBefore) != 0) result.append("HardLineBreakBefore+");
        if ((flags & SkUnicode::kPartOfIntraWordBreak) != 0) result.append("PartOfIntraWordBreak+");
        if ((flags & SkUnicode::kControl) != 0) result.append("Control+");
        if ((flags & SkUnicode::kTabulation) != 0) result.append("Tabulation+");

        if (result.isEmpty()) {
            result.append("NoCodeUnitFlag");
        } else {
            result.resize(result.size() - 1);
        }
        return result;
    }
}

UNIX_ONLY_TEST(SkUnicode_GetWords, reporter) {
    SkString text("1 22 333 4444 55555 666666 7777777");
    std::vector<SkUnicode::Position> expected = { 0, 1, 2, 4, 5, 8, 9, 13, 14, 19, 20, 26, 27, 34 };

    auto icu = SkUnicode::Make();

    std::vector<SkUnicode::Position> results;
    auto result = icu->getWords(text.data(), text.size(), "en", &results);

    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == expected.size());
    for (auto i = 0ul; i < results.size(); ++i) {
        REPORTER_ASSERT(reporter, results[i] == expected[i]);
    }
}

UNIX_ONLY_TEST(SkUnicode_GetBidiRegionsLTR, reporter) {
    SkString text("1 22 333 4444 55555 666666 7777777");

    auto icu = SkUnicode::Make();

    std::vector<SkUnicode::BidiRegion> results;
    auto result = icu->getBidiRegions(text.data(),
                                      text.size(),
                                      SkUnicode::TextDirection::kLTR,
                                      &results);

    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == 1);
    REPORTER_ASSERT(reporter, results[0].start == 0 &&
                              results[0].end == text.size() &&
                              results[0].level == 0);
}
UNIX_ONLY_TEST(SkUnicode_GetBidiRegionsRTL, reporter) {
    SkString text("الهيمنة على العالم عبارة قبيحة ، أفضل أن أسميها تحسين العالم.");

    auto icu = SkUnicode::Make();

    std::vector<SkUnicode::BidiRegion> results;
    auto result = icu->getBidiRegions(text.data(),
                                      text.size(),
                                      SkUnicode::TextDirection::kRTL,
                                      &results);

    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == 1);
    REPORTER_ASSERT(reporter, results[0].start == 0 &&
                              results[0].end == text.size() &&
                              results[0].level == 1);
}

UNIX_ONLY_TEST(SkUnicode_GetBidiRegionsMix1, reporter) {
    // Spaces become Arabic (RTL) but numbers remain English (LTR)
    SkString text("1 22 333 4444 55555 666666 7777777");
    std::vector<SkUnicode::BidiRegion> expected = {
        {0, 1, 2},
        {1, 2, 1},
        {2, 4, 2},
        {4, 5, 1},
        {5, 8, 2},
        {8, 9, 1},
        {9, 13, 2},
        {13, 14, 1},
        {14, 19, 2},
        {19, 20, 1},
        {20, 26, 2},
        {26, 27, 1},
        {27, 34, 2},
    };

    auto icu = SkUnicode::Make();

    std::vector<SkUnicode::BidiRegion> results;
    auto result = icu->getBidiRegions(text.data(),
                                      text.size(),
                                      SkUnicode::TextDirection::kRTL,
                                      &results);

    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == expected.size());
    for (auto i = 0ul; i < results.size(); ++i) {
      REPORTER_ASSERT(reporter, results[i].start == expected[i].start &&
                                results[i].end == expected[i].end &&
                                results[i].level == expected[i].level);
    }
}

UNIX_ONLY_TEST(SkUnicode_GetBidiRegionsMix2, reporter) {
    // Few Russian/English words (ЛТР) in the mix
    SkString text("World ЛТР Domination هي عبارة قبيحة ، أفضل أن أسميها World ЛТР Optimization.");
    std::vector<SkUnicode::BidiRegion> expected = {
        { 0, 24, 0},
        { 24, 80, 1},
        { 80, 107, 0},
    };

    auto icu = SkUnicode::Make();

    std::vector<SkUnicode::BidiRegion> results;
    auto result = icu->getBidiRegions(text.data(),
                                      text.size(),
                                      SkUnicode::TextDirection::kLTR,
                                      &results);

    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == expected.size());
    for (auto i = 0ul; i < results.size(); ++i) {
        REPORTER_ASSERT(reporter, results[i].start == expected[i].start &&
                                  results[i].end == expected[i].end &&
                                  results[i].level == expected[i].level);
    }
}

UNIX_ONLY_TEST(SkUnicode_IsBreakingWhitespace, reporter) {

    char16_t whitespaces [] = {
        0x0009, // character tabulation
        0x000A, // line feed
        0x000B, // line tabulation
        0x000C, // form feed
        0x000D, // carriage return
        0x0020, // space
      //0x0085, // next line
      //0x00A0, // no-break space
        0x1680, // ogham space mark
        0x2000, // en quad
        0x2001, // em quad
        0x2002, // en space
        0x2003, // em space
        0x2004, // three-per-em space
        0x2005, // four-per-em space
        0x2006, // six-per-em space
      //0x2007, // figure space
        0x2008, // punctuation space
        0x2009, // thin space
        0x200A, // hair space
        0x2028, // line separator
        0x2029, // paragraph separator
      //0x202F, // narrow no-break space
        0x205F, // medium mathematical space
        0x3000, // ideographic space
    };
    char16_t letters[] = { 'a', 'b', 'c', 'd' };

    auto icu = SkUnicode::Make();

    for (auto& uc : whitespaces) {
        REPORTER_ASSERT(reporter, icu->isBreakingWhitespace(uc));
    }

    for (auto& uc : letters) {
        REPORTER_ASSERT(reporter, !icu->isBreakingWhitespace(uc));
    }
}
/*
UNIX_ONLY_TEST(SkUnicode_ToUpper, reporter) {

    SkString lower("abcdefghijklmnopqrstuvwxyz");
    SkString upper("ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    auto icu = SkUnicode::Make();

    auto icu_result1 = icu->toUpper(lower);
    REPORTER_ASSERT(reporter, icu_result1.equals(upper));

    auto icu_result2 = icu->toUpper(upper);
    REPORTER_ASSERT(reporter, icu_result2.equals(upper));
}
*/
UNIX_ONLY_TEST(SkUnicode_ComputeCodeUnitFlags, reporter) {

    //SkString text("World domination is such an ugly phrase - I prefer to call it world optimisation");
    SkString text("1\n22 333 4444 55555 666666 7777777");
    // 4 8 13 19 24
    auto icu = SkUnicode::Make();

    SkTArray<SkUnicode::CodeUnitFlags> results;
    auto result = icu->computeCodeUnitFlags(text.data(),
                                            text.size(),
                                            /*replaceTabs=*/true,
                                            &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == SkToS16(text.size() + 1));
    for (auto i = 0; i < results.size(); ++i) {
        auto flags = results[i];
        auto expected = SkUnicode::CodeUnitFlags::kGraphemeStart;
        if (i == 1) {
            expected |= SkUnicode::CodeUnitFlags::kControl;
        }
        if (i == 2) {
            expected |= SkUnicode::CodeUnitFlags::kHardLineBreakBefore;
        }
        if (i == 1 || i == 4 || i == 8 || i == 13 || i == 19 || i == 26) {
            expected |= SkUnicode::CodeUnitFlags::kPartOfWhiteSpaceBreak;
            expected |= SkUnicode::CodeUnitFlags::kPartOfIntraWordBreak;
        }
        if (i == 0 || i == 2 || i == 5 || i == 9 || i == 14 || i == 20
                                                 || i == 27 || i == 34) {
            expected |= SkUnicode::CodeUnitFlags::kSoftLineBreakBefore;
        }
        REPORTER_ASSERT(reporter, flags == expected);
        if (flags != expected) {
            SkDebugf("[%d]:\n%s != \n%s\n\n", i, toString(flags).data(), toString(expected).data());
        }
    }
}

UNIX_ONLY_TEST(SkUnicode_ReorderVisual, reporter) {

    auto icu = SkUnicode::Make();

    auto reorder = [&](std::vector<SkUnicode::BidiLevel> levels,
                       std::vector<int32_t> expected) {
            std::vector<int32_t> logicalOrder(levels.size());
            icu->reorderVisual(levels.data(), levels.size(), logicalOrder.data());
            for (auto i = 0ul; i < levels.size(); ++i) {
                REPORTER_ASSERT(reporter, expected[i] == logicalOrder[i]);
            }
        };
    reorder({}, {});
    reorder({0}, {0});
    reorder({1}, {0});
    reorder({0, 1, 0, 1}, {0, 1, 2, 3});
    // u"\u202Dabc\u202EDEF\u202Dghi\u202EJKL\u202Dmno":
    reorder({2, 3, 4, 5, 6}, {0, 2, 4, 3, 1});
}
