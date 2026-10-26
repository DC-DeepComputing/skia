/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkDebug.h"
#include "modules/skunicode/src/SkUnicode_hardcoded.h"
#include <algorithm>
#include <array>
#include <utility>

bool SkUnicodeHardCodedCharProperties::isControl(SkUnichar utf8) {
    // Match u_iscntrl across Cc (Control), Cf (Format), Zl (Line Separator),
    // and Zp (Paragraph Separator) categories
    static constexpr std::array<std::pair<SkUnichar, SkUnichar>, 23> ranges {{
            {0x0000, 0x001F},
            {0x007F, 0x009F},
            {0x00AD, 0x00AD},
            {0x0600, 0x0605},
            {0x061C, 0x061C},
            {0x06DD, 0x06DD},
            {0x070F, 0x070F},
            {0x0890, 0x0891},
            {0x08E2, 0x08E2},
            {0x180E, 0x180E},
            {0x200B, 0x200F},
            {0x2028, 0x202E},
            {0x2060, 0x2064},
            {0x2066, 0x206F},
            {0xFEFF, 0xFEFF},
            {0xFFF9, 0xFFFB},
            {0x110BD, 0x110BD},
            {0x110CD, 0x110CD},
            {0x13430, 0x1343F},
            {0x1BCA0, 0x1BCA3},
            {0x1D173, 0x1D17A},
            {0xE0001, 0xE0001},
            {0xE0020, 0xE007F},
    }};
    // Ranges are sorted in ascending order, allowing early exit once utf8 precedes r.first
    for (const auto& r : ranges) {
        if (utf8 < r.first) {
            return false;
        }
        if (utf8 <= r.second) {
            return true;
        }
    }
    return false;
}

bool SkUnicodeHardCodedCharProperties::isWhitespace(SkUnichar unichar) {
    // Include ASCII information separators U+001C..U+001F (FS..US) to match u_isWhitespace
    static constexpr std::array<SkUnichar, 25> whitespaces {
            0x0009, // character tabulation
            0x000A, // line feed
            0x000B, // line tabulation
            0x000C, // form feed
            0x000D, // carriage return
            0x001C, // file separator
            0x001D, // group separator
            0x001E, // record separator
            0x001F, // unit separator
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
            0x3000};// ideographic space
    return std::find(whitespaces.begin(), whitespaces.end(), unichar) != whitespaces.end();
}

bool SkUnicodeHardCodedCharProperties::isSpace(SkUnichar unichar) {
    // Include ASCII information separators U+001C..U+001F (FS..US) to match u_isspace
    static constexpr std::array<SkUnichar, 29> spaces {
            0x0009, // character tabulation
            0x000A, // line feed
            0x000B, // line tabulation
            0x000C, // form feed
            0x000D, // carriage return
            0x001C, // file separator
            0x001D, // group separator
            0x001E, // record separator
            0x001F, // unit separator
            0x0020, // space
            0x0085, // next line
            0x00A0, // no-break space
            0x1680, // ogham space mark
            0x2000, // en quad
            0x2001, // em quad
            0x2002, // en space
            0x2003, // em space
            0x2004, // three-per-em space
            0x2005, // four-per-em space
            0x2006, // six-per-em space
            0x2007, // figure space
            0x2008, // punctuation space
            0x2009, // thin space
            0x200A, // hair space
            0x2028, // line separator
            0x2029, // paragraph separator
            0x202F, // narrow no-break space
            0x205F, // medium mathematical space
            0x3000}; // ideographic space
    return std::find(spaces.begin(), spaces.end(), unichar) != spaces.end();
}

bool SkUnicodeHardCodedCharProperties::isTabulation(SkUnichar utf8) {
    return utf8 == '\t';
}

bool SkUnicodeHardCodedCharProperties::isHardBreak(SkUnichar utf8) {
    // Match UAX #14 LF (U+000A) and BK (U+000B, U+000C, U+2028, U+2029) mandatory line breaks
    return utf8 == '\n' || utf8 == '\v' || utf8 == '\f' ||
           utf8 == u'\u2028' || utf8 == u'\u2029';
}

bool SkUnicodeHardCodedCharProperties::IsDefaultIgnorable(SkUnichar utf8) {
    // Match UCHAR_DEFAULT_IGNORABLE_CODE_POINT across all Unicode code points
    static constexpr std::array<std::pair<SkUnichar, SkUnichar>, 17> ranges {{
            {0x00AD, 0x00AD},
            {0x034F, 0x034F},
            {0x061C, 0x061C},
            {0x115F, 0x1160},
            {0x17B4, 0x17B5},
            {0x180B, 0x180F},
            {0x200B, 0x200F},
            {0x202A, 0x202E},
            {0x2060, 0x206F},
            {0x3164, 0x3164},
            {0xFE00, 0xFE0F},
            {0xFEFF, 0xFEFF},
            {0xFFA0, 0xFFA0},
            {0xFFF0, 0xFFF8},
            {0x1BCA0, 0x1BCA3},
            {0x1D173, 0x1D17A},
            {0xE0000, 0xE0FFF},
    }};
    auto it = std::ranges::lower_bound(
            ranges, utf8, {}, &std::pair<SkUnichar, SkUnichar>::second);
    return it != ranges.end() && utf8 >= it->first;
}

