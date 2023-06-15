/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skunicode/include/SkUnicode.h"

#include "include/private/SkBitmaskEnum.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTemplates.h"
#include <utility>
#include <array>

using namespace skia_private;

std::unique_ptr<SkUnicode> SkUnicode::Make() {
#ifdef SK_UNICODE_ICU_IMPLEMENTATION
    std::unique_ptr<SkUnicode> unicode = SkUnicode::MakeIcuBasedUnicode();
    if (unicode) {
        return unicode;
    }
#endif
    return nullptr;
}

std::unique_ptr<SkUnicode> MakeClientBasedUnicode(
        SkSpan<char> text,
        std::vector<SkUnicode::Position> words,
        std::vector<SkUnicode::Position> graphemeBreaks,
        std::vector<SkUnicode::LineBreakBefore> lineBreaks) {
#ifdef SK_UNICODE_CLIENT_IMPLEMENTATION
    std::unique_ptr<SkUnicode> unicode =
            SkUnicode::MakeClientBasedUnicode(text, words, graphemeBreaks, lineBreaks);
    if (unicode) {
        return unicode;
    }
#endif
    return nullptr;
}

SkString SkUnicode::convertUtf16ToUtf8(const char16_t* utf16, int utf16Units) {

    int utf8Units = SkUTF::UTF16ToUTF8(nullptr, 0, (uint16_t*)utf16, utf16Units);
    if (utf8Units < 0) {
        SkDEBUGF("Convert error: Invalid utf16 input");
        return SkString();
    }
    AutoTArray<char> utf8(utf8Units);
    SkDEBUGCODE(int dstLen =) SkUTF::UTF16ToUTF8(utf8.data(), utf8Units, (uint16_t*)utf16, utf16Units);
    SkASSERT(dstLen == utf8Units);

    return SkString(utf8.data(), utf8Units);
}

SkString SkUnicode::convertUtf16ToUtf8(const std::u16string& utf16) {
    return convertUtf16ToUtf8(utf16.c_str(), utf16.size());
}

std::u16string SkUnicode::convertUtf8ToUtf16(const char* utf8, int utf8Units) {

    int utf16Units = SkUTF::UTF8ToUTF16(nullptr, 0, utf8, utf8Units);
    if (utf16Units < 0) {
        SkDEBUGF("Convert error: Invalid utf8 input");
        return std::u16string();
    }

    AutoTArray<uint16_t> utf16(utf16Units);
    SkDEBUGCODE(int dstLen =) SkUTF::UTF8ToUTF16(utf16.data(), utf16Units, utf8, utf8Units);
    SkASSERT(dstLen == utf16Units);

    return std::u16string((char16_t *)utf16.data(), utf16Units);
}

std::u16string SkUnicode::convertUtf8ToUtf16(const SkString& utf8) {
    return convertUtf8ToUtf16(utf8.c_str(), utf8.size());
}

bool SkUnicode::hasTabulationFlag(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kTabulation) == SkUnicode::kTabulation;
}

bool SkUnicode::hasHardLineBreakFlag(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kHardLineBreakBefore) == SkUnicode::kHardLineBreakBefore;
}

bool SkUnicode::hasSoftLineBreakFlag(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kSoftLineBreakBefore) == SkUnicode::kSoftLineBreakBefore;
}

bool SkUnicode::hasGraphemeStartFlag(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kGraphemeStart) == SkUnicode::kGraphemeStart;
}

bool SkUnicode::hasControlFlag(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kControl) == SkUnicode::kControl;
}

bool SkUnicode::hasPartOfWhiteSpaceBreakFlag(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kPartOfWhiteSpaceBreak) == SkUnicode::kPartOfWhiteSpaceBreak;
}

bool SkUnicode::isControl(SkUnichar utf8) {
    return (utf8 < ' ') || (utf8 >= 0x7f && utf8 <= 0x9f) ||
           (utf8 >= 0x200D && utf8 <= 0x200F) ||
           (utf8 >= 0x202A && utf8 <= 0x202E);
}

bool SkUnicode::isWhitespace(SkUnichar unichar) {
    static constexpr std::array<SkUnichar, 21> whitespaces {
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
            0x3000};// ideographic space
    return std::find(whitespaces.begin(), whitespaces.end(), unichar) != whitespaces.end();
}

bool SkUnicode::isSpace(SkUnichar unichar) {
    static constexpr std::array<SkUnichar, 25> spaces {
            0x0009, // character tabulation
            0x000A, // line feed
            0x000B, // line tabulation
            0x000C, // form feed
            0x000D, // carriage return
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

bool SkUnicode::isTabulation(SkUnichar utf8) {
    return utf8 == '\t';
}

bool SkUnicode::isHardBreak(SkUnichar utf8) {
    return utf8 == '\n';
}

bool SkUnicode::isEmoji(SkUnichar unichar) {
    return 0x1F300 <= unichar && unichar <= 0x1FB00;
}

bool SkUnicode::isIdeographic(SkUnichar unichar) {
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
        if (range.first <= unichar && range.second > unichar) {
            return true;
        }
    }
    return false;
}

