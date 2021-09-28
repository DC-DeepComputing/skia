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
    VisualRun(TextRange textRange, GlyphIndex trailingSpacesStart, const SkFont& font, SkScalar lineBaseLine,
              SkPoint runOffset,
              bool leftToRight,
              SkSpan<SkPoint> positions, SkSpan<SkGlyphID> glyphs, SkSpan<uint32_t> clusters)
        : fFont(font)
        , fTextMetrics(TextMetrics(fFont))
        , fLineBaseLine(lineBaseLine)
        , fDirTextRange(textRange, leftToRight)
        , fTrailingSpacesStart(trailingSpacesStart) {
        if (positions.size() == 0) {
            SkASSERT(false);
            return;
        }
        fPositions.reserve_back(positions.size());
        runOffset -= SkPoint::Make(positions[0].fX, - fLineBaseLine);
        for (auto& pos : positions) {
            fPositions.emplace_back(pos + runOffset);
        }
        fGlyphs.reserve_back(glyphs.size());
        for (auto glyph : glyphs) {
            fGlyphs.emplace_back(glyph);
        }
        fClusters.reserve_back(clusters.size());
        for (auto cluster : clusters) {
            fClusters.emplace_back(SkToU16(cluster));
        }
        fAdvance= SkVector::Make(this->calculateWidth(0, glyphs.size()), fTextMetrics.height());
    }

    SkScalar calculateWidth(GlyphRange glyphRange) const {
        SkASSERT(glyphRange.fStart <= glyphRange.fEnd && glyphRange.fEnd < fPositions.size());
        return fPositions[glyphRange.fEnd].fX - fPositions[glyphRange.fStart].fX;
    }
    SkScalar calculateWidth(GlyphIndex start, GlyphIndex end) const {
      return calculateWidth(GlyphRange(start, end));
    }
    SkScalar width() const { return fAdvance.fX; }
    SkScalar height() const { return fAdvance.fY; }
    SkScalar firstGlyphPosition() const { return fPositions[0].fX; }
    TextMetrics textMetrics() const { return fTextMetrics; }

    bool leftToRight() const { return fDirTextRange.fLeftToRight; }
    size_t size() const { return fGlyphs.size(); }
    SkScalar baseLine() const { return fLineBaseLine; }
    GlyphIndex trailingSpacesStart() const { return fTrailingSpacesStart; }
    DirTextRange dirTextRange() const { return fDirTextRange; }

    template <typename Callback>
    void forEachTextBlockInGlyphRange(SkSpan<TextIndex> textBlock, Callback&& callback) const {
        if (this->leftToRight()) {
            DirTextRange dirTextRange(fDirTextRange.fStart, fDirTextRange.fStart, fDirTextRange.fLeftToRight);
            for (auto currentIndex : textBlock) {
                if (currentIndex >= fDirTextRange.fEnd) {
                    break;
                }
                if (currentIndex < fDirTextRange.fStart) {
                    continue;
                }
                dirTextRange.fStart = dirTextRange.fEnd;
                dirTextRange.fEnd = currentIndex;
                dirTextRange.fEnd = std::min(fDirTextRange.fEnd, dirTextRange.fEnd);

                callback(dirTextRange);
            }
        } else {
            // Revert chunks
            std::vector<TextIndex> revertedChunks;
/*
            TextIndex currentIndex = fUtf16Range.fEnd;
            TextRange textRange(fUtf16Range.fEnd, fUtf16Range.fEnd);
            for (auto chunkIndex = textChunks.size(); chunkIndex > 0; --chunkIndex) {
                auto currentTextChunk = textChunks[chunkIndex - 1];
                if (currentIndex >= fUtf16Range.fStart) {
                    break;
                }
                currentIndex -= currentTextChunk;
                if (currentIndex < fUtf16Range.fStart) {
                    continue;
                }
                textRange.fEnd = textRange.fStart;
                textRange.fStart -= currentTextChunk;

                callback(textRange);
            }
*/
        }
    }

    private:
    friend class WrappedText;
    SkFont fFont;
    TextMetrics fTextMetrics;
    SkScalar fLineBaseLine;

    SkVector fAdvance;
    DirTextRange fDirTextRange;
    SkSTArray<128, SkGlyphID, true> fGlyphs;
    SkSTArray<128, SkPoint, true> fPositions;
    SkSTArray<128, TextIndex, true> fClusters;
    GlyphIndex fTrailingSpacesStart;
};

class VisualLine {
public:
    VisualLine(TextRange text, bool hardLineBreak, SkScalar verticalOffset, size_t runCount)
        : fText(text)
        , fVisualRuns(runCount)
        , fTrailingSpaces(0, 0)
        , fOffset(SkPoint::Make(0, verticalOffset))
        , fActualWidth(0.0f)
        , fTextMetrics()
        , fIsHardBreak(hardLineBreak)
        , fGlyphCount(0ul) { }

    SkScalar baseline() const { return fTextMetrics.baseline(); }
    TextRange text() const { return fText; }
    GlyphRange trailingSpaces() const { return fTrailingSpaces; }
    bool isHardBreak() const { return fIsHardBreak; }
    size_t glyphCount() const { return fGlyphCount; }

    void addRun(VisualRun run) {
        fTextMetrics.merge(run.textMetrics());
        fActualWidth += run.width();  // What about trailing spaces?
        if (run.trailingSpacesStart() == 0) {
            // The entire run is trailing spaces, do not move the counter
        } else {
            // We can reset the trailing spaces counter
            fTrailingSpaces.fStart = fTrailingSpaces.fEnd + run.trailingSpacesStart();
        }
        fTrailingSpaces.fEnd += run.size();
        fVisualRuns.emplace_back(std::move(run));
    }
    bool isFirst(VisualRun* run) { return &fVisualRuns.front() == run; }
    bool isLast(VisualRun* run) { return &fVisualRuns.back() == run; }
private:
    friend class WrappedText;
    friend class VisualRun;
    TextRange fText;
    SkTArray<VisualRun, true> fVisualRuns;
    GlyphRange fTrailingSpaces; // This is a zero-based index across the line
    SkPoint fOffset;            // For left/right/center formatting and for height
    SkScalar fActualWidth;
    TextMetrics fTextMetrics;
    bool fIsHardBreak;
    size_t fGlyphCount;
};
}; // namespace text
} // namespace skia
#endif
