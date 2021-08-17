// Copyright 2021 Google LLC.

#include "experimental/sktext/src/TextRun.h"

namespace skia {
namespace text {

class Processor;

TextRun::TextRun(const SkShaper::RunHandler::RunInfo& info, TextIndex textStart, SkScalar glyphOffset)
    : fFont(info.fFont)
    , fBidiLevel(info.fBidiLevel)
    , fAdvance(info.fAdvance)
    , fUtf8Range(info.utf8Range)
    , fTextMetrics(info.fFont)
    , fRunStart(textStart)
    , fRunOffset(glyphOffset) {
    fGlyphs.push_back_n(info.glyphCount);
    fBounds.push_back_n(info.glyphCount);
    fPositions.push_back_n(info.glyphCount + 1);
    fOffsets.push_back_n(info.glyphCount);
    fClusters.push_back_n(info.glyphCount + 1);
}

void TextRun::commit() {
    fFont.getBounds(fGlyphs.data(), fGlyphs.size(), fBounds.data(), nullptr);
    // To make edge cases easier
    fPositions[fGlyphs.size()] = fAdvance;
    fClusters[fGlyphs.size()] = this->leftToRight() ? fUtf8Range.end() : fUtf8Range.begin();

    // Fill out glyph by text table
    TextIndex start = fClusters[0];
    for (auto clusterIndex = 0; clusterIndex < fClusters.size(); ++clusterIndex) {
        auto cluster = fClusters[clusterIndex];
        if (this->leftToRight()) {
            for (auto i = start; i < cluster; ++i) {
                fGlyphByText[i] = clusterIndex - 1;
            }
        } else {
            for (auto i = cluster; i >= start; --i) {
                fGlyphByText[i] = clusterIndex - 1;
            }
        }
        start = cluster;
    }
}

SkShaper::RunHandler::Buffer TextRun::newRunBuffer() {
    return {fGlyphs.data(), fPositions.data(), fOffsets.data(), fClusters.data(), {0.0f, 0.0f} };
}

SkScalar TextRun::calculateWidth(GlyphRange glyphRange) const {
    SkASSERT(glyphRange.fStart <= glyphRange.fEnd && glyphRange.fEnd < fPositions.size());
    return fPositions[glyphRange.fEnd].fX - fPositions[glyphRange.fStart].fX;
}

GlyphIndex TextRun::findGlyph(TextIndex textIndex) const {
    GlyphIndex glyphIndex = EMPTY_INDEX;
    for (size_t i = 0; i < fClusters.size(); ++i) {
        auto cluster = fClusters[i];
        if (this->leftToRight()) {
            if (cluster > textIndex) {
                break;
            }
        } else if (cluster < textIndex) {
            break;
        }
        glyphIndex = i;
    }
    return glyphIndex;
}

TextRange TextRun::getTextRange(GlyphRange glyphRange) const {
    return TextRange(this->fClusters[glyphRange.fStart], this->fClusters[glyphRange.fEnd]);
}

GlyphRange TextRun::getGlyphRange(TextRange textRange) const {
    // TODO: Implement
    return GlyphRange();
}

GlyphIndex TextRun::getGlyphIndex(TextIndex textIndex) const {
    //auto found = std::lower_bound(this->fClusters.begin(), this->fClusters.end())
        return 0;
}

SkTArray<GlyphIndex> TextRun::convertTextIntoGlyphs(GlyphRange glyphRange, SkSpan<TextIndex> textChunks) const {
    // For RTL text chunks are going into the order opposed to visual
    // Let's try and make this method the only one that is aware of the text order
    if (this->leftToRight()) {

    } else {
        TextIndex textIndex = this->fUtf16Range.width();
        GlyphIndex glyphIndex = glyphRange.fStart;
        for (auto i = textChunks.size(); i > 0; --i) {
            auto textChunk = textChunks[i - 1];
            TextRange textRange(textIndex - textChunk, textIndex);
            // Convert the text chunk into the glyph chunk (ts : te] -> [gs : ge)
            GlyphRange glyphRange(glyphIndex, glyphIndex);
            textIndex -= textChunk;
        }
    }
}

} // namespace text
} // namespace skia
