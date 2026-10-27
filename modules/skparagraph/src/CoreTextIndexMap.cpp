// Copyright 2026 Google LLC
#include "modules/skparagraph/src/CoreTextIndexMap.h"

#include "include/private/SkTo.h"

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

void CoreTextIndexMap::start(size_t utf8Bytes) {
    fUtf8Length = utf8Bytes;
    fUtf16Length = 0;
    // A scalar never takes more UTF-16 units than UTF-8 bytes, so utf8Bytes + 1 bounds both
    fUtf8ToUtf16.reset(SkToInt(fUtf8Length + 1));
    fUtf16ToUtf8.reset(SkToInt(fUtf8Length + 1));
}

void CoreTextIndexMap::finish() {
    fUtf8ToUtf16[SkToInt(fUtf8Length)] = SkTo<uint32_t>(fUtf16Length);
    fUtf16ToUtf8[SkToInt(fUtf16Length)] = SkTo<uint32_t>(fUtf8Length);
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
    // Both callers pass non-empty clipped style spans (addStyle) or their intersections with
    // script runs; toUtf16Index is monotonic, so start <= end keeps the length non-negative
    SkASSERT(utf8Range.start <= utf8Range.end);
    const size_t loc = this->toUtf16Index(utf8Range.start);
    const size_t end = this->toUtf16Index(utf8Range.end);
    return CFRangeMake(SkTo<CFIndex>(loc), SkTo<CFIndex>(end - loc));
}

TextRange CoreTextIndexMap::cfrangeToTextRange(CFRange cfRange) const {
    // The only caller passes CTRunGetStringRange ranges, which lie inside the string;
    // toUtf8Index is monotonic, so the UTF-8 end never precedes the start
    SkASSERT(cfRange.location >= 0 && cfRange.length >= 0);
    const size_t start = this->toUtf8Index(SkTo<size_t>(cfRange.location));
    // CoreText CTRun string indices reference UTF-16 code units; resolve exclusive end through next
    // scalar boundary
    const size_t end = this->toUtf8Index(SkTo<size_t>(cfRange.location + cfRange.length));
    return TextRange(start, end);
}

}  // namespace textlayout
}  // namespace skia
