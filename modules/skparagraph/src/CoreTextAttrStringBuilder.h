// Copyright 2026 Google LLC
#ifndef CoreTextAttrStringBuilder_DEFINED
#define CoreTextAttrStringBuilder_DEFINED

#include "include/core/SkFourByteTag.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTemplates.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/CoreTextFontRegistry.h"
#include "modules/skparagraph/src/CoreTextIndexMap.h"

#include "src/utils/mac/SkUniqueCFRef.h"
#include <CoreText/CoreText.h>

#include <string_view>

class SkUnicode;

namespace skia {
namespace textlayout {

class CoreTextAttrStringBuilder {
public:
    // Decodes utf8, which must outlive the builder, once: the same pass fills the index map, the
    // UTF-16 text for build() and the script runs, whose UAX #24 scripts come from unicode
    // (used only here; must implement getScript, as the base default returns Zzzz). Fonts come
    // from fontCollection in build()
    CoreTextAttrStringBuilder(std::string_view utf8,
                              TextDirection baseDirection,
                              sk_sp<FontCollection> fontCollection,
                              SkUnicode* unicode);

    // Records style for utf8Range clipped to the text; an empty or out-of-text range is ignored,
    // and a span that follows the previous one and differs only in paint extends that one
    void addStyle(TextRange utf8Range, const TextStyle& style);

    // Returns the text as a CFAttributedString with the base writing direction and, per recorded
    // span, its font, features, variations and locale; null if CoreFoundation fails
    SkUniqueCFRef<CFAttributedStringRef> build();

    const CoreTextIndexMap& indexMap() const { return fIndexMap; }

    // Script of the itemized run holding utf8Index; like SkShaper's script iterator, Common and
    // Inherited code points join the preceding script, or the following one at the start of the
    // text, and text with no other script is Latn
    SkFourByteTag scriptAt(size_t utf8Index) const;

private:
    // Sets kCTFontAttributeName on the cursive script runs inside spanRange to baseFont with
    // descAttrs but userFeatures as feature settings, dropping the letterSpacing liga=0: cursive
    // runs get no letterSpacing (Run::isCursiveScript), so their ligatures stay (Allah in Arial)
    void setCursiveRunsFont(CFMutableAttributedStringRef attrStr,
                            TextRange spanRange,
                            CTFontRef baseFont,
                            CGFloat fontSize,
                            CFDictionaryRef descAttrs,
                            CFArrayRef userFeatures) const;

    std::string_view fUtf8;
    TextDirection fBaseDirection;
    sk_sp<FontCollection> fFontCollection;
    CoreTextIndexMap fIndexMap;
    // Sized to the UTF-8 length, an upper bound on the UTF-16 length
    skia_private::AutoSTArray<256, uint16_t> fUtf16;

    struct StyleSpan {
        TextRange range;
        TextStyle style;
    };
    skia_private::STArray<8, StyleSpan> fStyles;

    struct ScriptRun {
        TextRange range;
        SkFourByteTag script;
    };
    skia_private::STArray<4, ScriptRun> fScriptRuns;
};

}  // namespace textlayout
}  // namespace skia

#endif  // CoreTextAttrStringBuilder_DEFINED
