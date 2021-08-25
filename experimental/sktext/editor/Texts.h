// Copyright 2021 Google LLC.
#ifndef Texts_DEFINED
#define Texts_DEFINED
#include <sstream>
#include "experimental/sktext/editor/Cursor.h"
#include "experimental/sktext/editor/Defaults.h"
#include "experimental/sktext/editor/Mouse.h"
#include "experimental/sktext/editor/Selection.h"
#include "experimental/sktext/include/Text.h"
#include "experimental/sktext/include/Types.h"
#include "experimental/sktext/src/Paint.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"
#include "tools/skui/ModifierKey.h"

namespace skia {
namespace editor {

using namespace skia::text;
// Shapes text once and then paints it at request (not keeping intermediate data)
// TODO: Keep only text blobs
class StaticText {
public:

    StaticText(std::u16string text, SkPoint offset, SkSize size, SkSpan<FontBlock> fontBlocks, SkPaint* backgroundPaint, TextDirection textDirection, TextAlign textAlign) {
        fOffset = offset;
        fSize = size;
        fFontBlocks = fontBlocks;
        fBackgroundPaint = backgroundPaint;
        fText = std::move(text);
        auto unicodeText = std::unique_ptr<UnicodeText>(new UnicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)fText.data(), fText.size())));
        auto shapedText = unicodeText->shape(fontBlocks, DEFAULT_TEXT_DIRECTION);
        auto wrappedText = shapedText->wrap(unicodeText.get(), size.width(), size.height());
        wrappedText->format(DEFAULT_TEXT_ALIGN, DEFAULT_TEXT_DIRECTION);

        fDrawableText = wrappedText->prepareToDraw(PositionType::kGraphemeCluster, {});
    }

    virtual ~StaticText() = default;

    virtual void paint(SkCanvas* canvas) const {
        auto textBlobs = fDrawableText->getTextBlobs();
        for (auto& textBlob : textBlobs) {
            canvas->drawTextBlob(textBlob.get(), 0, 0, *fBackgroundPaint);
        }
    }

    std::u16string fText;
    std::unique_ptr<DrawableText> fDrawableText;
    SkPaint* fBackgroundPaint;
    SkSize fSize;
    SkPoint fOffset;
    SkSpan<FontBlock> fFontBlocks;
};

// The text can be navigated but not selected
class DynamicText : public Visitor {
public:
    DynamicText(std::u16string text,
                SkPoint offset, SkSize size,
                SkSpan<FontBlock> fontBlocks,
                SkSpan<DecoratedBlock> decorations,
                TextDirection textDirection, TextAlign textAlign) {
        fOffset = offset;
        fRequiredSize = size;
        fFontBlocks = fontBlocks;
        fDecorations = decorations;
        fTextAlign = textAlign;
        fTextDirection = textDirection;

        this->rebuild(text);
    }

    virtual ~DynamicText() = default;

    virtual void onGlyphRun(const SkFont& font,
                    TextRange textRange,
                    SkRect boundingRect,
                    int glyphCount,
                    const uint16_t glyphs[],
                    const SkPoint positions[]) override {

        auto decoration = this->findDecoration(textRange);
        SkTextBlobBuilder builder;
        const auto& blobBuffer = builder.allocRunPos(font, SkToInt(glyphCount));
        sk_careful_memcpy(blobBuffer.glyphs, glyphs, glyphCount * sizeof(uint16_t));
        sk_careful_memcpy(blobBuffer.points(), positions, glyphCount * sizeof(SkPoint));
        auto textBlob = builder.make();
    }

    bool contains(SkScalar x, SkScalar y) {
        return SkRect::MakeXYWH(fOffset.fX, fOffset.fY, fRequiredSize.fWidth, fRequiredSize.fHeight).contains(x, y);
    }

    void invalidate() { fInvalidated = true; }
    bool isInvalidated() { return fInvalidated; }

    DecoratedBlock findDecoration(TextRange textRange);

