// Copyright 2021 Google LLC.

#include "experimental/sktext/include/Text.h"
#include "experimental/sktext/src/LogicalRun.h"
#include "experimental/sktext/src/VisualRun.h"
#include <memory>
#include <stack>

namespace skia {
namespace text {

UnicodeText::UnicodeText(std::unique_ptr<SkUnicode> unicode, SkSpan<uint16_t> utf16)
    : fUnicode(std::move(unicode))
    , fText16(std::u16string((char16_t*)utf16.data(), utf16.size())) {
    SkASSERT(fUnicode);
    initialize(utf16);
}

UnicodeText::UnicodeText(std::unique_ptr<SkUnicode> unicode, const SkString& utf8)
    : fUnicode(std::move(unicode)) {
    SkASSERT(fUnicode);
    fText16 = fUnicode->convertUtf8ToUtf16(utf8);
    initialize(SkSpan<uint16_t>((uint16_t*)fText16.data(), fText16.size()));
}

bool UnicodeText::isWhitespaces(TextRange range) const {
    if (range.leftToRight()) {
        for (auto i = range.fStart; i < range.fEnd; ++i) {
            if (!this->hasProperty(i, CodeUnitFlags::kPartOfWhiteSpace)) {
                return false;
            }
        }
    } else {
        for (auto i = range.fStart; i > range.fEnd; --i) {
            if (!this->hasProperty(i, CodeUnitFlags::kPartOfWhiteSpace)) {
                return false;
            }
        }
    }
    return true;
}

void UnicodeText::initialize(SkSpan<uint16_t> utf16) {
    // Get white spaces
    fCodeUnitProperties.push_back_n(utf16.size() + 1);
    this->fUnicode->forEachCodepoint((char16_t*)utf16.data(), utf16.size(),
       [this](SkUnichar unichar, int32_t start, int32_t end) {
            if (this->fUnicode->isWhitespace(unichar)) {
                for (auto i = start; i < end; ++i) {
                    fCodeUnitProperties[i] |=  CodeUnitFlags::kPartOfWhiteSpace;
                }
            }
       });

    // Get graphemes
    this->fUnicode->forEachBreak((char16_t*)utf16.data(), utf16.size(), SkUnicode::BreakType::kGraphemes,
                           [this](SkBreakIterator::Position pos, SkBreakIterator::Status) {
                                fCodeUnitProperties[pos]|= CodeUnitFlags::kGraphemeStart;
                            });

    // Get line breaks
    this->fUnicode->forEachBreak((char16_t*)utf16.data(), utf16.size(), SkUnicode::BreakType::kLines,
                           [this](SkBreakIterator::Position pos, SkBreakIterator::Status status) {
                                if (status == (SkBreakIterator::Status)SkUnicode::LineBreakType::kHardLineBreak) {
                                    // Hard line breaks clears off all the other flags
                                    // TODO: Treat \n as a formatting mark and do not pass it to SkShaper
                                    fCodeUnitProperties[pos - 1] = CodeUnitFlags::kHardLineBreakBefore;
                                } else {
                                    fCodeUnitProperties[pos] |= CodeUnitFlags::kSoftLineBreakBefore;
                                }
                            });
}

// Font iterator that finds all formatting marks
// and breaks runs on them (so we can select and interpret them later)
class FormattingFontIterator final : public SkShaper::FontRunIterator {
public:
    FormattingFontIterator(TextIndex textCount,
                           SkSpan<FontBlock> fontBlocks,
                           SkSpan<TextIndex> marks)
            : fTextCount(textCount)
            , fFontBlocks(fontBlocks)
            , fFormattingMarks(marks)
            , fCurrentBlock(fontBlocks.begin())
            , fCurrentMark(marks.begin())
            , fCurrentFontIndex(0)
            , fCurrentFont(fCurrentBlock->createFont()) {}

