// Copyright 2026 Google LLC
#ifndef CoreTextAttrStringBuilder_DEFINED
#define CoreTextAttrStringBuilder_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTArray.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/CoreTextFontRegistry.h"
#include "modules/skparagraph/src/CoreTextIndexMap.h"

#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
#include "src/utils/mac/SkUniqueCFRef.h"
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

    // Script of the itemized run holding utf8Index; like SkShaper's script iterator, common
    // code points join the preceding script, or the following one at the start of the text
    SkFourByteTag scriptAt(size_t utf8Index) const;

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

    struct ScriptRun {
        TextRange fRange;
        SkFourByteTag fScript;
    };
    skia_private::STArray<4, ScriptRun> fScriptRuns;
};

}  // namespace textlayout
}  // namespace skia

#endif  // defined(SK_PARAGRAPH_NATIVE_CORETEXT)

#endif  // CoreTextAttrStringBuilder_DEFINED