    bool rebuild(std::u16string text) {
        if (!this->fFontBlocks.empty()) {
            SkASSERT(this->fFontBlocks.size() == 1);
            this->fFontBlocks[0].charCount = text.size();
        }
        fText = std::move(text);
        auto unicodeText = std::unique_ptr<UnicodeText>(new UnicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)fText.data(), fText.size())));
        auto shapedText = unicodeText->shape(fFontBlocks, fTextDirection);
        fWrappedText = shapedText->wrap(unicodeText.get(), fRequiredSize.fWidth, fRequiredSize.fHeight);
        fWrappedText->format(fTextAlign, fTextDirection);
        fActualSize = fWrappedText->actualSize();
        fInvalidated = false;

        fSelectableText = fWrappedText->prepareToEdit(unicodeText.get(), PositionType::kGraphemeCluster);

        return true;
    }

    SkRect actualSize() const {
        return SkRect::MakeXYWH(fOffset.fX, fOffset.fY, fActualSize.fWidth, fActualSize.fHeight);
    }

    // TODO: Deal with decorations
    virtual void paint(SkCanvas* canvas) {
        SkASSERT(fDecorations.size() == 1);
        auto drawableText = fWrappedText->prepareToDraw(PositionType::kGraphemeCluster, SkSpan<TextIndex>(0, fText.size()));
        auto textBlobs = drawableText->getTextBlobs();
        for (auto& textBlob : textBlobs) {
            canvas->drawTextBlob(textBlob.get(), 0, 0, fDecorations[0].backgroundPaint);
        }
    }

    // TODO: Deal with decorations
    virtual void paint(SkCanvas* canvas, SkSpan<DecoratedBlock> decorations) {
        auto drawableText = fWrappedText->prepareToDraw(PositionType::kGraphemeCluster, SkSpan<TextIndex>(0, fText.size()));
        auto textBlobs = drawableText->getTextBlobs();
        for (auto& textBlob : textBlobs) {
            canvas->drawTextBlob(textBlob.get(), 0, 0, fDecorations[0].backgroundPaint);
        }
    }

protected:
    std::u16string fText;
    std::unique_ptr<WrappedText> fWrappedText;
    std::unique_ptr<SelectableText> fSelectableText;
    SkSize fRequiredSize;
    SkSize fActualSize;
    SkPoint fOffset;
    SkSpan<FontBlock> fFontBlocks;
    SkSpan<DecoratedBlock> fDecorations;
    TextAlign fTextAlign;
    TextDirection fTextDirection;
    bool fInvalidated;
};

// Text can change;  supports select/copy/paste
class EditableText : public DynamicText {
public:
    EditableText(std::u16string text, SkPoint offset, SkSize size, SkSpan<FontBlock> fontBlocks, SkSpan<DecoratedBlock> decorations, TextDirection textDirection, TextAlign textAlign)
        : DynamicText(text, offset, size, fontBlocks, decorations, textDirection, textAlign)
        , fSelection(std::make_unique<Selection>(DEFAULT_SELECTION_COLOR)) {
    }

    bool isEmpty() { return fText.empty(); }
/*
    Position adjustedPosition(PositionType positionType, SkPoint point) const {
        return fWrappedText->adjustedPosition(positionType, point - fOffset);
    }

    Position adjustedPosition(PositionType positionType, TextIndex textIndex) const {
        return fWrappedText->adjustedPosition(positionType, textIndex);
    }

    bool recalculateBoundaries(Position& position) const {
        return fWrappedText->recalculateBoundaries(position);
    }

    const Line* line(size_t index) const { return fSelectableText->line(index); }
*/
    Position previousElement(Position element) const { return fSelectableText->previousElement(element); }
    Position nextElement(Position current) const { return fSelectableText->nextElement(current); }
    Position firstElement(PositionType positionType) const { return fSelectableText->firstElement(positionType); }
    Position lastElement(PositionType positionType) const { return fSelectableText->lastElement(positionType); }

    bool isFirstOnTheLine(Position element) const { return fSelectableText->isFirstOnTheLine(element); }
    bool isLastOnTheLine(Position element) const { return fSelectableText->isLastOnTheLine(element); }

    void removeElement(TextRange toRemove) {
        std::u16string text;
        text.append(this->fText.substr(0, toRemove.fStart));
        text.append(this->fText.substr(toRemove.fEnd, std::u16string::npos));
        update(text);
    }

    void insertElement(SkUnichar unichar, TextIndex toInsert) {
        std::u16string text;
        text.append(fText.substr(0, toInsert));
        text.append(1, (unsigned)unichar);
        text.append(fText.substr(toInsert, std::u16string::npos));
        update(text);
    }

    void replaceElement(SkUnichar unichar, TextRange toReplace) {
        std::u16string text;
        text.append(fText.substr(0, toReplace.fStart));
        text.append(1, (unsigned)unichar);
        text.append(fText.substr(toReplace.fEnd, std::u16string::npos));
    }

    void update(std::u16string& text) {
        fText = text;
        // TODO: Update text styles
        SkASSERT(fFontBlocks.size() == 1);
        fFontBlocks[0].charCount = fText.size();
        SkASSERT(fDecorations.size() == 1);
        fDecorations[0].charCount = fText.size();
        // TODO: update all objects rather than recreate them
        rebuild(text);

        // TODO: Update the text selection
        fSelection->clear();
    }

    void select(TextRange text, SkRect boundaries) { fSelection->select(text, boundaries); }
    void clearSelection() { fSelection->clear(); }

    void paint(SkCanvas* canvas) override;

    SkTArray<DecoratedBlock> mergeSelectionIntoDecorations();

protected:
    std::unique_ptr<Selection> fSelection;
};

} // namespace editor
} // namespace skia
#endif // Texts_DEFINED