    void consume() override {
        SkASSERT(fCurrentBlock < fFontBlocks.end());
        SkASSERT(fCurrentMark < fFormattingMarks.end());

        if (fCurrentFontIndex <= *fCurrentMark) {
            if (fCurrentFontIndex == *fCurrentMark) {
                ++fCurrentMark;
            }
            ++fCurrentBlock;
            if (fCurrentBlock < fFontBlocks.end()) {
                fCurrentFontIndex += fCurrentBlock->charCount;
                fCurrentFont = fCurrentBlock->createFont();
            }
        } else {
            ++fCurrentMark;
        }
    }
    size_t endOfCurrentRun() const override {
        SkASSERT(fCurrentMark != fFormattingMarks.end() || fCurrentBlock != fFontBlocks.end());
        if (fCurrentMark == fFormattingMarks.end()) {
            return fCurrentFontIndex;
        } else if (fCurrentBlock == fFontBlocks.end()) {
            return *fCurrentMark;
        } else {
            return fCurrentFontIndex <= *fCurrentMark ? fCurrentFontIndex : *fCurrentMark;
        }
    }
    bool atEnd() const override {
        return (fCurrentBlock == fFontBlocks.end() || fCurrentFontIndex == fTextCount) &&
               (fCurrentMark == fFormattingMarks.end() || *fCurrentMark == fTextCount);
    }
    const SkFont& currentFont() const override { return fCurrentFont; }