bool SkUnicodeHardCodedCharProperties::isDefaultIgnorable(SkUnichar utf8) {
    return IsDefaultIgnorable(utf8);
}

bool SkUnicodeHardCodedCharProperties::IsZeroWidthCodepoint(SkUnichar utf8) {
    // Default-ignorable format characters and non-spacing combining marks take no advance of their
    // own, so a fallback .notdef box is not widened for them
    static constexpr std::array<std::pair<SkUnichar, SkUnichar>, 13> ranges {{
            {0x00AD, 0x00AD},
            {0x0300, 0x036F},
            {0x061C, 0x061C},
            {0x1AB0, 0x1AFF},
            {0x1DC0, 0x1DFF},
            {0x200B, 0x200F},
            {0x202A, 0x202E},
            {0x2060, 0x2069},
            {0x20D0, 0x20FF},
            {0xFE00, 0xFE0F},
            {0xFE20, 0xFE2F},
            {0xFEFF, 0xFEFF},
            {0xE0100, 0xE01EF},
    }};
    auto it = std::ranges::lower_bound(
            ranges, utf8, {}, &std::pair<SkUnichar, SkUnichar>::second);
    return it != ranges.end() && utf8 >= it->first;
}

SkFourByteTag SkUnicodeHardCodedCharProperties::ResolveCodepointScript(SkUnichar c) {
    // Map Unicode script blocks used for OpenType/CoreText script itemization; return 0 for
    // Common and Inherited code points so they inherit the surrounding run's script
    if ((c >= 0x0600 && c <= 0x06FF) ||
        (c >= 0x0750 && c <= 0x077F) ||
        (c >= 0x08A0 && c <= 0x08FF) ||
        (c >= 0xFB50 && c <= 0xFDFF) ||
        (c >= 0xFE70 && c <= 0xFEFF)) {
        return SkSetFourByteTag('A', 'r', 'a', 'b');
    }
    if ((c >= 0x0700 && c <= 0x074F) ||
        (c >= 0x0860 && c <= 0x086F)) {
        return SkSetFourByteTag('S', 'y', 'r', 'c');
    }
    if (c >= 0x10D00 && c <= 0x10D3F) {
        return SkSetFourByteTag('R', 'o', 'h', 'g');
    }
    if (c >= 0x0840 && c <= 0x085F) {
        return SkSetFourByteTag('M', 'a', 'n', 'd');
    }
    if ((c >= 0x1800 && c <= 0x18AF) ||
        (c >= 0x11660 && c <= 0x1167F)) {
        return SkSetFourByteTag('M', 'o', 'n', 'g');
    }
    if (c >= 0x07C0 && c <= 0x07FF) {
        return SkSetFourByteTag('N', 'k', 'o', 'o');
    }
    if (c >= 0xA840 && c <= 0xA87F) {
        return SkSetFourByteTag('P', 'h', 'a', 'g');
    }
    if ((c >= 0x0590 && c <= 0x05FF) ||
        (c >= 0xFB1D && c <= 0xFB4F)) {
        return SkSetFourByteTag('H', 'e', 'b', 'r');
    }
    if ((c >= 0x0400 && c <= 0x052F) ||
        (c >= 0x2DE0 && c <= 0x2DFF) ||
        (c >= 0xA640 && c <= 0xA69F)) {
        return SkSetFourByteTag('C', 'y', 'r', 'l');
    }
    if ((c >= 0x0370 && c <= 0x03FF) ||
        (c >= 0x1F00 && c <= 0x1FFF)) {
        return SkSetFourByteTag('G', 'r', 'e', 'k');
    }
    if ((c >= 0x0900 && c <= 0x097F) ||
        (c >= 0xA8E0 && c <= 0xA8FF)) {
        return SkSetFourByteTag('D', 'e', 'v', 'a');
    }
    if (c >= 0x0E00 && c <= 0x0E7F) {
        return SkSetFourByteTag('T', 'h', 'a', 'i');
    }
    if ((c >= 0x4E00 && c <= 0x9FFF) ||
        (c >= 0x3400 && c <= 0x4DBF) ||
        (c >= 0xF900 && c <= 0xFAFF) ||
        (c >= 0x20000 && c <= 0x2EBEF)) {
        return SkSetFourByteTag('H', 'a', 'n', 'i');
    }
    if (c >= 0x3040 && c <= 0x309F) {
        return SkSetFourByteTag('H', 'i', 'r', 'a');
    }
    if ((c >= 0x30A0 && c <= 0x30FF) ||
        (c >= 0x31F0 && c <= 0x31FF)) {
        return SkSetFourByteTag('K', 'a', 'n', 'a');
    }
    if ((c >= 0xAC00 && c <= 0xD7AF) ||
        (c >= 0x1100 && c <= 0x11FF) ||
        (c >= 0x3130 && c <= 0x318F)) {
        return SkSetFourByteTag('H', 'a', 'n', 'g');
    }
    if ((c >= 0x10A0 && c <= 0x10FF) ||
        (c >= 0x2D00 && c <= 0x2D2F)) {
        return SkSetFourByteTag('G', 'e', 'o', 'r');
    }
    if (c >= 0x0530 && c <= 0x058F) {
        return SkSetFourByteTag('A', 'r', 'm', 'n');
    }
    if ((c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z') ||
        (c >= 0x00C0 && c <= 0x02AF) ||
        (c >= 0x1E00 && c <= 0x1EFF)) {
        return SkSetFourByteTag('L', 'a', 't', 'n');
    }
    return 0;
}

bool SkUnicodeHardCodedCharProperties::isEmoji(SkUnichar unichar) {
    if (unichar == '#' || unichar == '*' || (unichar >= '0' && unichar <= '9')) {
        return true;
    }
    static constexpr std::pair<SkUnichar, SkUnichar> emojiRanges[] = {
        {0x203C, 0x203C}, {0x2049, 0x2049}, {0x2122, 0x2122}, {0x2139, 0x2139},
        {0x2194, 0x2199}, {0x21A9, 0x21AA}, {0x2300, 0x23FF}, {0x25AA, 0x25AB},
        {0x25B6, 0x25B6}, {0x25C0, 0x25C0}, {0x25FB, 0x25FE}, {0x2600, 0x27BF},
        {0x2934, 0x2935}, {0x2B05, 0x2B07}, {0x2B1B, 0x2B1C}, {0x2B50, 0x2B50},
        {0x2B55, 0x2B55}, {0x3030, 0x3030}, {0x303D, 0x303D}, {0x3297, 0x3297},
        {0x3299, 0x3299}, {0x1F000, 0x1F02F}, {0x1F0A0, 0x1F0FF}, {0x1F100, 0x1F64F},
        {0x1F680, 0x1F6FF}, {0x1F700, 0x1F77F}, {0x1F780, 0x1F7FF}, {0x1F800, 0x1F8FF},
        {0x1F900, 0x1FAFF}
    };
    for (const auto& r : emojiRanges) {
        if (unichar >= r.first && unichar <= r.second) {
            return true;
        }
    }
    return false;
}

bool SkUnicodeHardCodedCharProperties::isEmojiComponent(SkUnichar unichar) {
    return (unichar >= 0x1F3FB && unichar <= 0x1F3FF) ||
           (unichar >= 0x1F1E6 && unichar <= 0x1F1FF) ||
           unichar == 0x200D || unichar == 0xFE0F;
}

bool SkUnicodeHardCodedCharProperties::isEmojiModifier(SkUnichar unichar) {
    return unichar >= 0x1F3FB && unichar <= 0x1F3FF;
}

bool SkUnicodeHardCodedCharProperties::isEmojiModifierBase(SkUnichar unichar) {
    return this->isEmoji(unichar) && !this->isEmojiModifier(unichar);
}

bool SkUnicodeHardCodedCharProperties::isRegionalIndicator(SkUnichar unichar) {
    return unichar >= 0x1F1E6 && unichar <= 0x1F1FF;
}

bool SkUnicodeHardCodedCharProperties::isIdeographic(SkUnichar unichar) {
    static constexpr std::array<std::pair<SkUnichar, SkUnichar>, 8> ranges {{
          {4352,   4607}, // Hangul Jamo
          {11904, 42191}, // CJK_Radicals
          {43072, 43135}, // Phags_Pa
          {44032, 55215}, // Hangul_Syllables
          {63744, 64255}, // CJK_Compatibility_Ideographs
          {65072, 65103}, // CJK_Compatibility_Forms
          {65381, 65500}, // Katakana_Hangul_Halfwidth
          {131072, 196607}// Supplementary_Ideographic_Plane
    }};
    for (auto range : ranges) {
        // Block upper bounds in ranges are inclusive codepoints (e.g. U+11FF, U+D7AF)
        if (range.first <= unichar && range.second >= unichar) {
            return true;
        }
    }
    return false;
}
