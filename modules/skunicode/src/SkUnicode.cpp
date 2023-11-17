/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTemplates.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/base/SkBitmaskEnum.h"

using namespace skia_private;

namespace {
static inline SkUnichar nextUtf8Unit(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}
}

std::unique_ptr<SkUnicode> SkUnicode::Make() {
    std::unique_ptr<SkUnicode> unicode;
#ifdef SK_UNICODE_ICU_IMPLEMENTATION
    unicode = SkUnicode::MakeIcuBasedUnicode();
    if (unicode) {
        return unicode;
    }
#endif
#ifdef SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION
    unicode = SkUnicode::MakeLibgraphemeBasedUnicode();
    if (unicode) {
        return unicode;
    }
#endif
#ifdef SK_UNICODE_ICU4X_IMPLEMENTATION
    unicode = SkUnicode::MakeIcu4xBasedUnicode();
    if (unicode) {
        return unicode;
    }
#endif
    return nullptr;
}

std::unique_ptr<SkUnicode> MakeClientBasedUnicode(
        SkSpan<char> text,
        std::vector<SkUnicode::Position> words,               // NOLINT(performance-unnecessary-value-param)
        std::vector<SkUnicode::Position> graphemeBreaks,      // NOLINT(performance-unnecessary-value-param)
        std::vector<SkUnicode::LineBreakBefore> lineBreaks) { // NOLINT(performance-unnecessary-value-param)
#ifdef SK_UNICODE_CLIENT_IMPLEMENTATION
    std::unique_ptr<SkUnicode> unicode = SkUnicode::MakeClientBasedUnicode(
            text, std::move(words), std::move(graphemeBreaks), std::move(lineBreaks));
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

std::vector<SkUnichar> SkUnicode::getEmojiSequence(const char** begin, const char* end) {
    std::vector<SkUnichar> codepoints;
    if (*begin == end) {
        return codepoints;
    }
    const char* next = *begin;
    auto unicode = nextUtf8Unit(&next, end);
    if (!this->isEmojiBase(unicode)) {
        // This is not a basic emoji nor it an emoji sequence
        return codepoints;
    }

    if (!this->isEmojiComponent(unicode)) {
        // This is the basic emoji case
        codepoints.emplace_back(unicode);
        *begin = next;
        return codepoints;
    } else if (next == end) {
        // It could be an emoji sequence but it's not (ended too soon)
        return codepoints;
    }

    codepoints.emplace_back(unicode);
    while (next != end) {
        auto last = next;
        auto unicode = nextUtf8Unit(&next, end);
        if (!this->isEmojiBase(unicode) && this->isEmojiComponent(unicode)) {
            // Keep adding emoji components to our emoji sequence
            codepoints.emplace_back(unicode);
        } else {
            // This is no a part of the emoji sequence
            if (codepoints.size() < 2) {
                // There is no emoji sequence at all
                codepoints.clear();
            } else {
                // This is the end of the emoji sequence
                *begin = last;
            }
            return codepoints;
        }
    }
    *begin = end;
    return codepoints;
}