    SkFont createFont(const FontBlock& fontBlock) {
        if (fontBlock.chain->count() == 0) {
            return SkFont();
        }
        sk_sp<SkTypeface> typeface = fontBlock.chain->operator[](0);
        SkFont font(std::move(typeface), fontBlock.chain->size());
        font.setEdging(SkFont::Edging::kAntiAlias);
        font.setHinting(SkFontHinting::kSlight);
        font.setSubpixel(true);
        return font;
    }

private:
    TextIndex const fTextCount;
    SkSpan<FontBlock> fFontBlocks;
    SkSpan<TextIndex> fFormattingMarks;
    FontBlock* fCurrentBlock;
    TextIndex* fCurrentMark;
    TextIndex fCurrentFontIndex;
    SkFont fCurrentFont;
};

std::unique_ptr<ShapedText> UnicodeText::shape(SkSpan<FontBlock> fontBlocks,
                                               TextDirection textDirection) {

    // Break text into pieces by font blocks and by formatting marks
    // Formatting marks: \n (and possibly some other later)
    auto text16 = this->getText16();
    auto text8 = this->getUnicode()->convertUtf16ToUtf8(std::u16string(text16.data(), text16.size()));
    std::vector<TextIndex> formattingMarks;
    formattingMarks.emplace_back(text8.size());
    for (size_t i = 0; i < text16.size(); ++i) {
        if (this->isHardLineBreak(i)) {
            formattingMarks.emplace_back(i);
        }
    }

    auto shapedText = std::unique_ptr<ShapedText>(new ShapedText());

    // Shape the text
    FormattingFontIterator fontIter(text8.size(), fontBlocks, SkSpan<TextIndex>(&formattingMarks[0], 1));
    SkShaper::TrivialLanguageRunIterator langIter(text8.c_str(), text8.size());
    std::unique_ptr<SkShaper::BiDiRunIterator> bidiIter(
        SkShaper::MakeSkUnicodeBidiRunIterator(
            this->getUnicode(), text8.c_str(), text8.size(), textDirection == TextDirection::kLtr ? 0 : 1));
    std::unique_ptr<SkShaper::ScriptRunIterator> scriptIter(
        SkShaper::MakeSkUnicodeHbScriptRunIterator(this->getUnicode(), text8.c_str(), text8.size()));
    auto shaper = SkShaper::MakeShapeDontWrapOrReorder();
    if (shaper == nullptr) {
        // For instance, loadICU does not work. We have to stop the process
        return nullptr;
    }

    shaper->shape(
            text8.c_str(), text8.size(),
            fontIter, *bidiIter, *scriptIter, langIter,
            std::numeric_limits<SkScalar>::max(), shapedText.get());

    if (shapedText->fLogicalRuns.empty()) {
        // Create a fake run for an empty text (to avoid all the checks)
        SkShaper::RunHandler::RunInfo emptyInfo {
            fontIter.createFont(fontBlocks.front()),
            0,
            SkVector::Make(0.0f, 0.0f),
            0,
            SkShaper::RunHandler::Range(0, 0)
        };
        shapedText->fLogicalRuns.emplace_back(emptyInfo, 0, 0.0f);
        shapedText->fLogicalRuns.front().commit();
    }
    // Fill out all code unit properties
    size_t utf16Index = 0;
    SkTArray<size_t, true> uTF16FromUTF8;
    uTF16FromUTF8.push_back_n(text8.size() + 1, utf16Index);
    this->getUnicode()->forEachCodepoint(text8.c_str(), text8.size(),
    [&](SkUnichar unichar, int32_t start, int32_t end) {
        for (auto i = start; i < end; ++i) {
            uTF16FromUTF8[i] = utf16Index;
        }
        ++utf16Index;
    });
    uTF16FromUTF8[text8.size()] = text16.size();

    for (auto& logicalRun : shapedText->fLogicalRuns) {
        // Convert utf8 range into utf16 range
        logicalRun.convertUtf16Range([&](unsigned long index8) {
            return uTF16FromUTF8[index8];
        });

        // Convert all utf8 indexes into utf16 indexes (and also shift them to be on the entire text scale, too)
        logicalRun.convertClusterIndexes([&](TextIndex clusterIndex8) {
            return uTF16FromUTF8[clusterIndex8];
        });
    }

    return std::move(shapedText);
}

// TODO: Implement the vertical restriction (height) and add ellipsis
std::unique_ptr<WrappedText> ShapedText::wrap(UnicodeText* unicodeText, float width, float height) {

    auto wrappedText = std::unique_ptr<WrappedText>(new WrappedText());

    // line + spaces + clusters
    Stretch line;
    Stretch spaces;
    Stretch clusters;
    Stretch cluster;
    for (size_t runIndex = 0; runIndex < this->fLogicalRuns.size(); ++runIndex ) {

        auto& run = this->fLogicalRuns[runIndex];
        TextMetrics runMetrics(run.fFont);
        if (!run.leftToRight()) {
            cluster.setTextRange({ run.fUtf16Range.fStart, run.fUtf16Range.fEnd});
        }

        for (size_t glyphIndex = 0; glyphIndex < run.fPositions.size(); ++glyphIndex) {
            auto textIndex = run.fClusters[glyphIndex];

            if (cluster.isEmpty()) {
                cluster = Stretch(GlyphPos(runIndex, glyphIndex), textIndex, runMetrics);
                continue;
          }

          // The entire cluster belongs to a single run
          SkASSERT(cluster.glyphStart().runIndex() == runIndex);

          auto clusterWidth = run.calculateWidth(cluster.glyphStartIndex(), glyphIndex);
          cluster.finish(glyphIndex, textIndex, clusterWidth);

          auto isHardLineBreak = unicodeText->isHardLineBreak(cluster.textStart());
          auto isSoftLineBreak = unicodeText->isSoftLineBreak(cluster.textStart());
          auto isWhitespaces = unicodeText->isWhitespaces(cluster.textRange());
          auto isEndOfText = run.leftToRight() ? textIndex == run.fUtf16Range.fEnd : textIndex == run.fUtf16Range.fStart;

          if (isWhitespaces || isHardLineBreak) {
              // This is the end of the word
              if (!clusters.isEmpty()) {
                  line.moveTo(spaces);
                  line.moveTo(clusters);
                  spaces = clusters;
              }
          }

          // line + spaces + clusters + cluster
          if (isWhitespaces) {
              // Whitespaces do not extend the line width
              spaces.moveTo(cluster);
              clusters = cluster;
              continue;
          } else if (isHardLineBreak) {
              // Hard line break ends the line but does not extend the width
              // Same goes for the end of the text
              spaces.moveTo(cluster);
              addLine(wrappedText.get(), unicodeText->getUnicode(), line, spaces, true);
              line = spaces;
              clusters = spaces;
              continue;
          } else if (!SkScalarIsFinite(width)) {
              clusters.moveTo(cluster);
              continue;
          }

          // Now let's find out if we can add the cluster to the line
          if ((line.width() + spaces.width() + clusters.width() + cluster.width()) <= width) {
              clusters.moveTo(cluster);
          } else {
              if (line.isEmpty()) {
                  if (spaces.isEmpty() && clusters.isEmpty()) {
                      // There is only this cluster and it's too long;
                      // we are drawing it anyway
                      line.moveTo(cluster);
                  } else {
                      // We break the only one word on the line by this cluster
                      line.moveTo(clusters);
                  }
              } else {
                  // We move clusters + cluster on the next line
                  // TODO: Parametrise possible ways of breaking too long word
                  //  (start it from a new line or squeeze the part of it on this line)
              }
              this->addLine(wrappedText.get(), unicodeText->getUnicode(), line, spaces, false);
              line = spaces;
              clusters.moveTo(cluster);
          }

          cluster = Stretch(GlyphPos(runIndex, glyphIndex), textIndex, runMetrics);
        }
    }

    if (!clusters.isEmpty()) {
        line.moveTo(spaces);
        line.moveTo(clusters);
        spaces = clusters;
    } else if (wrappedText->fVisualLines.empty()) {
        // Empty text; we still need a line to avoid checking for empty lines every time
        line.moveTo(cluster);
        spaces.moveTo(cluster);
    }

    this->addLine(wrappedText.get(), unicodeText->getUnicode(), line, spaces, false);

    wrappedText->fActualSize.fWidth = width;

    return std::move(wrappedText);
}

SkTArray<int32_t> ShapedText::getVisualOrder(SkUnicode* unicode, RunIndex startRun, RunIndex endRun) {
    auto numRuns = endRun - startRun + 1;
    SkTArray<int32_t> results;
    results.push_back_n(numRuns);
    if (numRuns == 0) {
        return results;
    }

    SkTArray<SkUnicode::BidiLevel> runLevels(numRuns);
    size_t runLevelsIndex = 0;
    for (RunIndex runIndex = startRun; runIndex <= endRun; ++runIndex) {
        runLevels[runLevelsIndex++] = fLogicalRuns[runIndex].bidiLevel();
    }
    SkASSERT(runLevelsIndex == numRuns);
    unicode->reorderVisual(runLevels.data(), numRuns, results.data());

    return results;
}

// TODO: Fill line fOffset.fY
void ShapedText::addLine(WrappedText* wrappedText, SkUnicode* unicode, Stretch& stretch, Stretch& spaces, bool hardLineBreak) {

    auto startRun = stretch.glyphStart().runIndex();
    auto endRun = spaces.glyphEnd().runIndex();
    // Reorder and cut (if needed) runs so they fit the line
    auto visualOrder = std::move(this->getVisualOrder(unicode, startRun, endRun));
    // Walk through the line's runs in visual order
    auto firstRunIndex = startRun;
    auto runStart = wrappedText->fVisualRuns.size();
    for (auto visualIndex : visualOrder) {
        // TODO: Cut out the part of a shaped->logical run and copy it to wrapper->logical run
        auto& logicalRun = fLogicalRuns[firstRunIndex + visualIndex];
        auto startIndex = (startRun == (firstRunIndex + visualIndex)) ? stretch.glyphStart().glyphIndex() : 0;
        auto endIndex = (endRun == (firstRunIndex + visualIndex)) ? spaces.glyphEnd().glyphIndex() : logicalRun.size();

        // TODO: Convert indices
        wrappedText->fVisualRuns.emplace_back(logicalRun.fUtf16Range,
                                   SkSpan<SkPoint>(&logicalRun.fPositions[startIndex], endIndex - startIndex + 1),
                                   SkSpan<SkGlyphID>(&logicalRun.fGlyphs[startIndex], endIndex - startIndex),
                                   SkSpan<TextIndex>((TextIndex*)&logicalRun.fClusters[startIndex], endIndex - startIndex + 1));
        wrappedText->fActualSize.fHeight += stretch.textMetrics().height();
    }

    stretch.moveTo(spaces);
    wrappedText->fVisualLines.emplace_back(stretch.textRange(), stretch.textMetrics().height(), SkSpan<VisualRun>(&wrappedText->fVisualRuns[runStart], wrappedText->fVisualRuns.size() - runStart));
    fTextHeight += stretch.textMetrics().height();

    stretch.clean();
    spaces.clean();
}

void WrappedText::format(TextAlign textAlign, TextDirection textDirection) {

    if (fAligned == textAlign) {
        return;
    }
    SkScalar verticalOffset = 0.0f;
    for (auto& line : this->fVisualLines) {
        if (textAlign == TextAlign::kLeft) {
            // Good by default
        } else if (textAlign == TextAlign::kCenter) {
            line.fOffset.fX = (this->fActualSize.width() - line.fActualWidth) / 2.0f;
        } else {
            // TODO: Implement all formatting features
        }
        line.fOffset.fY = verticalOffset;
        verticalOffset += line.fTextMetrics.height();
    }
}

void WrappedText::visit(Visitor* visitor) const {

    size_t lineIndex = 0;
    SkScalar verticalOffset = 0.0f;
    for (auto& line : fVisualLines) {
        visitor->onBeginLine(lineIndex, line.text(), line.isHardBreak(), SkRect::MakeXYWH(0, verticalOffset, line.fActualWidth, line.fTextMetrics.height()));
        for (auto& run : fVisualRuns) {
            auto diff = line.fTextMetrics.above() - run.fTextMetrics.above();
            SkRect boundingRect = SkRect::MakeXYWH(line.fOffset.fX + run.fPositions[0].fX, line.fOffset.fY + diff, run.width(), run.fTextMetrics.height());
            visitor->onGlyphRun(run.fFont, run.textRange(), boundingRect, run.trailingSpacesStart(),
                                run.size(), run.fGlyphs.data(), run.fPositions.data(), run.fClusters.data());
        }
        visitor->onEndLine(lineIndex, line.text(), line.trailingSpaces());
        verticalOffset += line.fTextMetrics.height();
        ++lineIndex;
    }
}

void WrappedText::visit(UnicodeText* unicodeText, Visitor* visitor, PositionType positionType, SkSpan<TextIndex> textChunks) const {
    // Decor blocks have to be sorted by text cannot intersect but can skip some parts of the text
    // (in which case we use default text style from paragraph style)
    // The edges of the decor blocks don't have to match glyph, grapheme or even unicode code point edges
    // It's out responsibility to adjust them to some reasonable values
    // [a:b) -> [c:d) where
    // c is closest GG cluster edge to a from the left and d is closest GG cluster edge to b from the left

    SkScalar verticalOffset = 0.0f;
    LineIndex lineIndex = 0;
    for (auto& line : fVisualLines) {
        visitor->onBeginLine(lineIndex, line.text(), line.isHardBreak(), SkRect::MakeXYWH(0, verticalOffset, line.fActualWidth, line.fTextMetrics.height()));
        RunIndex runIndex = 0;
        for (auto& run : fVisualRuns) {

            run.forEachTextChunkInGlyphRange(textChunks, [&](TextRange textRange) {
                // TODO: Translate text range into glyph range (with all the appropriate adjustments)
                GlyphRange glyphRange = this->textToGlyphs(unicodeText, positionType, runIndex, textRange);
                auto diff = line.fTextMetrics.above() - run.fTextMetrics.above();
                SkRect boundingRect = SkRect::MakeXYWH(line.fOffset.fX + run.fPositions[glyphRange.fStart].fX, line.fOffset.fY + diff, glyphRange.width(), run.fTextMetrics.height());
                visitor->onGlyphRun(run.fFont, textRange, boundingRect, run.trailingSpacesStart(),
                                    glyphRange.width(), &run.fGlyphs[glyphRange.fStart], &run.fPositions[glyphRange.fStart], &run.fClusters[glyphRange.fStart]);
            });
            ++runIndex;
        }
        visitor->onEndLine(lineIndex, line.text(), line.trailingSpaces());
        verticalOffset += line.fTextMetrics.height();
        ++lineIndex;
    }
}

// TODO: Implement more effective search
GlyphRange WrappedText::textToGlyphs(UnicodeText* unicodeText, PositionType positionType, RunIndex runIndex, TextRange textRange) const {
    SkASSERT(runIndex < fVisualRuns.size());
    auto& run = fVisualRuns[runIndex];
    SkASSERT(run.fUtf16Range.contains(textRange));

    GlyphRange glyphRange(0, run.size());
    for (GlyphIndex glyph = 0; glyph < run.size(); ++glyph) {
        auto textIndex = run.fClusters[glyph];
        if (positionType == PositionType::kGraphemeCluster && unicodeText->hasProperty(textIndex, CodeUnitFlags::kGraphemeStart)) {
            if (textIndex <= textRange.fStart) {
                glyphRange.fStart = glyph;
            } else if (textIndex <= textRange.fEnd) {
                glyphRange.fEnd = glyph;
            } else {
                return glyphRange;
            }
        }
    }
    SkASSERT(false);
    return glyphRange;
}

std::unique_ptr<DrawableText> WrappedText::prepareToDraw(UnicodeText* unicodeText, PositionType positionType, SkSpan<TextIndex> blocks) const {
    auto drawableText = std::make_unique<DrawableText>();
    this->visit(unicodeText, drawableText.get(), positionType, blocks);
    return std::move(drawableText);
}

std::unique_ptr<SelectableText> WrappedText::prepareToEdit() const {
    auto selectableText = std::make_unique<SelectableText>();
    this->visit(selectableText.get());
    return selectableText;
}

void SelectableText::onBeginLine(size_t index, TextRange lineText, bool hardBreak, SkRect bounds) {
    SkASSERT(fBoxLines.size() == index);
    fBoxLines.emplace_back(index, lineText, hardBreak, bounds);
    fCurrentRunEnd = 0;
}

void SelectableText::onEndLine(size_t index, TextRange lineText, GlyphRange trailingSpaces) {
    auto& line = fBoxLines.back();
    line.fTextEnd = trailingSpaces.fStart;
    line.fTrailingSpacesEnd = trailingSpaces.fEnd;
    line.fBoxGlyphs.push_back_n(line.fTrailingSpacesEnd);
    line.fTextByGlyph.push_back_n(line.fTrailingSpacesEnd);
}

void SelectableText::onGlyphRun(const SkFont& font,
                                TextRange textRange,
                                SkRect boundingRect,
                                int trailingSpacesStart,
                                int glyphCount,
                                const uint16_t glyphs[],
                                const SkPoint positions[],
                                const TextIndex clusters[]) {
    auto& line = fBoxLines.back();
    for (auto i = 0; i < glyphCount; ++i) {
        auto pos = positions[i];
        auto size = positions[i + 1] - positions[i];
        line.fBoxGlyphs[i] = SkRect::MakeXYWH(pos.fX, pos.fY, size.fX, size.fY);
        line.fTextByGlyph[i] = clusters[i];
    }
}

// TODO: Do something (logN) that is not a linear search
Position SelectableText::findPosition(PositionType positionType, const BoxLine& line, SkScalar x) const {

    Position position(positionType);
    position.fGlyphRange = GlyphRange(0, line.fBoxGlyphs.size());
    position.fTextRange = line.fTextRange;
    position.fBoundaries.fTop = line.fBounds.fTop;
    position.fBoundaries.fBottom = line.fBounds.fBottom;

    // We look for the narrowest glyph range adjusted to positionType that contains the point.
    // So far we made sure that one unit of any positionType does not cross the run edges
    // Therefore it's going to be represented by a single text range only
    for (; position.fGlyphRange.fStart < position.fGlyphRange.fEnd; ++position.fGlyphRange.fStart) {
        auto glyphBox = line.fBoxGlyphs[position.fGlyphRange.fStart];
        if (glyphBox.fLeft > x) {
            break;
        }
        if (position.fPositionType == PositionType::kGraphemeCluster) {
            auto textIndex = line.fTextByGlyph[position.fGlyphRange.fStart];
            if (this->hasProperty(textIndex, GlyphUnitFlags::kGraphemeClusterStart)) {
                position.fTextRange.fStart = textIndex;
            }
        } else {
            // TODO: Implement
            SkASSERT(false);
        }
    }

    for (; position.fGlyphRange.fEnd > position.fGlyphRange.fStart ; --position.fGlyphRange.fEnd) {
        auto glyphBox = line.fBoxGlyphs[position.fGlyphRange.fStart];
        if (glyphBox.fRight <= x) {
            break;
        }
        if (position.fPositionType == PositionType::kGraphemeCluster) {
            auto textIndex = line.fTextByGlyph[position.fGlyphRange.fEnd];
            if (this->hasProperty(textIndex, GlyphUnitFlags::kGraphemeClusterStart)) {
                position.fTextRange.fEnd = textIndex;
                break;
            }
        } else {
            // TODO: Implement
            SkASSERT(false);
        }
    }

    position.fLineIndex = line.fIndex;
    position.fBoundaries.fLeft = line.fBoxGlyphs[position.fGlyphRange.fStart].fLeft;
    position.fBoundaries.fRight = line.fBoxGlyphs[position.fGlyphRange.fEnd].fRight;

    return position;
}

Position SelectableText::adjustedPosition(PositionType positionType, SkPoint xy) const {

    xy.fX = std::min(xy.fX, this->fActualSize.fWidth);
    xy.fY = std::min(xy.fY, this->fActualSize.fHeight);

    Position position(positionType);
    for (auto& line : fBoxLines) {
        if (line.fBounds.fTop > xy.fY) {
            // We are past the point vertically
            break;
        } else if (line.fBounds.fBottom <= xy.fY) {
            // We haven't reached the point vertically yet
            continue;
        }

        return this->findPosition(positionType, line, xy.fX);
    }

    return this->lastPosition(positionType);
}

Position SelectableText::previousPosition(Position current) const {

    const BoxLine* currentLine = &fBoxLines[current.fLineIndex];
    if (this->isFirstOnTheLine(current)) {
        // Go to the previous line
        if (current.fLineIndex == 0) {
            // We reached the end; there is nowhere to move
            current.fGlyphRange = GlyphRange(0, 0);
            return current;
        } else {
            current.fLineIndex -= 1;
            currentLine = &fBoxLines[current.fLineIndex];
            current.fGlyphRange.fStart = currentLine->fBoxGlyphs.size();
        }
    }

    auto position = this->findPosition(current.fPositionType, *currentLine, currentLine->fBoxGlyphs[current.fGlyphRange.fStart].centerX());

    if (current.fPositionType == PositionType::kGraphemeCluster) {
        // Either way we found us a grapheme cluster (just make sure of it)
        SkASSERT(this->hasProperty(current.fTextRange.fStart, GlyphUnitFlags::kGraphemeClusterStart));
    }

    return position;
}

Position SelectableText::nextPosition(Position current) const {

    const BoxLine* currentLine = &fBoxLines[current.fLineIndex];
    if (this->isLastOnTheLine(current)) {
        // Go to the previous line
        if (current.fLineIndex == this->fBoxLines.size() - 1) {
            // We reached the end; there is nowhere to move
            current.fGlyphRange = GlyphRange(currentLine->fBoxGlyphs.size(), currentLine->fBoxGlyphs.size());
            return current;
        } else {
            current.fLineIndex += 1;
            currentLine = &fBoxLines[current.fLineIndex];
            current.fGlyphRange.fEnd = 0;
        }
    }

    auto position = this->findPosition(current.fPositionType, *currentLine, currentLine->fBoxGlyphs[current.fGlyphRange.fStart].centerX());

    if (current.fPositionType == PositionType::kGraphemeCluster) {
        // Either way we found us a grapheme cluster (just make sure of it)
        SkASSERT(this->hasProperty(current.fTextRange.fEnd, GlyphUnitFlags::kGraphemeClusterStart));
    }

    return position;
}

Position SelectableText::firstPosition(PositionType positionType) const {
    auto firstLine = fBoxLines.front();
    auto firstGlyph = firstLine.fBoxGlyphs.front();

    Position beginningOfText(positionType);
    // Set the glyph range after the last glyph
    beginningOfText.fGlyphRange = GlyphRange { 0, 0};
    beginningOfText.fLineIndex = 0;
    beginningOfText.fBoundaries = SkRect::MakeXYWH(firstGlyph.fRight, firstGlyph.fRight, 0, firstGlyph.height());
    auto texts = this->glyphsToText(beginningOfText);
    SkASSERT(texts.size() == 1);
    beginningOfText.fTextRange = texts[0];

    return beginningOfText;
}

Position SelectableText::lastPosition(PositionType positionType) const {
    auto lastLine = fBoxLines.back();
    auto lastGlyph = lastLine.fBoxGlyphs.back();

    Position endOfText(positionType);
    endOfText.fLineIndex = lastLine.fIndex;
    endOfText.fGlyphRange = GlyphRange(lastLine.fBoxGlyphs.size(), lastLine.fBoxGlyphs.size());
    endOfText.fBoundaries = SkRect::MakeXYWH(lastGlyph.fRight, lastGlyph.fRight, 0, lastGlyph.height());

    auto texts = this->glyphsToText(endOfText);
    SkASSERT(texts.size() == 1);
    endOfText.fTextRange = texts[0];

    return endOfText;
}

std::vector<TextRange> SelectableText::glyphsToText(Position position) const {
    std::vector<TextRange> result;
    auto line = this->getLine(position.fLineIndex);
    TextRange textRange = EMPTY_RANGE;
    GlyphRange* currentRun = line.fRuns.begin();
    for (auto glyph = position.fGlyphRange.fStart; glyph < position.fGlyphRange.fEnd; ++glyph) {

        if (!currentRun->contains(glyph)) {
            while (!currentRun->contains(glyph)) {
                ++currentRun;
                SkASSERT(currentRun < line.fRuns.end());
            }
            SkASSERT(currentRun->contains(glyph));
            result.emplace_back(textRange);
            textRange = EMPTY_RANGE;
        }

        if (textRange.fStart == EMPTY_INDEX) {
            textRange.fStart = line.fTextByGlyph[glyph];
        }
        textRange.fEnd = line.fTextByGlyph[glyph];
    }
    return result;
}


} // namespace text
} // namespace skia
