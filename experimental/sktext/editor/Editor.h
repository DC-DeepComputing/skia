// Copyright 2021 Google LLC.
#ifndef Editor_DEFINED
#define Editor_DEFINED
#include <sstream>
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

const TextDirection DEFAULT_TEXT_DIRECTION = TextDirection::kLtr;
const TextAlign DEFAULT_TEXT_ALIGN = TextAlign::kLeft;
const SkColor DEFAULT_FOREGROUND = SK_ColorBLACK;
const SkScalar DEFAULT_CURSOR_WIDTH = 2;
const SkColor DEFAULT_CURSOR_COLOR = SK_ColorGRAY;
const SkColor DEFAULT_SELECTION_COLOR = SK_ColorCYAN;
const SkScalar DEFAULT_STATUS_HEIGHT = 20;
const SkPoint DEFAULT_TEXT_OFFSET = SkPoint::Make(0, 0);
const SkPoint DEFAULT_STATUS_OFFSET = SkPoint::Make(0, 0);

class Cursor {
public:
    static std::unique_ptr<Cursor> Make();
    Cursor();
    virtual ~Cursor() = default;
    void place(SkPoint xy, SkSize size) {
        if (size.width() < DEFAULT_CURSOR_WIDTH) {
            size.fWidth = DEFAULT_CURSOR_WIDTH;
        }
        fXY = xy;
        fSize = size;
    }

    void place(SkRect rect) {
        if (rect.width() < DEFAULT_CURSOR_WIDTH) {
            rect.fRight = rect.fLeft + DEFAULT_CURSOR_WIDTH;
        }
        fXY = SkPoint::Make(rect.fLeft, rect.fTop);
        fSize = SkSize::Make(rect.width(), rect.height());
    }

    void blink() {
        fBlink = !fBlink;
    }

    SkPoint getPosition() const { return fXY; }
    SkPoint getCenterPosition() const {
        return fXY + SkPoint::Make(0, fSize.fHeight / 2);
    }

    void paint(SkCanvas* canvas, SkPoint xy);

private:
    SkPaint fLinePaint;
    SkPaint fRectPaint;
    SkPoint fXY;
    SkSize fSize;
    bool fBlink;
};

class Mouse {
    const SkMSec MAX_DBL_TAP_INTERVAL = 300;
    const float MAX_DBL_TAP_DISTANCE = 100;
public:
    Mouse() : fMouseDown(false), fLastTouchPoint(), fLastTouchTime() { }

    void down();
    void up();
    void clearTouchInfo() {
        fLastTouchPoint = SkPoint::Make(0, 0);
        fLastTouchTime = 0.0;
    }
    bool isDown() { return fMouseDown; }
    bool isDoubleClick(SkPoint touch);

private:
    bool fMouseDown;
    SkPoint fLastTouchPoint;
    double fLastTouchTime;
};

class Selection {
public:
    Selection(SkColor color) : fGlyphBoxes(), fTextRanges() {
        fPaint.setColor(color);
        fPaint.setAlphaf(0.3f);
    }
    void select(TextRange range, SkRect rect);
    void clear() {
        fGlyphBoxes.clear();
        fTextRanges.clear();
    }
    void paint(SkCanvas* canvas, SkPoint xy);
private:
    SkPaint fPaint;
    std::vector<SkRect> fGlyphBoxes;
    std::vector<TextRange> fTextRanges;
};

struct Style {
    sk_sp<SkTypeface> fTypeface;
    SkScalar fFontSize;
    SkFontStyle fFontStyle;
    SkColor fForeground;
    size_t fLength;

    std::string toString() {
        SkString ff;
        fTypeface->getFamilyName(&ff);
        std::stringstream ss;
        ss << ff.c_str() << ", "
           << fFontSize
           << (fFontStyle.slant() == SkFontStyle::Slant::kItalic_Slant ? ", italic " : "");
        if (fFontStyle.weight() != SkFontStyle::Weight::kNormal_Weight) {
            ss << ", " << fFontStyle.weight();
        }
        return ss.str();
    }
};

