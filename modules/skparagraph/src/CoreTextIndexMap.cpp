// Copyright 2026 Google LLC
#include "modules/skparagraph/src/CoreTextIndexMap.h"

#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)

#include "include/private/SkTo.h"
#include "src/core/SkUTF.h"

namespace skia {
namespace textlayout {

CoreTextIndexMap::CoreTextIndexMap()
    : fUtf8Length(0)
    , fUtf16Length(0) {
    fUtf8ToUtf16.reset(1);
    fUtf8ToUtf16[0] = 0;
    fUtf16ToUtf8.reset(1);
    fUtf16ToUtf8[0] = 0;
}

CoreTextIndexMap::CoreTextIndexMap(const char* utf8, size_t utf8Bytes)
    : CoreTextIndexMap() {
    this->reset(utf8, utf8Bytes);
}

void CoreTextIndexMap::reset(const char* utf8, size_t utf8Bytes) {
    if (!utf8 || utf8Bytes == 0) {
        fUtf8Length = 0;
        fUtf16Length = 0;
        fUtf8ToUtf16.reset(1);
        fUtf8ToUtf16[0] = 0;
        fUtf16ToUtf8.reset(1);
        fUtf16ToUtf8[0] = 0;
        return;
    }

    fUtf8Length = utf8Bytes;

    size_t u16Len = 0;
    const char* p = utf8;
    const char* const end = utf8 + utf8Bytes;
    // Monotonically advancing pointer guarantees loop termination on malformed UTF-8 sequences
    while (p < end) {
        const char* prev = p;
        SkUnichar u = SkUTF::NextUTF8(&p, end);
        if (u < 0) {
            p = prev + 1;
            u = 0xFFFD;
        }
        SkASSERT(p > prev);
        // Supplementary code points above U+FFFF require surrogate pairs in UTF-16
        u16Len += (u >= 0x10000) ? 2 : 1;
    }

    fUtf16Length = u16Len;
    fUtf8ToUtf16.reset(SkTo<int>(fUtf8Length + 1));
    fUtf16ToUtf8.reset(SkTo<int>(fUtf16Length + 1));

    p = utf8;
    size_t u16Pos = 0;
    while (p < end) {
        const char* prev = p;
        size_t u8Start = prev - utf8;
        size_t u16Start = u16Pos;
        SkUnichar u = SkUTF::NextUTF8(&p, end);
        if (u < 0) {
            p = prev + 1;
            u = 0xFFFD;
        }
        SkASSERT(p > prev);
        size_t u8End = p - utf8;
        size_t u16Units = (u >= 0x10000) ? 2 : 1;

        // Map trailing UTF-8 bytes to the first UTF-16 unit of the scalar value
        for (size_t i = u8Start; i < u8End; ++i) {
            fUtf8ToUtf16[SkTo<int>(i)] = SkTo<uint32_t>(u16Start);
        }
        // Map both surrogate code units back to the first byte of the UTF-8 sequence
        for (size_t i = 0; i < u16Units; ++i) {
            fUtf16ToUtf8[SkTo<int>(u16Start + i)] = SkTo<uint32_t>(u8Start);
        }
        u16Pos += u16Units;
    }

    SkASSERT(u16Pos == fUtf16Length);
    fUtf8ToUtf16[SkTo<int>(fUtf8Length)] = SkTo<uint32_t>(fUtf16Length);
    fUtf16ToUtf8[SkTo<int>(fUtf16Length)] = SkTo<uint32_t>(fUtf8Length);
}

size_t CoreTextIndexMap::toUtf16Index(size_t utf8Index) const {
    if (utf8Index >= fUtf8Length) {
        return fUtf16Length;
    }
    return fUtf8ToUtf16[SkTo<int>(utf8Index)];
}

size_t CoreTextIndexMap::toUtf8Index(size_t utf16Index) const {
    if (utf16Index >= fUtf16Length) {
        return fUtf8Length;
    }
    return fUtf16ToUtf8[SkTo<int>(utf16Index)];
}

CFRange CoreTextIndexMap::utf8RangeToCFRange(TextRange utf8Range) const {
    size_t loc = this->toUtf16Index(utf8Range.start);
    size_t end = this->toUtf16Index(utf8Range.end);
    if (end < loc) {
        end = loc;
    }
    return CFRangeMake(SkTo<CFIndex>(loc), SkTo<CFIndex>(end - loc));
}

TextRange CoreTextIndexMap::cfrangeToTextRange(CFRange cfRange) const {
    if (cfRange.location < 0) {
        return TextRange(0, 0);
    }
    size_t start = this->toUtf8Index(SkTo<size_t>(cfRange.location));
    // CoreText CTRun string indices reference UTF-16 code units; resolve exclusive end through next scalar boundary
    size_t endCf = cfRange.length > 0 ? SkTo<size_t>(cfRange.location + cfRange.length)
                                      : SkTo<size_t>(cfRange.location);
    size_t end = this->toUtf8Index(endCf);
    if (end < start) {
        end = start;
    }
    return TextRange(start, end);
}

}  // namespace textlayout
}  // namespace skia

#endif  // defined(SK_PARAGRAPH_NATIVE_CORETEXT)
