
/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/src/SkUnicode_hybrid.h"
#include "tests/Test.h"

#include <vector>
#include <unicode/uscript.h>

using namespace skia_private;

#if defined SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION
#if defined SK_UNICODE_ICU_IMPLEMENTATION
UNIX_ONLY_TEST(SkUnicode_Scripts, reporter) {
    SkString english_thai("English ตัวอักษรไทย English ตัวอักษรไทย");
    auto hybrid = ::MakeHybridBasedUnicode({
          UScriptCode::USCRIPT_THAI,
          UScriptCode::USCRIPT_LAO,
          UScriptCode::USCRIPT_MYANMAR,
          UScriptCode::USCRIPT_KHMER,
          UScriptCode::USCRIPT_HANGUL,
          UScriptCode::USCRIPT_HIRAGANA,
          UScriptCode::USCRIPT_KATAKANA,
          UScriptCode::USCRIPT_HAN,
      });

    auto flags = [&](SkUnicode::CodeUnitFlags code) -> SkString {
        SkString result;
        if (hybrid->hasGraphemeStartFlag(code)) result.append("G ");
        if (hybrid->hasSoftLineBreakFlag(code)) result.append("S ");
        if (hybrid->hasHardLineBreakFlag(code)) result.append("H ");
        if (hybrid->hasPartOfWhiteSpaceBreakFlag(code)) result.append("W ");
        return result;
    };

    skia_private::TArray<SkUnicode::CodeUnitFlags, true> results;
    hybrid->computeCodeUnitFlags(english_thai.data(), english_thai.size(), false, &results);
    for (int i = 0; i < results.size(); i++) {
        SkDebugf("%d: %s\n", i, flags(results[i]).c_str());
    }
}

UNIX_ONLY_TEST(SkUnicode_CompareImplementations, reporter) {
    SkString english_thai("English ตัวอักษรไทย English ตัวอักษรไทย");
    auto hybrid = ::MakeHybridBasedUnicode({
          UScriptCode::USCRIPT_THAI,
          UScriptCode::USCRIPT_LAO,
          UScriptCode::USCRIPT_MYANMAR,
          UScriptCode::USCRIPT_KHMER,
          UScriptCode::USCRIPT_HANGUL,
          UScriptCode::USCRIPT_HIRAGANA,
          UScriptCode::USCRIPT_KATAKANA,
          UScriptCode::USCRIPT_HAN,
      });

    auto ranges = ::breakTextIntoRanges(hybrid.get(),
                                    SkSpan<const char>(english_thai.data(), english_thai.size()));
    for (auto i = 0; i < ranges.size(); ++i) {
        SkDebugf("%d: %s @%d\n", i, ranges[i].first ? "icu" : "libgrapheme", ranges[i].second);
    }

    auto icu = SkUnicode::MakeIcuBasedUnicode();
    auto libgrapheme = SkUnicode::MakeLibgraphemeBasedUnicode();

    skia_private::TArray<SkUnicode::CodeUnitFlags, true> hybridResults;
    hybrid->computeCodeUnitFlags(english_thai.data(), english_thai.size(), false, &hybridResults);
    skia_private::TArray<SkUnicode::CodeUnitFlags, true> icuResults;
    icu->computeCodeUnitFlags(english_thai.data(), english_thai.size(), false, &icuResults);
    skia_private::TArray<SkUnicode::CodeUnitFlags, true> libgraphemeResults;
    libgrapheme->computeCodeUnitFlags(english_thai.data(), english_thai.size(), false, &libgraphemeResults);

    SkASSERT(hybridResults.size() == icuResults.size() &&
             hybridResults.size() == libgraphemeResults.size());

    for (auto i = 0; i < hybridResults.size(); ++i) {
        if (hybridResults[i] != icuResults[i]) {
            SkDebugf("icu[%d]: %d %d\n", i, hybridResults[i], icuResults[i]);
        }
        if (hybridResults[i] != libgraphemeResults[i]) {
            SkDebugf("libgrapheme[%d]: %d %d\n", i, hybridResults[i], libgraphemeResults[i]);
        }
    }
}
#endif
#endif