class DisplayText {
public:
    DisplayText(SkPoint offset, SkScalar width) {
        fOffset = offset;
        fSize = SkSize::Make(width, SK_ScalarInfinity);
        fText = u"";
        fBlocks = SkSpan<Block>(nullptr, 0);
        fUnicodeText = nullptr;
        fShapedText = nullptr;
        fWrappedText = nullptr;
        fFormattedText = nullptr;
    }

    DisplayText(std::u16string text, SkPoint offset, SkSize size, SkSpan<Block> fontBlocks, TextDirection textDirection, TextAlign textAlign) {
        fOffset = offset;
        fSize = size;
        fBlocks = fontBlocks;
        fText = std::move(text);
        fUnicodeText = Text::parse(SkSpan<uint16_t>((uint16_t*)fText.data(), fText.size()));
        fShapedText = fUnicodeText->shape(fontBlocks, DEFAULT_TEXT_DIRECTION);
        fWrappedText = fShapedText->wrap(size.width(), size.height(), fUnicodeText->getUnicode());
        fFormattedText = fWrappedText->format(DEFAULT_TEXT_ALIGN, DEFAULT_TEXT_DIRECTION);
    }

    bool update(std::u16string text) {
        if (!this->fBlocks.empty()) {
            SkASSERT(this->fBlocks.size() == 1);
            this->fBlocks[0].charCount = text.size();
        }
        this->fText = std::move(text);
        this->fUnicodeText = Text::parse(SkSpan<uint16_t>((uint16_t*)this->fText.data(), this->fText.size()));
        this->fShapedText = fUnicodeText->shape(this->fBlocks, DEFAULT_TEXT_DIRECTION);
        this->fWrappedText = fShapedText->wrap(this->fSize.fWidth, this->fSize.fHeight, this->fUnicodeText->getUnicode());
        this->fFormattedText = fWrappedText->format(DEFAULT_TEXT_ALIGN, DEFAULT_TEXT_DIRECTION);
        return true;
    }

    std::u16string fText;
    std::unique_ptr<UnicodeText> fUnicodeText;
    std::unique_ptr<ShapedText> fShapedText;
    std::unique_ptr<WrappedText> fWrappedText;
    sk_sp<FormattedText> fFormattedText;
    SkSize fSize;
    SkPoint fOffset;
    SkSpan<Block> fBlocks;
};

class EditText : DisplayText {

};

class Editor : public sk_app::Window::Layer, FormattedText::Visitor {
public:
    static std::unique_ptr<Editor> Make(std::u16string text, SkSize size, SkSpan<Block> fontBlocks);
    static std::unique_ptr<Editor> MakeDemo(SkScalar width);

    Editor(std::u16string text, SkSize size, SkSpan<Block> fontBlocks);
    ~Editor() override = default;

    void paint(SkCanvas* canvas, SkPoint point);
    void blink() { fCursor->blink(); }
    void onResize(int width, int height) override;
private:

    void updateText(std::u16string text);

    void onAttach(sk_app::Window* w) override { fParent = w; }
    void onPaint(SkSurface* surface) override;

    bool onMouse(int x, int y, skui::InputState state, skui::ModifierKey modifiers) override;
    bool onKey(skui::Key, skui::InputState, skui::ModifierKey) override;
    bool onChar(SkUnichar c, skui::ModifierKey modifier) override;
    void invalidate() { if (fParent) { fParent->inval(); } }

    void onBeginLine(TextRange, float baselineY) override;
    void onEndLine(TextRange, float baselineY) override;
    void onGlyphRun(SkFont font,
                    TextRange textRange,
                    int glyphCount,
                    const uint16_t glyphs[],
                    const SkPoint  positions[],
                    const SkPoint offsets[]) override;
    void onPlaceholder(TextRange, const SkRect& bounds) override;

    bool moveCursor(skui::Key key);
    bool insertCodepoint(SkUnichar unichar);
    bool deleteElement(skui::Key key);

    std::unique_ptr<Cursor> fCursor;
    std::unique_ptr<Mouse> fMouse;
    std::unique_ptr<Selection> fSelection;

    DisplayText fParagraph;
    DisplayText fStatus;

    SkCanvas* fCanvas;
    SkPoint fXY;
    sk_app::Window* fParent;
    int fWidth;
    int fHeight;

    PositionType fDefaultPositionType;
    bool fInsertMode;
};

} // namespace editor
} // namespace skia
#endif // Editor_DEFINED
