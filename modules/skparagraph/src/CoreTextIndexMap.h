// Copyright 2026 Google LLC
#ifndef CoreTextIndexMap_DEFINED
#define CoreTextIndexMap_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/TextStyle.h"

#include "src/core/SkUTF.h"

#include <CoreFoundation/CoreFoundation.h>

namespace skia {
namespace textlayout {

// Bidirectional UTF-8 byte offset <-> UTF-16 code unit mapping for CoreText interop
// Stack buffer avoids heap allocations on layout hot paths for lines under 256 bytes
// TODO(jlavrova: 1406897): Fold malformed-UTF8 handling into
// SkUnicode::extractUtfConversionMapping once CoreText and SkUnicode callers are unified
class CoreTextIndexMap {
public:
    CoreTextIndexMap() : fUtf8Length(0), fUtf16Length(0) {
        fUtf8ToUtf16.reset(1);
        fUtf8ToUtf16[0] = 0;
        fUtf16ToUtf8.reset(1);
        fUtf16ToUtf8[0] = 0;
    }

    // Returns the scalar that ptr points at and moves ptr past it; an invalid or truncated
    // sequence gives one U+FFFD and moves ptr by exactly one byte. ptr must point before end.
    // Every decode loop over text this map indexes uses it, so all count invalid bytes alike
    static SkUnichar NextScalar(const char** ptr, const char* end) {
        SkASSERT(*ptr < end);
        const char* start = *ptr;
        SkUnichar u = SkUTF::NextUTF8(ptr, end);
        // SkUTF::NextUTF8 jumps to end on an error; one byte keeps the rest of the text decodable
        if (u < 0) {
            *ptr = start + 1;
            return 0xFFFD;
        }
        return u;
    }

    // Discards the mapping and sizes both tables for utf8Bytes + 1 entries, an upper bound on
    // both lengths, so the heap is used from 256 bytes up. The caller decodes the text itself
    // with NextScalar: start(), append() per scalar in text order, then finish()
    void start(size_t utf8Bytes) {
        fUtf8Length = utf8Bytes;
        fUtf16Length = 0;
        // A scalar never takes more UTF-16 units than UTF-8 bytes, so utf8Bytes + 1 bounds both
        fUtf8ToUtf16.reset(SkToInt(fUtf8Length + 1));
        fUtf16ToUtf8.reset(SkToInt(fUtf8Length + 1));
    }

    // Maps the scalar at [utf8Start, utf8End), right after the previous one, to the next UTF-16
    // position (two units above U+FFFF) and returns that position
    size_t append(size_t utf8Start, size_t utf8End, SkUnichar unichar) {
        SkASSERT(utf8Start < utf8End && utf8End <= fUtf8Length);
        size_t u16Pos = fUtf16Length;
        // Map trailing UTF-8 bytes to the first UTF-16 unit of the scalar value
        for (size_t i = utf8Start; i < utf8End; ++i) {
            fUtf8ToUtf16[SkToInt(i)] = SkTo<uint32_t>(u16Pos);
        }
        // Supplementary code points take a surrogate pair; both units map back to the first byte
        fUtf16ToUtf8[SkToInt(u16Pos)] = SkTo<uint32_t>(utf8Start);
        fUtf16Length = u16Pos + 1;
        if (unichar >= 0x10000) {
            fUtf16ToUtf8[SkToInt(u16Pos + 1)] = SkTo<uint32_t>(utf8Start);
            ++fUtf16Length;
        }
        return u16Pos;
    }

    // Writes the end entries of both tables; the mapping is complete after it
    void finish() {
        fUtf8ToUtf16[SkToInt(fUtf8Length)] = SkTo<uint32_t>(fUtf16Length);
        fUtf16ToUtf8[SkToInt(fUtf16Length)] = SkTo<uint32_t>(fUtf8Length);
    }

    // Returns the UTF-16 index of the scalar that holds utf8Index: a byte inside a scalar maps
    // to its first code unit, an index at or past the end to the UTF-16 length
    size_t toUtf16Index(size_t utf8Index) const {
        if (utf8Index >= fUtf8Length) {
            return fUtf16Length;
        }
        return fUtf8ToUtf16[SkTo<int>(utf8Index)];
    }

    // CoreText uses UTF-16 code units; intra-codepoint offsets resolve to leading scalar byte
    size_t toUtf8Index(size_t utf16Index) const {
        if (utf16Index >= fUtf16Length) {
            return fUtf8Length;
        }
        return fUtf16ToUtf8[SkTo<int>(utf16Index)];
    }

    // Converts a half-open UTF-8 TextRange, start <= end, to the half-open UTF-16 CFRange
    // CoreText takes; a range inside one scalar gives an empty CFRange
    CFRange utf8RangeToCFRange(TextRange utf8Range) const {
        // Both callers pass non-empty clipped style spans (addStyle) or their intersections with
        // script runs; toUtf16Index is monotonic, so start <= end keeps the length non-negative
        SkASSERT(utf8Range.start <= utf8Range.end);
        size_t loc = this->toUtf16Index(utf8Range.start);
        size_t end = this->toUtf16Index(utf8Range.end);
        return CFRangeMake(SkTo<CFIndex>(loc), SkTo<CFIndex>(end - loc));
    }

    // Converts a half-open UTF-16 CFRange with non-negative location and length to the
    // half-open UTF-8 TextRange for ParagraphImpl
    TextRange cfrangeToTextRange(CFRange cfRange) const {
        // The only caller passes CTRunGetStringRange ranges, which lie inside the string;
        // toUtf8Index is monotonic, so the UTF-8 end never precedes the start
        SkASSERT(cfRange.location >= 0 && cfRange.length >= 0);
        size_t start = this->toUtf8Index(SkTo<size_t>(cfRange.location));
        size_t end = this->toUtf8Index(SkTo<size_t>(cfRange.location + cfRange.length));
        return TextRange(start, end);
    }

private:
    size_t fUtf8Length;
    size_t fUtf16Length;
    skia_private::AutoSTArray<256, uint32_t> fUtf8ToUtf16;
    skia_private::AutoSTArray<256, uint32_t> fUtf16ToUtf8;
};

}  // namespace textlayout
}  // namespace skia

#endif  // CoreTextIndexMap_DEFINED
