// Copyright 2026 Google LLC
#ifndef CoreTextLineShaper_DEFINED
#define CoreTextLineShaper_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skshaper/include/SkShaper.h"

#include <CoreText/CoreText.h>

class SkString;
class SkUnicode;

namespace skia {
namespace textlayout {

class FontCollection;
class ParagraphImpl;
class Run;
struct CoreTextRunState;
struct RegionShapeContext;

class CoreTextLineShaper {
public:
    explicit CoreTextLineShaper(ParagraphImpl* paragraph);

    // Replaces the paragraph's runs, font switches and unresolved code points with ones from
    // CoreText: one CTLine per hard-break segment, plus a run per placeholder. Returns false for
    // empty text or when CoreText fails, and then leaves the runs, font switches and unresolved
    // code points empty
    bool shape();

    size_t unresolvedGlyphs() const { return fUnresolvedGlyphs; }

    // Shapes an ellipsis or hyphen in one CTLine with the style's font, cascade, trak, features
    // (liga off outside cursive runs when letter-spaced) and unicode's script runs, without
    // SkShaper or the default font manager. All CTRuns go into one RunHandler buffer; LastResort
    // or disallowed fallback glyphs become glyph 0, so the run reports itself unresolved
    static void ShapeAffix(const SkString& text,
                           const TextStyle& style,
                           sk_sp<SkTypeface> clusterTypeface,
                           bool fakeMissingFontStyles,
                           uint8_t bidiLevel,
                           sk_sp<FontCollection> fontCollection,
                           SkUnicode* unicode,
                           SkShaper::RunHandler* handler);

private:
    // Empties the paragraph's runs, font switches and unresolved code points, so a failed
    // shape() leaves nothing for the HarfBuzz fallback to append to
    void resetParagraphOutput();

    // Appends a single-glyph placeholder Run for placeholder at placeholderIndex and advances
    // advanceX by the placeholder's width
    void appendPlaceholderRun(const Placeholder& placeholder,
                              size_t placeholderIndex,
                              SkScalar& advanceX);

    // Splits range after each '\n' and shapes every piece with shapeTextRegion; false on the
    // first piece CoreText fails on
    bool shapeTextRange(TextRange range, SkScalar& advanceX);

    // Shapes regionRange in one CTLine with its blocks' styles and appends a Run and a font
    // switch per CTRun, starting at advanceX, which grows by the line's advance. Also counts
    // unresolved glyphs and records their code points. False if CoreText fails
    bool shapeTextRegion(TextRange regionRange, SkScalar& advanceX);

    // Resolves the CTRun at runIdx in ctx into a CoreTextRunState and appends its Run to
    // fParagraph, advancing advanceX by the run's width
    void shapeRegionRun(RegionShapeContext& ctx, int runIdx, SkScalar& advanceX);

    // Emplaces a Run from runState into fParagraph, populates its glyph buffers, records its font
    // switch, and advances advanceX by runState.fRunAdvanceX
    void appendRegionRun(CoreTextRunState& runState, SkScalar& advanceX);

    // Fills run with .notdef glyph IDs, baseline positions from runStartX, cluster indices, and
    // unresolved code point counts for a LastResort or disallowed-fallback run
    void populateFallbackRun(Run& run, const CoreTextRunState& runState, SkScalar runStartX);

    // Fills run with shaped glyph IDs, pen positions from runStartX, draw offsets, cluster
    // indices, and unresolved glyph counts, then commits runState's shaping buffer
    void populateShapedRun(Run& run, CoreTextRunState& runState, SkScalar runStartX);

    // Scans glyphRange in fParagraph->text() and records any non-control, non-zero-width code
    // points in fParagraph->fUnresolvedCodepoints while incrementing fUnresolvedGlyphs once
    void recordUnresolvedGlyph(TextRange glyphRange);

    ParagraphImpl* fParagraph;
    size_t fUnresolvedGlyphs;
};

}  // namespace textlayout
}  // namespace skia

#endif  // CoreTextLineShaper_DEFINED
