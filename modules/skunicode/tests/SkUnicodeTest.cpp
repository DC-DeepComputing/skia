
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
#include "src/core/SkBitmaskEnum.h"
#include "tests/Test.h"

#include "modules/skunicode/include/SkUnicode.h"

#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu.h"
#endif
#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_libgrapheme.h"
#endif
#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu4x.h"
#endif
#if defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_client.h"
#endif
#if defined(SK_UNICODE_APPLE_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_apple.h"
#endif

#include <vector>

#ifdef SK_UNICODE_ICU_IMPLEMENTATION
#define DEF_TEST_ICU(name, reporter) \
    DEF_TEST(name##ICU, reporter) { name(reporter, SkUnicodes::ICU::Make()); }
#else
#define DEF_TEST_ICU(name, reporter)
#endif

#ifdef SK_UNICODE_ICU4X_IMPLEMENTATION
#define DEF_TEST_ICU4X(name, reporter) \
    DEF_TEST(name##ICU4X, reporter) { name(reporter, SkUnicodes::ICU4X::Make()); }
#else
#define DEF_TEST_ICU4X(name, reporter)
#endif

#ifdef SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION
#define DEF_TEST_LIBGRAPHEME(name, reporter) \
    DEF_TEST(name##LIBGRAPHEME, reporter) { name(reporter, SkUnicodes::Libgrapheme::Make()); }
#else
#define DEF_TEST_LIBGRAPHEME(name, reporter)
#endif

#ifdef SK_UNICODE_APPLE_IMPLEMENTATION
#define DEF_TEST_APPLE(name, reporter) \
    DEF_TEST(name##Apple, reporter) { name(reporter, SkUnicodes::Apple::Make()); }
#else
#define DEF_TEST_APPLE(name, reporter)
#endif

#define DEF_TEST_NOIMPL(name, reporter)

#define DEF_TEST_UNICODES(name, reporter) \
    static void name(skiatest::Reporter* reporter, sk_sp<SkUnicode> unicode); \
    DEF_TEST_ICU(name, reporter) \
    DEF_TEST_ICU4X(name, reporter) \
    DEF_TEST_LIBGRAPHEME(name, reporter) \
    DEF_TEST_APPLE(name, reporter) \
    DEF_TEST_NOIMPL(name, reporter) \
    void name(skiatest::Reporter* reporter, sk_sp<SkUnicode> unicode)

#define DEF_TEST_ICU_UNICODES(name, reporter) \
    static void name(skiatest::Reporter* reporter, sk_sp<SkUnicode> unicode); \
    DEF_TEST_ICU(name, reporter) \
    DEF_TEST_ICU4X(name, reporter) \
    DEF_TEST_APPLE(name, reporter) \
    DEF_TEST_NOIMPL(name, reporter) \
    void name(skiatest::Reporter* reporter, sk_sp<SkUnicode> unicode)

using namespace skia_private;

#ifdef SK_UNICODE_CLIENT_IMPLEMENTATION
UNIX_ONLY_TEST(SkUnicode_Client, reporter) {
    std::u16string text = u"\U000f2008";
    auto utf8 = SkUnicode::convertUtf16ToUtf8(text.data(), text.size());
    auto client = SkUnicodes::Client::Make
                  (SkSpan<char>(&utf8[0], utf8.size()), {}, {}, {});
    skia_private::TArray<SkUnicode::CodeUnitFlags, true> results;
    client->computeCodeUnitFlags(utf8.data(), utf8.size(), false, &results);

    for (auto flag : results) {
        REPORTER_ASSERT(reporter, !SkUnicode::hasPartOfWhiteSpaceBreakFlag(flag));
    }
}
#endif

#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
UNIX_ONLY_TEST(SkUnicode_Compiled_Native, reporter) {
    auto icu = SkUnicodes::ICU::Make();
    if (!icu) {
        REPORTER_ASSERT(reporter, icu);
        return;
    }
    std::u16string text = u"\U000f2008";
    auto utf8 = SkUnicode::convertUtf16ToUtf8(text.data(), text.size());
    skia_private::TArray<SkUnicode::CodeUnitFlags, true> results;
    icu->computeCodeUnitFlags(utf8.data(), utf8.size(), false, &results);
    for (auto flag : results) {
        REPORTER_ASSERT(reporter, !SkUnicode::hasPartOfWhiteSpaceBreakFlag(flag));
    }
}
#endif

#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
UNIX_ONLY_TEST(SkUnicode_GetUtf8Words, reporter) {
    SkString text("1 22 333 4444 55555 666666 7777777");
    std::vector<SkUnicode::Position> expected = { 0, 1, 2, 4, 5, 8, 9, 13, 14, 19, 20, 26, 27, 34 };
    auto libgrapheme = SkUnicodes::Libgrapheme::Make();
    std::vector<SkUnicode::Position> results;
    auto result = libgrapheme->getUtf8Words(text.data(), text.size(), "en", &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == expected.size());
    for (auto i = 0ul; i < results.size(); ++i) {
        REPORTER_ASSERT(reporter, results[i] == expected[i]);
    }
}
#endif

#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
UNIX_ONLY_TEST(SkUnicode_Compiled_GetSentences, reporter) {
    auto icu = SkUnicodes::ICU::Make();
    if (!icu) {
        REPORTER_ASSERT(reporter, icu);
        return;
    }
    SkString text("Hello world! Hello world? Hello world... Not a sentence end: 3.1415926");
    std::vector<SkUnicode::Position> expected = {0, 13, 26, 41, 70};
    std::vector<SkUnicode::Position> results;
    auto result = icu->getSentences(text.data(), text.size(), nullptr, &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == expected.size());
    for (auto i = 0ul; i < results.size(); ++i) {
        REPORTER_ASSERT(reporter, results[i] == expected[i]);
    }
}
#endif

bool hasWordFlag(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kWordBreak) == SkUnicode::kWordBreak;
}

// On Windows libgrapheme produces different results
DEF_TEST_ICU_UNICODES(SkUnicode_GetBidiRegionsLTR, reporter) {
    if (!unicode) {
        return;
    }
    SkString text("1 22 333 4444 55555 666666 7777777");
    std::vector<SkUnicode::BidiRegion> results;
    auto result = unicode->getBidiRegions(text.data(),
                                          text.size(),
                                          SkUnicode::TextDirection::kLTR,
                                          &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == 1);
    REPORTER_ASSERT(reporter, results[0].start == 0 &&
                              results[0].end == text.size() &&
                              results[0].level == 0);
}

DEF_TEST_ICU_UNICODES(SkUnicode_GetBidiRegionsRTL, reporter) {
    if (!unicode) {
        return;
    }
    SkString text("الهيمنة على العالم عبارة قبيحة ، أفضل أن أسميها تحسين العالم.");
    std::vector<SkUnicode::BidiRegion> results;
    auto result = unicode->getBidiRegions(text.data(),
                                          text.size(),
                                          SkUnicode::TextDirection::kRTL,
                                          &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == 1);
    REPORTER_ASSERT(reporter, results[0].start == 0 &&
                              results[0].end == text.size() &&
                              results[0].level == 1);
}

DEF_TEST_ICU_UNICODES(SkUnicode_GetBidiRegionsMix1, reporter) {
    if (!unicode) {
        return;
    }
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
    std::vector<SkUnicode::BidiRegion> results;
    auto result = unicode->getBidiRegions(text.data(),
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

DEF_TEST_ICU_UNICODES(SkUnicode_GetBidiRegionsMix2, reporter) {
    if (!unicode) {
      return;
    }
    // Few Russian/English words (ЛТР) in the mix
    SkString text("World ЛТР Domination هي عبارة قبيحة ، أفضل أن أسميها World ЛТР Optimization.");
    std::vector<SkUnicode::BidiRegion> expected = {
        { 0, 24, 0},
        { 24, 80, 1},
        { 80, 107, 0},
    };
    std::vector<SkUnicode::BidiRegion> results;
    auto result = unicode->getBidiRegions(text.data(),
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

// Currently, libgrapheme uses different default rules and produces slightly
// different results; it does not matter for text shaping
DEF_TEST_ICU_UNICODES(SkUnicode_ToUpper, reporter) {
    if (!unicode) {
        return;
    }
    SkString lower("abcdefghijklmnopqrstuvwxyz");
    SkString upper("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    auto icu_result1 = unicode->toUpper(lower);
    REPORTER_ASSERT(reporter, icu_result1.equals(upper));
    auto icu_result2 = unicode->toUpper(upper);
    REPORTER_ASSERT(reporter, icu_result2.equals(upper));
}

DEF_TEST_ICU_UNICODES(SkUnicode_ComputeCodeUnitFlags, reporter) {
    if (!unicode) {
        return;
    }
    //SkString text("World domination is such an ugly phrase - I prefer to call it world optimisation");
    SkString text("1\n22 333 4444 55555 666666 7777777");
    // 4 8 13 19 24
    TArray<SkUnicode::CodeUnitFlags> results;
    auto result = unicode->computeCodeUnitFlags(text.data(),
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
    }
}

DEF_TEST_UNICODES(SkUnicode_ReorderVisual, reporter) {
    if (!unicode) {
        return;
    }
    auto reorder = [&](std::vector<SkUnicode::BidiLevel> levels,
                       std::vector<int32_t> expected) {
            std::vector<int32_t> logicalOrder(levels.size());
            unicode->reorderVisual(levels.data(), levels.size(), logicalOrder.data());
            for (auto i = 0ul; i < levels.size(); ++i) {
                REPORTER_ASSERT(reporter, expected[i] == logicalOrder[i]);
            }
        };
    reorder({}, {});
    reorder({0}, {0});
    reorder({1}, {0});
    reorder({0, 1, 0, 1}, {0, 1, 2, 3});
}

[[maybe_unused]] static void SkUnicode_Emoji(SkUnicode* icu, skiatest::Reporter* reporter) {
    std::u32string emojis(U"😄😁😆😅😂🤣");
    std::u32string not_emojis(U"満毎行昼本可");
    for (auto e : emojis) {
        REPORTER_ASSERT(reporter, icu->isEmoji(e));
    }
    for (auto n: not_emojis) {
        REPORTER_ASSERT(reporter, !icu->isEmoji(n));
    }
}

#ifdef SK_UNICODE_ICU_IMPLEMENTATION
UNIX_ONLY_TEST(SkUnicode_Compiled_Emoji, reporter) {
    auto icu = SkUnicodes::ICU::Make();
    if (!icu) {
        REPORTER_ASSERT(reporter, icu);
        return;
    }
    SkUnicode_Emoji(icu.get(), reporter);
}
#endif

#ifdef SK_UNICODE_ICU4X_IMPLEMENTATION
UNIX_ONLY_TEST(SkUnicode_ICU4X_Emoji, reporter) {
    auto icu = SkUnicodes::ICU4X::Make();
    if (!icu) {
        REPORTER_ASSERT(reporter, icu);
        return;
    }
    SkUnicode_Emoji(icu.get(), reporter);
}
#endif

[[maybe_unused]] static void SkUnicode_Ideographic(SkUnicode* icu, skiatest::Reporter* reporter) {
    std::u32string ideographic(U"満毎行昼本可");
    std::u32string not_ideographic(U"😄😁😆😅😂🤣");
    for (auto i : ideographic) {
        REPORTER_ASSERT(reporter, icu->isIdeographic(i));
    }
    for (auto n: not_ideographic) {
        REPORTER_ASSERT(reporter, !icu->isIdeographic(n));
    }
}

#ifdef SK_UNICODE_ICU_IMPLEMENTATION
UNIX_ONLY_TEST(SkUnicode_Compiled_Ideographic, reporter) {
    auto icu = SkUnicodes::ICU::Make();
    if (!icu) {
        REPORTER_ASSERT(reporter, icu);
        return;
    }
    SkUnicode_Ideographic(icu.get(), reporter);
}
#endif

#ifdef SK_UNICODE_ICU4X_IMPLEMENTATION
UNIX_ONLY_TEST(SkUnicode_ICU4X_Ideographic, reporter) {
    auto icu = SkUnicodes::ICU4X::Make();
    if (!icu) {
        REPORTER_ASSERT(reporter, icu);
        return;
    }
    SkUnicode_Ideographic(icu.get(), reporter);
}
#endif

#ifdef SK_UNICODE_APPLE_IMPLEMENTATION
DEF_TEST(SkUnicode_Apple_Words, reporter) {
    auto unicode = SkUnicodes::Apple::Make();
    REPORTER_ASSERT(reporter, unicode);
    SkString text("1 22 333 4444 55555 666666 7777777");
    std::vector<SkUnicode::Position> expected = { 0, 1, 2, 4, 5, 8, 9, 13, 14, 19, 20, 26, 27, 34 };
    std::vector<SkUnicode::Position> results;
    auto result = unicode->getUtf8Words(text.data(), text.size(), nullptr, &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == expected.size());
    for (size_t i = 0; i < results.size() && i < expected.size(); ++i) {
        REPORTER_ASSERT(reporter, results[i] == expected[i]);
    }
}

DEF_TEST(SkUnicode_Apple_Sentences, reporter) {
    auto unicode = SkUnicodes::Apple::Make();
    REPORTER_ASSERT(reporter, unicode);
    SkString text("Hello world! Hello world? Hello world... Not a sentence end: 3.1415926");
    std::vector<SkUnicode::Position> expected = {0, 13, 26, 41, 70};
    std::vector<SkUnicode::Position> results;
    auto result = unicode->getSentences(text.data(), text.size(), nullptr, &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == expected.size());
    for (size_t i = 0; i < results.size() && i < expected.size(); ++i) {
        REPORTER_ASSERT(reporter, results[i] == expected[i]);
    }
}

DEF_TEST(SkUnicode_Apple_Emoji, reporter) {
    auto unicode = SkUnicodes::Apple::Make();
    REPORTER_ASSERT(reporter, unicode);
    SkUnicode_Emoji(unicode.get(), reporter);

    // Verify that Japanese characters, currency, and math symbols are NOT emojis
    std::u32string non_emojis = U"あいうえおアイウエオ€$¥+-×÷=√∞";
    for (auto ch : non_emojis) {
        REPORTER_ASSERT(reporter, !unicode->isEmoji(ch));
    }

    // Verify modifier bases
    REPORTER_ASSERT(reporter, unicode->isEmojiModifierBase(0x261D));  // Index pointing up (☝️)
    REPORTER_ASSERT(reporter, unicode->isEmojiModifierBase(0x270A));  // Raised fist (✊)
    REPORTER_ASSERT(reporter, unicode->isEmojiModifierBase(0x1F3C3)); // Runner (🏃)
    REPORTER_ASSERT(reporter, unicode->isEmojiModifierBase(0x1F3CA)); // Swimmer (🏊)
    REPORTER_ASSERT(reporter, unicode->isEmojiModifierBase(0x1F44D)); // Thumbs up (👍)

    // Verify tag components for subdivision flags
    REPORTER_ASSERT(reporter, unicode->isEmojiComponent(0xE0067)); // Tag 'g'
    REPORTER_ASSERT(reporter, unicode->isEmojiComponent(0xE007F)); // Cancel tag
}

DEF_TEST(SkUnicode_Apple_Emoji_Graphemes, reporter) {
    auto unicode = SkUnicodes::Apple::Make();
    REPORTER_ASSERT(reporter, unicode);
    // Compound emoji: Family (👨‍👩‍👧‍👦 = 25 UTF-8 bytes) followed by space and "Hello"
    std::string text = "👨‍👩‍👧‍👦 Hello 🇯🇵 👩🏽‍💻";
    TArray<SkUnicode::CodeUnitFlags> results;
    auto result = unicode->computeCodeUnitFlags(text.data(), text.size(), false, &results);
    REPORTER_ASSERT(reporter, result);
    // Family emoji starts at 0, internal bytes must NOT be grapheme starts
    REPORTER_ASSERT(reporter, SkUnicode::hasGraphemeStartFlag(results[0]));
    for (size_t i = 1; i < 25; ++i) {
        REPORTER_ASSERT(reporter, !SkUnicode::hasGraphemeStartFlag(results[i]));
    }
    // Space at byte 25 must be a grapheme start
    REPORTER_ASSERT(reporter, SkUnicode::hasGraphemeStartFlag(results[25]));
}

DEF_TEST(SkUnicode_Apple_HardLineBreaks, reporter) {
    auto unicode = SkUnicodes::Apple::Make();
    REPORTER_ASSERT(reporter, unicode);

    // Trailing newline in UTF-8
    std::string textWithTrailingNL = "hello\n";
    TArray<SkUnicode::CodeUnitFlags> results8;
    unicode->computeCodeUnitFlags(textWithTrailingNL.data(), textWithTrailingNL.size(), false, &results8);
    REPORTER_ASSERT(reporter, SkUnicode::hasHardLineBreakFlag(results8[textWithTrailingNL.size()]));

    // CRLF in UTF-8
    std::string textCRLF = "abc\r\ndef";
    results8.clear();
    unicode->computeCodeUnitFlags(textCRLF.data(), textCRLF.size(), false, &results8);
    REPORTER_ASSERT(reporter, SkUnicode::hasHardLineBreakFlag(results8[5]));

    // UTF-16 hard line breaks in computeCodeUnitFlags
    std::u16string text16 = u"abc\ndef";
    TArray<SkUnicode::CodeUnitFlags> results16;
    unicode->computeCodeUnitFlags(text16.data(), text16.size(), false, &results16);
    REPORTER_ASSERT(reporter, SkUnicode::hasHardLineBreakFlag(results16[4]));

    // UTF-16 hard line break status in SkBreakIterator
    auto breakIter = unicode->makeBreakIterator(SkUnicode::BreakType::kLines);
    REPORTER_ASSERT(reporter, breakIter);
    REPORTER_ASSERT(reporter, breakIter->setText(text16.data(), text16.size()));
    auto pos = breakIter->first();
    bool foundHardBreak = false;
    while (!breakIter->isDone()) {
        pos = breakIter->next();
        if (pos == 4 && breakIter->status() == static_cast<SkBreakIterator::Status>(SkUnicode::LineBreakType::kHardLineBreak)) {
            foundHardBreak = true;
        }
    }
    REPORTER_ASSERT(reporter, foundHardBreak);
}

DEF_TEST(SkUnicode_Apple_CodePoints, reporter) {
    auto unicode = SkUnicodes::Apple::Make();
    REPORTER_ASSERT(reporter, unicode);

    using namespace SkUnicodes::Apple::CodePoints;

    // 1. Mandatory hard break code points (UAX #14)
    const SkUnichar hardBreaks[] = {
        kLineFeed,            // '\n' (0x000A)
        kVerticalTab,         // '\v' (0x000B)
        kFormFeed,            // '\f' (0x000C)
        kCarriageReturn,      // '\r' (0x000D)
        kNextLine,            // NEL (0x0085)
        kLineSeparator,       // LS (0x2028)
        kParagraphSeparator,  // PS (0x2029)
    };
    for (SkUnichar hb : hardBreaks) {
        REPORTER_ASSERT(reporter, unicode->isHardBreak(hb));
    }

    // Non-breaks must return false
    const SkUnichar nonHardBreaks[] = {
        ' ', '\t', 'a', 'Z', '0', '-', 0x00A0 /* NBSP */, 0x200B /* ZWSP */
    };
    for (SkUnichar nhb : nonHardBreaks) {
        REPORTER_ASSERT(reporter, !unicode->isHardBreak(nhb));
    }

    // Test that every hard break triggers kHardLineBreakBefore in computeCodeUnitFlags
    for (SkUnichar hb : hardBreaks) {
        char buf[8];
        size_t len = SkUTF::ToUTF8(hb, buf);
        std::string s = "A";
        s.append(buf, len);
        s.append("B");
        TArray<SkUnicode::CodeUnitFlags> flags;
        unicode->computeCodeUnitFlags(s.data(), s.size(), false, &flags);
        size_t bPos = 1 + len;
        REPORTER_ASSERT(reporter, SkUnicode::hasHardLineBreakFlag(flags[bPos]));
    }

    // 2. Emoji syntax special code points (TR51)
    REPORTER_ASSERT(reporter, unicode->isEmojiComponent(kZeroWidthJoiner));
    REPORTER_ASSERT(reporter, unicode->isEmojiComponent(kCombiningEnclosingKeycap));
    REPORTER_ASSERT(reporter, unicode->isEmojiComponent(kVariationSelector16));

    // 3. Regional Indicator Symbols (U+1F1E6..U+1F1FF)
    for (SkUnichar u = kRegionalIndicatorStart; u <= kRegionalIndicatorEnd; ++u) {
        REPORTER_ASSERT(reporter, unicode->isRegionalIndicator(u));
        REPORTER_ASSERT(reporter, unicode->isEmojiComponent(u));
        REPORTER_ASSERT(reporter, unicode->isEmoji(u));
    }
    // Boundaries just outside the regional indicator range
    REPORTER_ASSERT(reporter, !unicode->isRegionalIndicator(kRegionalIndicatorStart - 1));
    REPORTER_ASSERT(reporter, !unicode->isRegionalIndicator(kRegionalIndicatorEnd + 1));

    // 4. Fitzpatrick Skin Tone Modifiers (U+1F3FB..U+1F3FF)
    for (SkUnichar u = kModifierFitzpatrickStart; u <= kModifierFitzpatrickEnd; ++u) {
        REPORTER_ASSERT(reporter, unicode->isEmojiModifier(u));
        REPORTER_ASSERT(reporter, unicode->isEmojiComponent(u));
    }
    // Boundaries outside Fitzpatrick modifiers
    REPORTER_ASSERT(reporter, !unicode->isEmojiModifier(kModifierFitzpatrickStart - 1));
    REPORTER_ASSERT(reporter, !unicode->isEmojiModifier(kModifierFitzpatrickEnd + 1));

    // 5. Hair Components (U+1F9B0..U+1F9B3)
    for (SkUnichar u = kEmojiHairComponentStart; u <= kEmojiHairComponentEnd; ++u) {
        REPORTER_ASSERT(reporter, unicode->isEmojiComponent(u));
    }
    REPORTER_ASSERT(reporter, !unicode->isEmojiComponent(kEmojiHairComponentStart - 1));
    REPORTER_ASSERT(reporter, !unicode->isEmojiComponent(kEmojiHairComponentEnd + 1));

    // 6. Emoji Tag Characters (U+E0020..U+E007F)
    for (SkUnichar u = kTagSpace; u <= kTagCancel; ++u) {
        REPORTER_ASSERT(reporter, unicode->isEmojiComponent(u));
    }
    REPORTER_ASSERT(reporter, !unicode->isEmojiComponent(kTagSpace - 1));
    REPORTER_ASSERT(reporter, !unicode->isEmojiComponent(kTagCancel + 1));
}
#endif

