// Copyright 2026 Google LLC
#ifndef CoreTextIndexMap_DEFINED
#define CoreTextIndexMap_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"

#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
#include "src/core/SkUTF.h"

#include <CoreFoundation/CoreFoundation.h>

namespace skia {
namespace textlayout {

// Bidirectional UTF-8 byte offset <-> UTF-16 code unit mapping for CoreText interop
// Stack buffer avoids heap allocations on layout hot paths for lines under 256 code units
class CoreTextIndexMap {
public:
    CoreTextIndexMap();
    CoreTextIndexMap(const char* utf8, size_t utf8Bytes);

    void reset(const char* utf8, size_t utf8Bytes);
    // The same mapping, built in the one decode pass that also calls
    // onCodepoint(unichar, utf8Start, utf16Start) per scalar, an invalid byte being one U+FFFD
    template <typename OnCodepoint>
    void reset(const char* utf8, size_t utf8Bytes, OnCodepoint&& onCodepoint);

    size_t toUtf16Index(size_t utf8Index) const;
    // CoreText uses UTF-16 code units; intra-codepoint offsets resolve to leading scalar byte
    size_t toUtf8Index(size_t utf16Index) const;

    CFRange utf8RangeToCFRange(TextRange utf8Range) const;
    // Converts half-open UTF-16 CFRange to half-open UTF-8 TextRange for ParagraphImpl
    TextRange cfrangeToTextRange(CFRange cfRange) const;

private:
    size_t fUtf8Length;
    size_t fUtf16Length;
    skia_private::AutoSTArray<256, uint32_t> fUtf8ToUtf16;
    skia_private::AutoSTArray<256, uint32_t> fUtf16ToUtf8;
};

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

#endif  // defined(SK_PARAGRAPH_NATIVE_CORETEXT)

#endif  // CoreTextIndexMap_DEFINED
