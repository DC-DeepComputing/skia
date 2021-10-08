// Copyright 2021 Google LLC.
#include "experimental/sktext/include/Text.h"
#include "experimental/sktext/src/Line.h"
#include "experimental/sktext/src/LogicalRun.h"

namespace skia {
namespace text {
LogicalLine::LogicalLine(const Stretch& stretch, const Stretch& spaces, SkScalar verticalOffset, bool hardLineBreak)
    : fTextStart(stretch.glyphStart())
    , fTextEnd(stretch.glyphEnd())
    , fWhitespacesEnd (spaces.glyphEnd())
    , fText(stretch.textRange())
    , fWhitespaces(spaces.textRange())
    , fTextWidth(stretch.width())
    , fSpacesWidth(spaces.width())
    , fHorizontalOffset(0.0f)
    , fVerticalOffset(verticalOffset)
    , fHardLineBreak(hardLineBreak) {
    SkASSERT(stretch.isEmpty() ||
                    spaces.isEmpty() ||
        (stretch.glyphEnd() == spaces.glyphStart()));

    if (!stretch.isEmpty()) {
        this->fTextMetrics.merge(stretch.textMetrics());
    }
    if (!spaces.isEmpty()) {
        this->fTextMetrics.merge(spaces.textMetrics());
    }
}

Stretch::Stretch(size_t runIndex, const LogicalRun* run)
    : fGlyphStart(runIndex, 0)
    , fGlyphEnd(runIndex, run->size())
    , fWidth(run->width())
    , fTextRange(run->getTextRange())
    , fTextMetrics(run->getTextMetrics()) { }

} // namespace text
} // namespace skia
