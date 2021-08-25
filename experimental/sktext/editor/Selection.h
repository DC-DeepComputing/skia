// Copyright 2021 Google LLC.
#ifndef Selection_DEFINED
#define Selection_DEFINED
#include <sstream>
#include "experimental/sktext/editor/Defaults.h"
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

class Selection : public SelectableText {
public:
    Selection(SkColor color) : fChunks() {
        fBackground.setColor(color);
        fBackground.setAlphaf(0.3f);
    }

    void select(TextRange range, SkRect rect);

    void clear() {
        fChunks.clear();
    }

    bool isEmpty() const { return fChunks.empty(); }
    size_t count() const { return fChunks.size(); }
    DecoratedBlock selected(size_t index) const { return DecoratedBlock(fChunks[index].fTextRange.width(), fForeground, fBackground); }

    void paint(SkCanvas* canvas, SkPoint xy);

private:
    friend class EditableText;

    struct Chunk {
        Chunk(TextRange textRange, SkRect box) : fTextRange(textRange), fGlyphBox(box) { }
        TextRange fTextRange;
        // TODO: Use glyph range, too. For now we only paint selection as a background so we do not need the glyphs
        GlyphRange fGlyphRange;
        SkRect fGlyphBox;
    };

    SkPaint fForeground;
    SkPaint fBackground;

    // List of selected chunks
    std::vector<Chunk> fChunks;
};

} // namespace editor
} // namespace skia
#endif // Selection_DEFINED
