// Copyright 2021 Google LLC.
#ifndef VisualRun_DEFINED
#define VisualRun_DEFINED

#include "experimental/sktext/include/Types.h"
#include "experimental/sktext/src/Line.h"
#include "modules/skshaper/include/SkShaper.h"

namespace skia {
namespace text {

class VisualRun {
    public:
    VisualRun(TextRange textRange, SkSpan<SkPoint> positions, SkSpan<SkGlyphID> glyphs, SkSpan<TextIndex> clusters)
        : fUtf16Range(textRange) {
        fPositions.reserve_back(positions.size());
        for (auto& pos : positions) {
            fPositions.emplace_back(pos);
        }
        fGlyphs.reserve_back(glyphs.size());
        for (auto glyph : glyphs) {
            fGlyphs.emplace_back(glyph);
        }
        fClusters.reserve_back(glyphs.size());
        for (auto cluster : clusters) {
            fClusters.emplace_back(cluster);
        }
    }

    SkScalar calculateWidth(GlyphRange glyphRange) const {
        SkASSERT(glyphRange.fStart <= glyphRange.fEnd && glyphRange.fEnd < fPositions.size());
        return fPositions[glyphRange.fEnd].fX - fPositions[glyphRange.fStart].fX;
    }
    SkScalar calculateWidth(GlyphIndex start, GlyphIndex end) const {
      return calculateWidth(GlyphRange(start, end));
    }
    SkScalar width() const { return fAdvance.fX; }
    SkScalar firstGlyphPosition() const { return fPositions[0].fX; }

    bool leftToRight() const { return fBidiLevel % 2 == 0; }
    uint8_t bidiLevel() const { return fBidiLevel; }
    size_t size() const { return fGlyphs.size(); }
    TextMetrics textMetrics() const { return fTextMetrics; }
    GlyphIndex trailingSpacesStart() const { return fTrailingSpacesStart; }
    TextRange textRange() const { return fUtf16Range; }

    template <typename Callback>
    void forEachTextChunkInGlyphRange(SkSpan<TextIndex> textChunks, Callback&& callback) const {
        if (this->leftToRight()) {
            TextIndex currentIndex = 0;
            TextRange textRange(fUtf16Range.fStart, fUtf16Range.fStart);
            for (auto currentTextChunk : textChunks) {
                if (currentIndex >= fUtf16Range.fEnd) {
                    break;
                }
                currentIndex += currentTextChunk;
                if (currentIndex < fUtf16Range.fStart) {
                    continue;
                }
                textRange.fStart = textRange.fEnd;
                textRange.fEnd += currentTextChunk;
                textRange.fEnd = std::min(fUtf16Range.fEnd, textRange.fEnd);

                callback(textRange);
            }
        } else {
            // TODO: Implement RTL
            SkASSERT(false);
        }
    }

    private:
    friend class WrappedText;
    SkFont fFont;
    TextMetrics fTextMetrics;

    SkVector fAdvance;
    SkShaper::RunHandler::Range fUtf8Range;
    TextRange fUtf16Range;
    TextIndex fRunStart;
    SkScalar  fRunOffset;
    SkSTArray<128, SkGlyphID, true> fGlyphs;
    SkSTArray<128, SkPoint, true> fPositions;
    SkSTArray<128, TextIndex, true> fClusters;
    GlyphIndex fTrailingSpacesStart;
    uint8_t fBidiLevel;
};

class VisualLine {
public:
    VisualLine(TextRange text, SkScalar verticalOffset, SkSpan<VisualRun> runs)
        : fText(text)
        , fRuns(runs)
        , fTrailingSpaces(0, 0)
        , fOffset(SkPoint::Make(0, verticalOffset))
        , fActualWidth(0.0f)
        , fTextMetrics() {
        // Calculate all the info
        for (auto& run : fRuns) {
            fTextMetrics.merge(run.textMetrics());
            fActualWidth += run.width();  // What about trailing spaces?
            if (run.trailingSpacesStart() == 0) {
                // The entire run is trailing spaces, do not move the counter
            } else {
                // We can reset the trailing spaces counter
                fTrailingSpaces.fStart = fTrailingSpaces.fEnd + run.trailingSpacesStart();
            }
            fTrailingSpaces.fEnd += run.size();
        }
        // TODO: Set fHardBreak
    }

    SkScalar baseline() const { return fTextMetrics.baseline(); }
    TextRange text() const { return fText; }
    GlyphRange trailingSpaces() const { return fTrailingSpaces; }
    bool isHardBreak() const { return fIsHardBreak; }

    bool isFirst(VisualRun* run) { return &fRuns.front() == run; }
    bool isLast(VisualRun* run) { return &fRuns.back() == run; }
private:
    friend class WrappedText;
    TextRange fText;
    SkSpan<VisualRun> fRuns;
    GlyphRange fTrailingSpaces; // This is a zero-based index across the line
    SkPoint fOffset;            // For left/right/center formatting and for height
    SkScalar fActualWidth;
    TextMetrics fTextMetrics;
    bool fIsHardBreak;
};
}; // namespace text
} // namespace skia
#endif
