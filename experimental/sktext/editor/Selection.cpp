// Copyright 2021 Google LLC.
#include "experimental/sktext/editor/Selection.h"

using namespace skia::text;

namespace skia {
namespace editor {

void Selection::select(TextRange textRange, SkRect rect) {
    fChunks.clear();
    fChunks.emplace_back(textRange, rect);
}

void Selection::paint(SkCanvas* canvas, SkPoint xy) {
    for (auto& chunk : fChunks) {
        canvas->drawRect(
                SkRect::MakeXYWH(chunk.fGlyphBox.fLeft + xy.fX, chunk.fGlyphBox.fTop + xy.fY, chunk.fGlyphBox.width(), chunk.fGlyphBox.height()),
                fBackground);
    }
}

} // namespace editor
} // namespace skia
