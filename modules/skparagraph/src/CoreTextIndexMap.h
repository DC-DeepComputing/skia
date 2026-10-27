// Copyright 2026 Google LLC
#ifndef CoreTextIndexMap_DEFINED
#define CoreTextIndexMap_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
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
    CoreTextIndexMap();
    CoreTextIndexMap(const char* utf8, size_t utf8Bytes);

    // Replaces the mapping with one for utf8[0, utf8Bytes); a null utf8 gives the empty map
    // and each invalid byte counts as one U+FFFD
    void reset(const char* utf8, size_t utf8Bytes);
    // The same mapping as reset(utf8, utf8Bytes), and in its one decode pass also calls
    // onCodepoint(unichar, utf8Start, utf16Start) once per scalar, in text order
    template <typename OnCodepoint>
    void reset(const char* utf8, size_t utf8Bytes, OnCodepoint&& onCodepoint);

    // Returns the UTF-16 index of the scalar that holds utf8Index: a byte inside a scalar maps
    // to its first code unit, an index at or past the end to the UTF-16 length
    size_t toUtf16Index(size_t utf8Index) const;
    // CoreText uses UTF-16 code units; intra-codepoint offsets resolve to leading scalar byte
    size_t toUtf8Index(size_t utf16Index) const;

    // Converts a half-open UTF-8 TextRange to the half-open UTF-16 CFRange CoreText takes;
    // an inverted range gives an empty CFRange at its start
    CFRange utf8RangeToCFRange(TextRange utf8Range) const;
    // Converts half-open UTF-16 CFRange to half-open UTF-8 TextRange for ParagraphImpl
    TextRange cfrangeToTextRange(CFRange cfRange) const;

private:
    size_t fUtf8Length;
    size_t fUtf16Length;
    skia_private::AutoSTArray<256, uint32_t> fUtf8ToUtf16;
    skia_private::AutoSTArray<256, uint32_t> fUtf16ToUtf8;
};

// Builds both tables in one decode pass over utf8[0, utf8Bytes), replacing the old mapping (a
// null utf8 gives the empty map). Each invalid byte becomes one U+FFFD. onCodepoint(unichar,
// utf8Start, utf16Start) runs once per scalar, in text order, before that scalar's entries are
// written. Both tables get utf8Bytes + 1 entries, so the heap is used from 256 bytes up
template <typename OnCodepoint>
void CoreTextIndexMap::reset(const char* utf8, size_t utf8Bytes, OnCodepoint&& onCodepoint) {
    fUtf8Length = utf8 ? utf8Bytes : 0;
    // A scalar never takes more UTF-16 units than UTF-8 bytes, so utf8Bytes + 1 bounds both
    fUtf8ToUtf16.reset(SkToInt(fUtf8Length + 1));
    fUtf16ToUtf8.reset(SkToInt(fUtf8Length + 1));
    const char* p = utf8;
    const char* const end = utf8 + fUtf8Length;
    size_t u16Pos = 0;
    // Monotonically advancing pointer guarantees loop termination on malformed UTF-8 sequences
    while (p < end) {
        const char* prev = p;
        SkUnichar u = SkUTF::NextUTF8(&p, end);
        if (u < 0) {
            p = prev + 1;
            u = 0xFFFD;
        }
        const size_t u8Start = prev - utf8;
        const size_t u8End = p - utf8;
        // Supplementary code points above U+FFFF require surrogate pairs in UTF-16
        const size_t u16Units = u >= 0x10000 ? 2 : 1;
        onCodepoint(u, u8Start, u16Pos);
        // Map trailing UTF-8 bytes to the first UTF-16 unit of the scalar value
        for (size_t i = u8Start; i < u8End; ++i) {
            fUtf8ToUtf16[SkToInt(i)] = SkTo<uint32_t>(u16Pos);
        }
        // Map both surrogate code units back to the first byte of the UTF-8 sequence
        for (size_t i = 0; i < u16Units; ++i) {
            fUtf16ToUtf8[SkToInt(u16Pos + i)] = SkTo<uint32_t>(u8Start);
        }
        u16Pos += u16Units;
    }
    fUtf16Length = u16Pos;
    fUtf8ToUtf16[SkToInt(fUtf8Length)] = SkTo<uint32_t>(fUtf16Length);
    fUtf16ToUtf8[SkToInt(fUtf16Length)] = SkTo<uint32_t>(fUtf8Length);
}

}  // namespace textlayout
}  // namespace skia

#endif  // CoreTextIndexMap_DEFINED
