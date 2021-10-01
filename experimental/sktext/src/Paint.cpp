// Copyright 2021 Google LLC.
#include "experimental/sktext/src/Paint.h"

namespace skia {
namespace text {

    bool Paint::drawText(std::u16string text, SkCanvas* canvas, SkScalar x, SkScalar y) {
        SkPaint foregroundPaint(SkColors::kBlack);
        SkPaint backgroundPaint(SkColors::kWhite);
        return drawText(std::move(text), canvas, TextDirection::kLtr, TextAlign::kLeft, foregroundPaint, backgroundPaint, SkString("Roboto"), 14, SkFontStyle::Normal(), x, y);
    }

    bool Paint::drawText(std::u16string text, SkCanvas* canvas, SkScalar width) {
        SkPaint foregroundPaint(SkColors::kBlack);
        SkPaint backgroundPaint(SkColors::kWhite);
        return drawText(std::move(text), canvas,
                        TextDirection::kLtr, TextAlign::kLeft, foregroundPaint, backgroundPaint, SkString("Roboto"), 14, SkFontStyle::Normal(),
                        SkSize::Make(width, SK_ScalarInfinity), 0, 0);
    }

    bool Paint::drawText(std::u16string text, SkCanvas* canvas,
                         TextDirection textDirection, TextAlign textAlign,
                         SkPaint foreground, SkPaint background,
                         const SkString& fontFamily, SkScalar fontSize, SkFontStyle fontStyle, SkScalar x, SkScalar y) {
        return drawText(std::move(text), canvas,
                        textDirection, textAlign, foreground, background,
                        fontFamily, fontSize, fontStyle, SkSize::Make(SK_ScalarInfinity, SK_ScalarInfinity), x, y);
    }

    bool Paint::drawText(std::u16string text, SkCanvas* canvas,
                         TextDirection textDirection, TextAlign textAlign,
                         SkPaint foreground, SkPaint background,
                         const SkString& fontFamily, SkScalar fontSize, SkFontStyle fontStyle, SkSize reqSize, SkScalar x, SkScalar y) {

        size_t textSize = text.size();
        sk_sp<TrivialFontChain> fontChain = sk_make_sp<TrivialFontChain>(fontFamily.c_str(), fontSize, fontStyle);
        FontBlock fontBlock(text.size(), fontChain);
        if (fontChain->getTypeface() == nullptr || fontChain->getTypeface().get() == nullptr) {
            return false;
        }

        auto unicodeText = std::make_unique<UnicodeText>(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)text.data(), text.size()));
        auto formattedText = Paint::layout(unicodeText.get(), std::move(text), textDirection, textAlign, reqSize, SkSpan<FontBlock>(&fontBlock, 1));
        if (formattedText == nullptr) {
            return false;
        }

        DecoratedBlock decoratedBlock(textSize, foreground, background);
        Paint paint;
        paint.paint(canvas, SkPoint::Make(x, y), unicodeText.get(), formattedText.get(), SkSpan<DecoratedBlock>(&decoratedBlock, 1));

        return true;
    }

    void Paint::onBeginLine(size_t index, TextRange lineText, bool hardBreak, SkRect bounds, const TextMetrics& lineMetrics) {
        fLineOffset = SkPoint::Make(0, lineMetrics.baseline() + bounds.top());
    }

    void Paint::onGlyphRun(size_t chunkIndex,
                           const SkFont& font,
                           TextRange textRange,
                           SkRect boundingRect,
                           int trailingSpacesStart,
                           int glyphCount,
                           const uint16_t glyphs[],
                           const SkPoint positions[],
                           const TextIndex clusters[]) {

        SkTextBlobBuilder builder;
        const auto& blobBuffer = builder.allocRunPos(font , SkToInt(glyphCount));
        sk_careful_memcpy(blobBuffer.glyphs, glyphs, glyphCount * sizeof(uint16_t));
        sk_careful_memcpy(blobBuffer.points(), positions, glyphCount * sizeof(SkPoint));
        auto blob = builder.make();

        auto found = WrappedText::findChunk(SkSpan<size_t>(fChunks.data(), fChunks.size()), textRange);
        auto& decoratedBlock = fDecoratedBlocks[found];
        if (!decoratedBlock.backgroundPaint.nothingToDraw()) {
            boundingRect.offset(fXY.fX, fXY.fY);
            fCanvas->drawRect(boundingRect, decoratedBlock.backgroundPaint);
        }
        fCanvas->drawTextBlob(blob, fXY.fX, fXY.fY + fLineOffset.fY, decoratedBlock.foregroundPaint);
    }

    std::unique_ptr<WrappedText> Paint::layout(
                                       UnicodeText* unicodeText,
                                       std::u16string text,
                                       TextDirection textDirection, TextAlign textAlign,
                                       SkSize reqSize,
                                       SkSpan<FontBlock> fontBlocks) {
        auto fontResolvedText = unicodeText->resolveFonts(fontBlocks);
        auto shapedText = fontResolvedText->shape(unicodeText, textDirection);
        auto wrappedText = shapedText->wrap(unicodeText, reqSize.width(), reqSize.height());
        wrappedText->format(textAlign, textDirection);
        return wrappedText;
    }

    void Paint::paintDetailed(SkCanvas* canvas, SkPoint xy, UnicodeText* unicodeText, WrappedText* wrappedText, SkSpan<DecoratedBlock> decoratedBlocks) {
        fDecoratedBlocks = decoratedBlocks;
        fCanvas = canvas;
        fXY = xy;

        fChunks.resize(decoratedBlocks.size());
        size_t index = 0;
        for (size_t i = 0; i < decoratedBlocks.size(); ++i) {
            index += decoratedBlocks[i].charCount;
            fChunks[i] = index;
        }

        wrappedText->visit(unicodeText, this, PositionType::kGraphemeCluster, SkSpan<size_t>(fChunks.data(), fChunks.size()));
    }

    void Paint::paint(SkCanvas* canvas, SkPoint xy, UnicodeText* unicodeText, WrappedText* wrappedText, SkSpan<DecoratedBlock> decoratedBlocks) {

        SkTArray<size_t> chunks;
        chunks.resize(decoratedBlocks.size());
        size_t index = 0;
        for (size_t i = 0; i < decoratedBlocks.size(); ++i) {
            index += decoratedBlocks[i].charCount;
            chunks[i] = index;
        }
        auto drawableText = wrappedText->prepareToDraw(unicodeText, PositionType::kGraphemeCluster, SkSpan<size_t>(chunks.data(), chunks.size()));

        auto textBlobs = drawableText->getTextBlobs();
        auto textBlocks = drawableText->getChunkIndexes();
        auto baseLines = drawableText->getBaselines();
        for (auto index = 0; index < textBlobs.size(); ++index) {
            auto& textBlob = textBlobs[index];
            auto textBlock = textBlocks[index];
            auto decoratedBlock = decoratedBlocks[textBlock];
            auto baseLine = baseLines[index];
            canvas->drawTextBlob(textBlob, xy.fX, xy.fY + baseLine, decoratedBlock.foregroundPaint);
        }
    }

} // namespace text
} // namespace skia
