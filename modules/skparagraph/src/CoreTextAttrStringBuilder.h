// Copyright 2026 Google LLC
#ifndef CoreTextAttrStringBuilder_DEFINED
#define CoreTextAttrStringBuilder_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTArray.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/CoreTextFontRegistry.h"
#include "modules/skparagraph/src/CoreTextIndexMap.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#include <CoreText/CoreText.h>

namespace skia {
namespace textlayout {

class CoreTextAttrStringBuilder {
public:
    CoreTextAttrStringBuilder(const char* utf8, size_t utf8Bytes,
                              TextDirection baseDirection,
                              sk_sp<FontCollection> fontCollection);

    void addStyle(TextRange utf8Range, const TextStyle& style);

    SkUniqueCFRef<CFAttributedStringRef> build();

    const CoreTextIndexMap& indexMap() const { return fIndexMap; }

    // Identifies cursive scripts (Arabic, Syriac, NKo) where inter-glyph tracking breaks ligature topology
    static bool isCursiveCodepoint(SkUnichar c);

private:
    const char* fUtf8;
    size_t fUtf8Bytes;
    TextDirection fBaseDirection;
    sk_sp<FontCollection> fFontCollection;
    CoreTextIndexMap fIndexMap;

    struct StyleSpan {
        TextRange fRange;
        TextStyle fStyle;
    };
    skia_private::STArray<8, StyleSpan> fStyles;
};

}  // namespace textlayout
}  // namespace skia

#endif  // defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#endif  // CoreTextAttrStringBuilder_DEFINED
