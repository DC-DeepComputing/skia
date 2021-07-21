// Copyright 2021 Google LLC.
#include "experimental/sktext/editor/Editor.h"
#include "experimental/sktext/src/Paint.h"

namespace skia {
namespace editor {

std::unique_ptr<Cursor> Cursor::Make() { return std::make_unique<Cursor>(); }

Cursor::Cursor() {
    fLinePaint.setColor(SK_ColorGRAY);
    fLinePaint.setAntiAlias(true);

    fRectPaint.setColor(DEFAULT_CURSOR_COLOR);
    fRectPaint.setStyle(SkPaint::kStroke_Style);
    fRectPaint.setStrokeWidth(2);
    fRectPaint.setAntiAlias(true);

    fXY = SkPoint::Make(0, 0);
    fSize = SkSize::Make(0, 0);
    fBlink = true;
}

void Cursor::paint(SkCanvas* canvas, SkPoint xy) {

    if (fBlink) {
       canvas->drawRect(SkRect::MakeXYWH(fXY.fX + xy.fX, fXY.fY + xy.fY, DEFAULT_CURSOR_WIDTH, fSize.fHeight),
                fRectPaint);
    } else {
        //canvas->drawLine(fXY + xy, fXY + xy + SkPoint::Make(1, fSize.fHeight), fLinePaint);
    }
}

void Mouse::down() {
    fMouseDown = true;
}

void Mouse::up() {
    fMouseDown = false;
}

bool Mouse::isDoubleClick(SkPoint touch) {
    if ((touch - fLastTouchPoint).length() > MAX_DBL_TAP_DISTANCE) {
        fLastTouchPoint = touch;
        fLastTouchTime = SkTime::GetMSecs();
        return false;
    }
    double now = SkTime::GetMSecs();
    if (now - fLastTouchTime > MAX_DBL_TAP_INTERVAL) {
        fLastTouchPoint = touch;
        fLastTouchTime = SkTime::GetMSecs();
        return false;
    }

    clearTouchInfo();
    return true;
}

void Selection::select(TextRange range, SkRect rect) {
    fGlyphBoxes.clear();
    fTextRanges.clear();

    fGlyphBoxes.emplace_back(rect);
    fTextRanges.emplace_back(range);
}

void Selection::paint(SkCanvas* canvas, SkPoint xy) {
    for (auto& box : fGlyphBoxes) {
        canvas->drawRect(
                SkRect::MakeXYWH(box.fLeft + xy.fX, box.fTop + xy.fY, box.width(), box.height()),
                fPaint);
    }
}

std::unique_ptr<Editor> Editor::Make(std::u16string text, SkSize size, SkSpan<Block> fontBlocks) {
    return std::unique_ptr<Editor>(new Editor(text, size, fontBlocks));
}

Editor::Editor(std::u16string text, SkSize size, SkSpan<Block> fontBlocks)
        : fParagraph(text, size, fontBlocks, DEFAULT_TEXT_DIRECTION, DEFAULT_TEXT_ALIGN)
        , fStatus(size.fWidth)
        , fDefaultPositionType(PositionType::kGraphemeCluster)
        , fInsertMode(true) {

    fParent = nullptr;
    fCursor = Cursor::Make();
    fMouse = std::make_unique<Mouse>();
    fSelection = std::make_unique<Selection>(DEFAULT_SELECTION_COLOR);

    // Place the cursor at the end of the output text
    // (which is the end of the text for LTR and the beginning of the text for RTL
    // or possibly something in the middle for a combination of LTR & RTL)
    // In order to get that position we look for a position outside of the text
    // and that will give us the last glyph on the line
    auto endOfText = fParagraph.fFormattedText->lastElement(fDefaultPositionType);
    fParagraph.fFormattedText->recalculateBoundaries(endOfText);
    fCursor->place(endOfText.fBoundaries);
}

void Editor::updateText(std::u16string text) {
    // TODO: Update text styles
    SkASSERT(fParagraph.fBlocks.size() == 1);
    fParagraph.fBlocks[0].charCount = fParagraph.fText.size();
    // TODO: update all objects rather than recreate them
    fParagraph.update(text);

    // Update the (shift it to point at the grapheme edge)
    auto position = fParagraph.fFormattedText->adjustedPosition(fDefaultPositionType, fCursor->getCenterPosition());
    fParagraph.fFormattedText->recalculateBoundaries(position);
    fCursor->place(position.fBoundaries);

    // TODO: Update the mouse
    fMouse->clearTouchInfo();

    // TODO: Update the text selection
    fSelection->clear();
}

// Moving the cursor by the output grapheme clusters (shifting to another line if necessary)
// We don't want to move by the input text indexes because then we will have to take in account LTR/RTL
bool Editor::moveCursor(skui::Key key) {
    auto cursorPosition = fCursor->getCenterPosition();
    auto position = fParagraph.fFormattedText->adjustedPosition(PositionType::kGraphemeCluster, cursorPosition);
    if (position.fRun == nullptr) {
        return false;
    }
    if (key == skui::Key::kLeft) {
        position = fParagraph.fFormattedText->previousElement(position);
    } else if (key == skui::Key::kRight) {
        position = fParagraph.fFormattedText->nextElement(position);
    } else if (key == skui::Key::kHome) {
        position = fParagraph.fFormattedText->firstElement(PositionType::kGraphemeCluster);
    } else if (key == skui::Key::kEnd) {
        position = fParagraph.fFormattedText->lastElement(PositionType::kGraphemeCluster);
    } else if (key == skui::Key::kUp) {
        // Move one line up (if possible)
        if (position.fLineIndex == 0) {
            return false;
        }
        auto line = fParagraph.fFormattedText->line(position.fLineIndex);
        cursorPosition.offset(0, - line->getMetrics().height());
        position = fParagraph.fFormattedText->adjustedPosition(PositionType::kGraphemeCluster, cursorPosition);
    } else if (key == skui::Key::kDown) {
        // Move one line down (if possible)
        if (position.fLineIndex == fParagraph.fFormattedText->countLines() - 1) {
            return false;
        }
        auto line = fParagraph.fFormattedText->line(position.fLineIndex);
        cursorPosition.offset(0, line->getMetrics().height());
        position = fParagraph.fFormattedText->adjustedPosition(PositionType::kGraphemeCluster, cursorPosition);
     }

    // Place the cursor at the new position
    fParagraph.fFormattedText->recalculateBoundaries(position);
    fCursor->place(position.fBoundaries);
    this->invalidate();

    return true;
}

void Editor::onPaint(SkSurface* surface) {
    SkCanvas* canvas = surface->getCanvas();
    SkAutoCanvasRestore acr(canvas, true);
    canvas->clipRect({0, 0, (float)fWidth, (float)fHeight});
    canvas->drawColor(SK_ColorWHITE);
    this->paint(canvas, SkPoint::Make(0, 0));
}

void Editor::onResize(int width, int height) {
    if (SkISize{fWidth, fHeight} != SkISize{width, height}) {
        fHeight = height;
        if (width != fWidth) {
            fWidth = width;
        }
        this->invalidate();
    }
}

bool Editor::onChar(SkUnichar c, skui::ModifierKey modi) {
    using sknonstd::Any;

    fSelection->clear();

    modi &= ~skui::ModifierKey::kFirstPress;
    if (!Any(modi & (skui::ModifierKey::kControl | skui::ModifierKey::kOption |
                     skui::ModifierKey::kCommand))) {
        if (((unsigned)c < 0x7F && (unsigned)c >= 0x20) || c == 0x000A) {
            insertCodepoint(c);
            return true;
        }
    }
    static constexpr skui::ModifierKey kCommandOrControl =
            skui::ModifierKey::kCommand | skui::ModifierKey::kControl;
    if (Any(modi & kCommandOrControl) && !Any(modi & ~kCommandOrControl)) {
        return false;
    }
    return false;
}

bool Editor::deleteElement(skui::Key key) {

    if (fParagraph.fText.empty()) {
        return false;
    }

    auto cursorPosition = fCursor->getCenterPosition();
    auto position = fParagraph.fFormattedText->adjustedPosition(fDefaultPositionType, cursorPosition);

    // IMPORTANT: We assume that a single element (grapheme cluster) does not cross the run boundaries;
    // It's not exactly true but we are going to enforce in by breaking the grapheme by the run boundaries
    if (key == skui::Key::kBack) {
        // Move to the previous element (could be another run)
        position = fParagraph.fFormattedText->previousElement(position);
    }

    // Remove the grapheme
    std::u16string text;
    text.append(fParagraph.fText.substr(0, position.fTextRange.fStart));
    text.append(fParagraph.fText.substr(position.fTextRange.fEnd, std::u16string::npos));
    updateText(text);

    // Find the grapheme the cursor points to
    position = fParagraph.fFormattedText->adjustedPosition(fDefaultPositionType, cursorPosition);
    fCursor->place(position.fBoundaries);
    this->invalidate();

    return true;
}

bool Editor::insertCodepoint(SkUnichar unichar) {
    auto cursorPosition = fCursor->getCenterPosition();
    auto position = fParagraph.fFormattedText->adjustedPosition(fDefaultPositionType, cursorPosition);

    std::u16string text;
    text.append(fParagraph.fText.substr(0, position.fTextRange.fStart));
    text.append(1, (unsigned)unichar);
    if (fInsertMode) {
        text.append(fParagraph.fText.substr(position.fTextRange.fStart, std::u16string::npos));
    } else {
        text.append(fParagraph.fText.substr(position.fTextRange.fEnd, std::u16string::npos));
    }
    updateText(text);

    // Find the element the cursor points to
    position = fParagraph.fFormattedText->adjustedPosition(fDefaultPositionType, cursorPosition);

    // Move the cursor to the next element
    position = fParagraph.fFormattedText->nextElement(position);
    fParagraph.fFormattedText->recalculateBoundaries(position);
    fCursor->place(position.fBoundaries);

    this->invalidate();

    return true;
}

bool Editor::onKey(skui::Key key, skui::InputState state, skui::ModifierKey modifiers) {

    fSelection->clear();

    if (state != skui::InputState::kDown) {
        return false;
    }
    using sknonstd::Any;
    skui::ModifierKey ctrlAltCmd = modifiers & (skui::ModifierKey::kControl |
                                                skui::ModifierKey::kOption  |
                                                skui::ModifierKey::kCommand);
    //bool shift = Any(modifiers & (skui::ModifierKey::kShift));
    if (!Any(ctrlAltCmd)) {
        // no modifiers
        switch (key) {
            case skui::Key::kLeft:
            case skui::Key::kRight:
            case skui::Key::kUp:
            case skui::Key::kDown:
            case skui::Key::kHome:
            case skui::Key::kEnd:
                this->moveCursor(key);
                break;
            case skui::Key::kDelete:
            case skui::Key::kBack:
                this->deleteElement(key);
                return true;
            case skui::Key::kOK:
                return this->onChar(0x000A, modifiers);
            default:
                break;
        }
    }
    return false;
}

bool Editor::onMouse(int x, int y, skui::InputState state, skui::ModifierKey modifiers) {

    //bool shiftOrDrag = sknonstd::Any(modifiers & skui::ModifierKey::kShift) || (skui::InputState::kDown != state);
    if (skui::InputState::kDown == state) {
        auto position = fParagraph.fFormattedText->adjustedPosition(fDefaultPositionType, SkPoint::Make(x, y));
        if (fMouse->isDoubleClick(SkPoint::Make(x, y))) {
            fSelection->select(position.fTextRange, position.fBoundaries);
            position.fBoundaries.fLeft = position.fBoundaries.fRight - DEFAULT_CURSOR_WIDTH;
        } else {
            fMouse->down();
            fSelection->clear();
        }

        fCursor->place(position.fBoundaries);
        this->invalidate();
        return true;
    }
    fMouse->up();
    return false;
}

void Editor::onBeginLine(TextRange, float baselineY) {}
void Editor::onEndLine(TextRange, float baselineY) {}
void Editor::onPlaceholder(TextRange, const SkRect& bounds) {}
void Editor::onGlyphRun(SkFont font,
                        TextRange textRange,
                        int glyphCount,
                        const uint16_t glyphs[],
                        const SkPoint positions[],
                        const SkPoint offsets[]) {
    SkTextBlobBuilder builder;
    const auto& blobBuffer = builder.allocRunPos(font, SkToInt(glyphCount));
    sk_careful_memcpy(blobBuffer.glyphs, glyphs, glyphCount * sizeof(uint16_t));
    sk_careful_memcpy(blobBuffer.points(), positions, glyphCount * sizeof(SkPoint));
    SkPaint foreground;
    foreground.setColor(DEFAULT_FOREGROUND);
    fCanvas->drawTextBlob(builder.make(), fXY.fX, fXY.fY, foreground);
}

void Editor::paint(SkCanvas* canvas, SkPoint xy) {
    fCanvas = canvas;
    fXY = xy;
    this->fParagraph.fFormattedText->visit(this);

    fSelection->paint(canvas, xy);
    fCursor->paint(canvas, xy);
}

std::unique_ptr<Editor> Editor::MakeDemo(SkScalar width) {

    std::u16string text0 = u"In a hole in the ground there lived a hobbit. Not a nasty, dirty, "
                            "wet hole full of worms and oozy smells. This was a hobbit-hole and "
                            "that means good food, a warm hearth, and all the comforts of home.";

    static Block block(text0.size(), sk_make_sp<TrivialFontChain>("Roboto", 40));
    return Editor::Make(text0, SkSize::Make(width, SK_ScalarInfinity), SkSpan<Block>(&block, 1));
}
} // namespace text
} // namespace skia
