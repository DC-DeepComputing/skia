/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <unicode/ubidi.h>
#include "modules/skunicode/include/SkUnicode.h"

#include "include/private/SkBitmaskEnum.h"
#include "include/private/base/SkTemplates.h"

using namespace skia_private;

static void ubidi_close_wrapper(UBiDi* bidi) {
    ubidi_close(bidi);
}

using SkUnicodeBidi = std::unique_ptr<UBiDi, SkFunctionObject<ubidi_close_wrapper>>;

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

class SkBidiIterator_common : public SkBidiIterator {
    SkUnicodeBidi fBidi;
public:
    explicit SkBidiIterator_common(SkUnicodeBidi bidi) : fBidi(std::move(bidi)) {}
    Position getLength() override { return ubidi_getLength(fBidi.get()); }
    Level getLevelAt(Position pos) override { return ubidi_getLevelAt(fBidi.get(), pos); }

    static std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t utf16[], int utf16Units, Direction dir) {
        UErrorCode status = U_ZERO_ERROR;
        SkUnicodeBidi bidi(ubidi_openSized(utf16Units, 0, &status));
        if (U_FAILURE(status)) {
            SkDEBUGF("Bidi error: %s", u_errorName(status));
            return nullptr;
        }
        SkASSERT(bidi);
        uint8_t bidiLevel = (dir == SkBidiIterator::kLTR) ? UBIDI_LTR : UBIDI_RTL;
        // The required lifetime of utf16 isn't well documented.
        // It appears it isn't used after ubidi_setPara except through ubidi_getText.
        ubidi_setPara(bidi.get(), (const UChar*)utf16, utf16Units, bidiLevel, nullptr, &status);
        if (U_FAILURE(status)) {
            SkDEBUGF("Bidi error: %s", u_errorName(status));
            return nullptr;
        }
        return std::unique_ptr<SkBidiIterator>(new SkBidiIterator_common(std::move(bidi)));
    }

    // ICU bidi iterator works with utf16 but clients (Flutter for instance) may work with utf8
    // This method allows the clients not to think about all these details
    static std::unique_ptr<SkBidiIterator> makeBidiIterator(const char utf8[], int utf8Units, Direction dir) {
        // Convert utf8 into utf16 since ubidi only accepts utf16
        if (!SkTFitsIn<int32_t>(utf8Units)) {
            SkDEBUGF("Bidi error: text too long");
            return nullptr;
        }

        // Getting the length like this seems to always set U_BUFFER_OVERFLOW_ERROR
        int utf16Units = SkUTF::UTF8ToUTF16(nullptr, 0, utf8, utf8Units);
        if (utf16Units < 0) {
            SkDEBUGF("Bidi error: Invalid utf8 input");
            return nullptr;
        }
        std::unique_ptr<uint16_t[]> utf16(new uint16_t[utf16Units]);
        SkDEBUGCODE(int dstLen =) SkUTF::UTF8ToUTF16(utf16.get(), utf16Units, utf8, utf8Units);
        SkASSERT(dstLen == utf16Units);

        return makeBidiIterator(utf16.get(), utf16Units, dir);
    }
};

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

bool SkUnicode::isTabulation(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kTabulation) == SkUnicode::kTabulation;
}

bool SkUnicode::isHardLineBreak(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kHardLineBreakBefore) == SkUnicode::kHardLineBreakBefore;
}

bool SkUnicode::isSoftLineBreak(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kSoftLineBreakBefore) == SkUnicode::kSoftLineBreakBefore;
}

bool SkUnicode::isGraphemeStart(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kGraphemeStart) == SkUnicode::kGraphemeStart;
}

bool SkUnicode::isControl(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kControl) == SkUnicode::kControl;
}

bool SkUnicode::isPartOfWhiteSpaceBreak(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kPartOfWhiteSpaceBreak) == SkUnicode::kPartOfWhiteSpaceBreak;
}

/** Replaces invalid utf-8 sequences with REPLACEMENT CHARACTER U+FFFD. */
static inline SkUnichar utf8_next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}

bool SkUnicode::extractBidi(const char utf8[],
                                   int utf8Units,
                                   TextDirection dir,
                                   std::vector<BidiRegion>* bidiRegions) {
    // Convert to UTF16 since for now bidi iterator only operates on utf16
    auto utf16 = convertUtf8ToUtf16(utf8, utf8Units);

    // Create bidi iterator
    UErrorCode status = U_ZERO_ERROR;
    SkUnicodeBidi bidi(ubidi_openSized(utf16.size(), 0, &status));
    if (U_FAILURE(status)) {
        SkDEBUGF("Bidi error: %s", u_errorName(status));
        return false;
    }
    SkASSERT(bidi);
    uint8_t bidiLevel = (dir == TextDirection::kLTR) ? UBIDI_LTR : UBIDI_RTL;
    // The required lifetime of utf16 isn't well documented.
    // It appears it isn't used after ubidi_setPara except through ubidi_getText.
    ubidi_setPara(bidi.get(), (const UChar*)utf16.c_str(), utf16.size(), bidiLevel, nullptr,
                        &status);
    if (U_FAILURE(status)) {
        SkDEBUGF("Bidi error: %s", u_errorName(status));
        return false;
    }

    // Iterate through bidi regions and the result positions into utf8
    const char* start8 = utf8;
    const char* end8 = utf8 + utf8Units;
    BidiLevel currentLevel = 0;

    Position pos8 = 0;
    Position pos16 = 0;
    Position end16 = ubidi_getLength(bidi.get());

    if (end16 == 0) {
        return true;
    }
    if (ubidi_getDirection(bidi.get()) != UBIDI_MIXED) {
        // The entire paragraph is unidirectional.
        bidiRegions->emplace_back(0, utf8Units, ubidi_getLevelAt(bidi.get(), 0));
        return true;
    }

    while (pos16 < end16) {
        auto level = ubidi_getLevelAt(bidi.get(), pos16);
        if (pos16 == 0) {
            currentLevel = level;
        } else if (level != currentLevel) {
            Position end = start8 - utf8;
            bidiRegions->emplace_back(pos8, end, currentLevel);
            currentLevel = level;
            pos8 = end;
        }
        SkUnichar u = utf8_next(&start8, end8);
        pos16 += SkUTF::ToUTF16(u);
    }
    Position end = start8 - utf8;
    if (end != pos8) {
        bidiRegions->emplace_back(pos8, end, currentLevel);
    }
    return true;
}

std::unique_ptr<SkBidiIterator> SkUnicode::makeBidiIterator(const uint16_t text[],
                                                            int count,
                                                            SkBidiIterator::Direction dir) {
    return SkBidiIterator_common::makeBidiIterator(text, count, dir);
}
std::unique_ptr<SkBidiIterator> SkUnicode::makeBidiIterator(const char text[],
                                                            int count,
                                                            SkBidiIterator::Direction dir) {
    return SkBidiIterator_common::makeBidiIterator(text, count, dir);
}
