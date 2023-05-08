/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkBitmaskEnum.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTemplates.h"
#include "modules/skunicode/include/SkUnicode.h"
#include <algorithm>
#include <array>

using namespace skia_private;

std::unique_ptr<SkUnicode> SkUnicode::Make() {
#ifdef SK_UNICODE_ICU_IMPLEMENTATION
    std::unique_ptr<SkUnicode> unicode = SkUnicode::MakeIcuBasedUnicode();
    if (unicode) {
        return unicode;
    }
#endif
#ifdef SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION
    std::unique_ptr<SkUnicode> unicode = SkUnicode::MakeLibgraphemeBasedUnicode();
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

bool SkUnicode::isControl(SkUnichar codePoint) {
    return (codePoint < ' ') || (codePoint >= 0x7f && codePoint <= 0x9f) ||
           (codePoint >= 0x200D && codePoint <= 0x200F) ||
           (codePoint >= 0x202A && codePoint <= 0x202E);
}

bool SkUnicode::isWhitespace(SkUnichar codePoint) {
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
    return std::find(whitespaces.begin(), whitespaces.end(), codePoint) != whitespaces.end();
}

bool SkUnicode::isSpace(SkUnichar codePoint) {
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
    return std::find(spaces.begin(), spaces.end(), codePoint) != spaces.end();
}

bool SkUnicode::isTabulation(SkUnichar codePoint) {
    return codePoint == '\t';
}

bool SkUnicode::isHardBreak(SkUnichar codePoint) {
    return codePoint == '\n';
}

bool SkUnicode::isEmoji(SkUnichar codePoint) {
    return (codePoint >= 0x1F600 && codePoint <= 0x1F64F);
}
