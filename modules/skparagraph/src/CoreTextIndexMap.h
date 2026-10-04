// Copyright 2026 Google LLC
#ifndef CoreTextIndexMap_DEFINED
#define CoreTextIndexMap_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkTemplates.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/TextStyle.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
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

    size_t toUtf16Index(size_t utf8Index) const;
    size_t toUtf8Index(size_t utf16Index) const;

    CFRange utf8RangeToCFRange(TextRange utf8Range) const;
    TextRange cfrangeToTextRange(CFRange cfRange) const;

    size_t utf8Length() const { return fUtf8Length; }
    size_t utf16Length() const { return fUtf16Length; }
    bool empty() const { return fUtf8Length == 0; }

private:
    size_t fUtf8Length;
    size_t fUtf16Length;
    skia_private::AutoSTArray<256, uint32_t> fUtf8ToUtf16;
    skia_private::AutoSTArray<256, uint32_t> fUtf16ToUtf8;
};

}  // namespace textlayout
}  // namespace skia

#endif  // defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#endif  // CoreTextIndexMap_DEFINED
