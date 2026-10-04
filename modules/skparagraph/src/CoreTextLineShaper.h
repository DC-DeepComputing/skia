// Copyright 2026 Google LLC
#ifndef CoreTextLineShaper_DEFINED
#define CoreTextLineShaper_DEFINED

#include "include/core/SkFourByteTag.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/TextStyle.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#include <CoreText/CoreText.h>

namespace skia {
namespace textlayout {

class ParagraphImpl;

class CoreTextLineShaper {
public:
    explicit CoreTextLineShaper(ParagraphImpl* paragraph);

    bool shape();

    size_t unresolvedGlyphs() const { return fUnresolvedGlyphs; }

    static SkFourByteTag detectScript(const char* utf8, size_t len);
    static bool isLastResortFont(CTFontRef font);

private:
    bool shapeTextRegion(TextRange regionRange, SkScalar& advanceX);

    ParagraphImpl* fParagraph;
    size_t fUnresolvedGlyphs;
};

}  // namespace textlayout
}  // namespace skia

#endif  // defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#endif  // CoreTextLineShaper_DEFINED
