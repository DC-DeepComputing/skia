// Copyright 2021 Google LLC.

#include "experimental/sktext/include/Text.h"
#include "experimental/sktext/src/LogicalRun.h"
#include "experimental/sktext/src/VisualRun.h"
#include <memory>
#include <stack>

namespace skia {
namespace text {

void UnicodeText::initialize(SkSpan<uint16_t> utf16) {
    // Get white spaces
    this->fUnicode->forEachCodepoint((char16_t*)utf16.data(), utf16.size(),
       [this](SkUnichar unichar, int32_t start, int32_t end) {
            if (this->fUnicode->isWhitespace(unichar)) {
                for (auto i = start; i < end; ++i) {
                    this->fCodeUnitProperties[i] |=  CodeUnitFlags::kPartOfWhiteSpace;
                }
            }
       });

    // Get graphemes
    this->fUnicode->forEachBreak((char16_t*)utf16.data(), utf16.size(), SkUnicode::BreakType::kGraphemes,
                           [this](SkBreakIterator::Position pos, SkBreakIterator::Status) {
                                this->fCodeUnitProperties[pos]|= CodeUnitFlags::kGraphemeStart;
                            });

    // Get line breaks
    this->fUnicode->forEachBreak((char16_t*)utf16.data(), utf16.size(), SkUnicode::BreakType::kLines,
                           [this](SkBreakIterator::Position pos, SkBreakIterator::Status status) {
                                if (status == (SkBreakIterator::Status)SkUnicode::LineBreakType::kHardLineBreak) {
                                    // Hard line breaks clears off all the other flags
                                    // TODO: Treat \n as a formatting mark and do not pass it to SkShaper
                                    this->fCodeUnitProperties[pos - 1] = CodeUnitFlags::kHardLineBreakBefore;
                                } else {
                                    this->fCodeUnitProperties[pos] |= CodeUnitFlags::kSoftLineBreakBefore;
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

class ShapingHandler : public SkShaper::RunHandler {
public:
    ShapingHandler()
    : fCurrentRun(nullptr)
    , fParagraphTextStart(0)
    , fRunGlyphStart(0.0f) { }
    void beginLine() override {}
    void runInfo(const RunInfo&) override {}
    void commitRunInfo() override {}
    void commitLine() override {}
    void commitRunBuffer(const RunInfo&) override {
        fCurrentRun->commit();
        fLogicalRuns.emplace_back(std::move(*fCurrentRun));
        fRunGlyphStart += fCurrentRun->width();
    }
    Buffer runBuffer(const RunInfo& info) override {
        fCurrentRun = std::make_unique<LogicalRun>(info, fParagraphTextStart, fRunGlyphStart);
        return fCurrentRun->newRunBuffer();
    }
    bool isEmpty() const { return fLogicalRuns.empty(); }
    SkTArray<LogicalRun, false> logicalRuns() { return std::move(fLogicalRuns); }
private:
    TextIndex fParagraphTextStart;
    SkScalar fRunGlyphStart;
    std::unique_ptr<LogicalRun> fCurrentRun;
    SkTArray<LogicalRun, false> fLogicalRuns;
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

    ShapingHandler shapingHandler;
    shaper->shape(
            text8.c_str(), text8.size(),
            fontIter, *bidiIter, *scriptIter, langIter,
            std::numeric_limits<SkScalar>::max(), &shapingHandler);

    if (shapingHandler.isEmpty()) {
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
    } else {
        shapedText->fLogicalRuns = std::move(shapingHandler.logicalRuns());
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

std::unique_ptr<WrappedText> ShapedText::wrap(UnicodeText* unicodeText, float width, float height) {

    auto wrappedText = std::unique_ptr<WrappedText>(new WrappedText());

    // line : spaces : clusters
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

// TODO: Fill line fOffset.fY
void ShapedText::addLine(WrappedText* wrappedText, SkUnicode* unicode, Stretch& stretch, Stretch& spaces, bool hardLineBreak) {

    // This is just chosen to catch the common/fast cases. Feel free to tweak.
    constexpr int kPreallocCount = 4;
    auto start = stretch.glyphStart().runIndex();
    auto end = spaces.glyphEnd().runIndex();
    auto numRuns = end - start + 1;
    if (start > end) {
        return;
    }
    SkAutoSTArray<kPreallocCount, SkUnicode::BidiLevel> runLevels(numRuns);
    size_t runLevelsIndex = 0;
    for (auto runIndex = start; runIndex <= end; ++runIndex) {
        auto& run = fLogicalRuns[runIndex];
        runLevels[runLevelsIndex++] = run.bidiLevel();
    }
    SkASSERT(runLevelsIndex == numRuns);
    SkAutoSTArray<kPreallocCount, int32_t> logicalOrder(numRuns);
    unicode->reorderVisual(runLevels.data(), numRuns, logicalOrder.data());
    auto firstRunIndex = start;
    auto runStart = wrappedText->fVisualRuns.size();
    for (auto index : logicalOrder) {
        // TODO: Cut out the part of a shaped->logical run and copy it to wrapper->logical run
        auto& logicalRun = fLogicalRuns[firstRunIndex + index];
        auto startIndex = (start == (firstRunIndex + index)) ? stretch.glyphStart().glyphIndex() : 0;
        auto endIndex = (end == (firstRunIndex + index)) ? spaces.glyphEnd().glyphIndex() : logicalRun.size();

        wrappedText->fVisualRuns.emplace_back(logicalRun.fUtf16Range, SkSpan<SkPoint>(&logicalRun.fPositions[startIndex], endIndex - startIndex),
                                   SkSpan<SkGlyphID>(&logicalRun.fGlyphs[startIndex], endIndex - startIndex));
        wrappedText->fActualSize.fHeight += stretch.textMetrics().height();
    }

    stretch.moveTo(spaces);
    wrappedText->fVisualLines.emplace_back(stretch.textRange(), stretch.textMetrics().height(), SkSpan<VisualRun>(&wrappedText->fVisualRuns[runStart], wrappedText->fVisualRuns.size() - runStart));

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

void WrappedText::visit(Visitor* visitor, PositionType positionType, SkSpan<TextIndex> textChunks) const {
    // Decor blocks have to be sorted by text cannot intersect but can skip some parts of the text
    // (in which case we use default text style from paragraph style)2
    // The edges of the decor blocks don't have to match glyph, grapheme or even unicode code point edges
    // It's out responsibility to adjust them to some reasonable values
    // [a:b) -> [c:d) where
    // c is closest GG cluster edge to a from the left and d is closest GG cluster edge to b from the left

    size_t lineIndex = 0;
    for (auto& line : fVisualLines) {
        visitor->onBeginLine(lineIndex, line.text(), SkRect::MakeXYWH(line.fOffset.fX, line.fOffset.fY, line.fActualWidth, line.fTextMetrics.height()));
        size_t runIndex = 0;
        GlyphRange trailingSpaces(0,0);
        SkRect trailingSpacesBounds = SkRect::MakeEmpty();
        for (auto& run : fVisualRuns) {

            if (textChunks.empty()) {
                auto diff = line.fTextMetrics.above() - run.fTextMetrics.above();
                SkRect boundingRect = SkRect::MakeXYWH(line.fOffset.fX + run.fPositions[0].fX, line.fOffset.fY + diff, run.width(), run.fTextMetrics.height());
                visitor->onGlyphRun(run.fFont, run.textRange(), boundingRect, run.size(), run.fGlyphs.data(), run.fPositions.data());
                // TODO: Find and pass text <-> glyph information
                visitor->onBoxRun(boundingRect, run.size(), nullptr, nullptr, run.fPositions.data());
            } else {
                run.forEachTextChunkInGlyphRange(textChunks, [&](TextRange textRange) {
                    // TODO: Translate text range into glyph range (with all the appropriate adjustments)
                    GlyphRange glyphRange = this->textToGlyphs(positionType, textRange);
                    auto diff = line.fTextMetrics.above() - run.fTextMetrics.above();
                    SkRect boundingRect = SkRect::MakeXYWH(line.fOffset.fX + run.fPositions[glyphRange.fStart].fX, line.fOffset.fY + diff, glyphRange.width(), run.fTextMetrics.height());
                    visitor->onGlyphRun(run.fFont, textRange, boundingRect, glyphRange.width(), &run.fGlyphs[glyphRange.fStart], &run.fPositions[glyphRange.fStart]);
                    visitor->onBoxRun(boundingRect, run.size(), nullptr, nullptr, run.fPositions.data());
                });
            }

            if (runIndex < line.trailingSpaces().runIndex()) {
                trailingSpaces.fStart += run.size();
                trailingSpacesBounds.fLeft += run.fAdvance.fX;
            } else if (runIndex == line.trailingSpaces().runIndex()) {
                trailingSpaces.fStart += line.trailingSpaces().glyphIndex();
                trailingSpacesBounds.fLeft += run.calculateWidth(0, line.trailingSpaces().glyphIndex());
            }
            trailingSpacesBounds.fRight += run.fAdvance.fX;
            ++runIndex;
        }

        trailingSpacesBounds.fTop = line.fOffset.fY;
        trailingSpacesBounds.fBottom = line.fOffset.fY + line.fTextMetrics.height();
        visitor->onEndLine(lineIndex, line.text(), trailingSpaces, trailingSpacesBounds);
        ++lineIndex;
    }
}

std::unique_ptr<DrawableText> WrappedText::prepareToDraw(PositionType positionType, SkSpan<TextIndex> blocks) const {
    auto drawableText = std::make_unique<DrawableText>();
    this->visit(drawableText.get(), positionType, blocks);
    return std::move(drawableText);
}

std::unique_ptr<SelectableText> WrappedText::prepareToEdit(UnicodeText* unicodeText, PositionType positionType) const {
    auto selectableText = std::make_unique<SelectableText>();
    this->visit(selectableText.get(), positionType, {});
    return std::move(selectableText);
}

Position SelectableText::findPosition(PositionType positionType, const BoxLine& line, TextIndex textIndex) const {

    auto glyphIndex = line.fGlyphByText[textIndex];

    Position position(positionType);
    position.fGlyphRange = GlyphRange(glyphIndex, glyphIndex);
    position.fTextRange = TextRange(textIndex, textIndex);
    position.fBoundaries.fTop = line.fBounds.fTop;
    position.fBoundaries.fBottom = line.fBounds.fBottom;

    // We look for the narrowest glyph range adjusted to positionType
    // that is represented by text range containing the text index.
    // However we have to move by glyph order (that is the only order garanteed).
    // So far we made sure that one unit of any positionType does not cross the run edges
    // Therefore it's going to be represented by one text range only
    for (; position.fGlyphRange.fStart >= 0; --position.fGlyphRange.fStart) {
        if (position.fPositionType == PositionType::kGraphemeCluster) {
            auto leftTextIndex = line.fTextByGlyph[position.fGlyphRange.fStart];
            if (this->hasProperty(leftTextIndex, GlyphUnitFlags::kGraphemeClusterStart)) {
                position.fTextRange.fStart = leftTextIndex;
                break;
            }
        } else {
            // TODO: Implement
            SkASSERT(false);
        }
    }

    for (; position.fGlyphRange.fEnd <= line.fBoxGlyphs.size(); ++position.fGlyphRange.fEnd) {
        if (position.fPositionType == PositionType::kGraphemeCluster) {
            auto rightTextIndex = line.fTextByGlyph[position.fGlyphRange.fEnd];
            if (this->hasProperty(rightTextIndex, GlyphUnitFlags::kGraphemeClusterStart)) {
                position.fTextRange.fEnd = rightTextIndex;
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

    return std::move(position);
}

Position SelectableText::findPosition(PositionType positionType, const BoxLine& line, SkPoint point) const {

    Position position(positionType);
    position.fGlyphRange = GlyphRange(0, line.fBoxGlyphs.size());
    position.fTextRange = line.fTextRange;
    position.fBoundaries.fTop = line.fBounds.fTop;
    position.fBoundaries.fBottom = line.fBounds.fBottom;

    // We look for the narrowest glyph range adjusted to positionType that contains the point.
    // So far we made sure that one unit of any positionType does not cross the run edges
    // Therefore it's going to be represented by one text range only
    for (; position.fGlyphRange.fStart >= 0; --position.fGlyphRange.fStart) {
        if (position.fPositionType == PositionType::kGraphemeCluster) {
            auto textIndex = line.fTextByGlyph[position.fGlyphRange.fStart];
            if (this->hasProperty(textIndex, GlyphUnitFlags::kGraphemeClusterStart)) {
                position.fTextRange.fStart = textIndex;
                break;
            }
        } else {
            // TODO: Implement
            SkASSERT(false);
        }
    }

    for (; position.fGlyphRange.fEnd <= line.fBoxGlyphs.size(); ++position.fGlyphRange.fEnd) {
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

    return std::move(position);
}

// Find the element that includes textIndex
Position SelectableText::adjustedPosition(PositionType positionType, TextIndex textIndex) const {
    Position position(positionType);
    for (auto& line : fBoxLines) {
        if (!line.fTextRange.contains(textIndex)) {
            continue;
        }
        return this->findPosition(positionType, line, textIndex);
    }
    return this->lastElement(positionType);
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

        return this->findPosition(positionType, line, xy);
    }

    return this->lastElement(positionType);
}

Position SelectableText::previousElement(Position element) const {

    const BoxLine* currentLine = &fBoxLines[element.fLineIndex];
    if (this->isFirstOnTheLine(element)) {
        // Go to the previous line
        if (element.fLineIndex == 0) {
            // We reached the end; there is nowhere to move
            element.fGlyphRange = GlyphRange(0, 0);
            return element;
        } else {
            element.fLineIndex -= 1;
            currentLine = &fBoxLines[element.fLineIndex];
            element.fGlyphRange.fStart = currentLine->fBoxGlyphs.size();
        }
    }

    auto position = this->findPosition(element.fPositionType, *currentLine, currentLine->fBoxGlyphs[element.fGlyphRange.fStart].centerX());

    if (element.fPositionType == PositionType::kGraphemeCluster) {
        // Either way we found us a grapheme cluster (just make sure of it)
        SkASSERT(this->hasProperty(element.fTextRange.fStart, GlyphUnitFlags::kGraphemeClusterStart));
    }

    return position;
}

Position SelectableText::nextElement(Position element) const {

    const BoxLine* currentLine = &fBoxLines[element.fLineIndex];
    if (this->isLastOnTheLine(element)) {
        // Go to the previous line
        if (element.fLineIndex == this->fBoxLines.size() - 1) {
            // We reached the end; there is nowhere to move
            element.fGlyphRange = GlyphRange(currentLine->fBoxGlyphs.size(), currentLine->fBoxGlyphs.size());
            return element;
        } else {
            element.fLineIndex += 1;
            currentLine = &fBoxLines[element.fLineIndex];
            element.fGlyphRange.fEnd = 0;
        }
    }

    auto position = this->findPosition(element.fPositionType, *currentLine, currentLine->fBoxGlyphs[element.fGlyphRange.fEnd].centerX());

    if (element.fPositionType == PositionType::kGraphemeCluster) {
        // Either way we found us a grapheme cluster (just make sure of it)
        SkASSERT(this->hasProperty(element.fTextRange.fEnd, GlyphUnitFlags::kGraphemeClusterStart));
    }

    return position;
}

Position SelectableText::firstElement(PositionType positionType) const {

    Position beginningOfText(positionType);
    // Set the glyph range after the last glyph
    beginningOfText.fGlyphRange = GlyphRange { 0, 0};
    beginningOfText.fLineIndex = 0;

    return beginningOfText;
}

Position SelectableText::lastElement(PositionType positionType) const {
    auto lastLine = fBoxLines.back();
    auto lastGlyph = lastLine.fBoxGlyphs.back();

    Position endOfText(positionType);
    endOfText.fLineIndex = lastLine.fIndex;
    endOfText.fGlyphRange = GlyphRange(lastLine.fBoxGlyphs.size(), lastLine.fBoxGlyphs.size());
    auto texts = this->glyphsToText(positionType, endOfText.fGlyphRange);
    SkASSERT(texts.size() == 1);
    endOfText.fTextRange = texts[0];
    endOfText.fBoundaries = SkRect::MakeXYWH(lastGlyph.fRight, lastGlyph.fRight, 0, lastGlyph.height());
    return endOfText;
}

} // namespace text
} // namespace skia
