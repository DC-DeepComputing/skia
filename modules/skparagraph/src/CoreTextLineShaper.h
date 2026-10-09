// Copyright 2026 Google LLC
#ifndef CoreTextLineShaper_DEFINED
#define CoreTextLineShaper_DEFINED

#include "include/core/SkFourByteTag.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"

#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
#include <CoreText/CoreText.h>

namespace skia {
namespace textlayout {

class ParagraphImpl;

class CoreTextLineShaper {
public:
    explicit CoreTextLineShaper(ParagraphImpl* paragraph);

    bool shape();

    size_t unresolvedGlyphs() const { return fUnresolvedGlyphs; }

    // Identifies script from leading codepoints to populate SkShaper RunInfo metadata
    static SkFourByteTag detectScript(const char* utf8, size_t len);

    // Identifies fallback to LastResort font when glyph cascades are exhausted
    static bool isLastResortFont(CTFontRef font);

private:
    bool shapeTextRange(TextRange range, SkScalar& advanceX);
    bool shapeTextRegion(TextRange regionRange, SkScalar& advanceX);

    ParagraphImpl* fParagraph;
    size_t fUnresolvedGlyphs;
};

}  // namespace textlayout
}  // namespace skia

#endif  // defined(SK_PARAGRAPH_NATIVE_CORETEXT)

#endif  // CoreTextLineShaper_DEFINED
