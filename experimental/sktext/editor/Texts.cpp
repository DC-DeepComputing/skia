// Copyright 2021 Google LLC.
#include "experimental/sktext/editor/Texts.h"

using namespace skia::text;

namespace skia {
namespace editor {

void EditableText::paint(SkCanvas* canvas) {

    if (fSelection->isEmpty()) {
        DynamicText::paint(canvas);
    } else {
        auto decorations = this->mergeSelectionIntoDecorations();
        DynamicText::paint(canvas, SkSpan<DecoratedBlock>(decorations.data(), decorations.size()));
    }
}

SkTArray<DecoratedBlock> EditableText::mergeSelectionIntoDecorations() {
    SkTArray<DecoratedBlock> merged;
    merged.reserve_back(fDecorations.size() + fSelection->count());

    size_t indexDecor = 0ul;                        // index in fDecorations
    size_t decorPos = 0ul;
    for (auto& selected : fSelection->fChunks) {
        // Add all the decoration blocks that are placed before the selected block
        DecoratedBlock& decor = fDecorations[indexDecor];
        while (indexDecor < fDecorations.size()) {
            decor = fDecorations[indexDecor++];
            if (decorPos + decor.charCount >= selected.fTextRange.fStart) {
                break;
            }
            // The entire decoration block is before
            merged.emplace_back(decor);
            decorPos += decor.charCount;
        }

        auto lastDecorPos = decorPos;
        if (selected.fTextRange.fStart > decorPos) {
            // The decoration block is has a part that is before the selection so we add it
            merged.emplace_back(selected.fTextRange.fStart - decorPos, decor.foregroundPaint, decor.backgroundPaint);
            decorPos = selected.fTextRange.fStart;
        }
        SkASSERT(decorPos == selected.fTextRange.fStart);

        // So the next decoration intesects the selection (and the selection wins)
        merged.emplace_back(selected.fTextRange.width(), fSelection->fForeground, fSelection->fBackground);
        decorPos += selected.fTextRange.width();
        SkASSERT(decorPos == selected.fTextRange.fEnd);

        if (lastDecorPos + decor.charCount > selected.fTextRange.fEnd) {
            // We still need to add the rest of the decoration block
            merged.emplace_back(lastDecorPos + decor.charCount - selected.fTextRange.fEnd, decor.foregroundPaint, decor.backgroundPaint);
            decorPos += lastDecorPos + decor.charCount - selected.fTextRange.fEnd;
        }
    }
    return merged;
}
} // namespace editor
} // namespace skia
