/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/SkottieTextEditor.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkM44.h"
#include "include/core/SkPath.h"
#include "include/core/SkString.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkUTF.h"

namespace {

SkPath make_cursor_path() {
    // Normalized values, relative to text/font size.
    constexpr float kWidth  = 0.2f,
                    kHeight = 0.75f;

    SkPath p;

    p.lineTo(kWidth  , 0);
    p.moveTo(kWidth/2, 0);
    p.lineTo(kWidth/2, kHeight);
    p.moveTo(0       , kHeight);
    p.lineTo(kWidth  , kHeight);

    return p;
}

size_t char_index(const SkString& str, size_t utf8_index) {
    const char* utf8     = str.c_str();
    const char* utf8_end = utf8 + str.size();

    SkASSERT(utf8 + utf8_index < utf8_end);

    size_t cindex = 0;

    while (utf8 != str.c_str() + utf8_index) {
        SkASSERT(utf8 < str.c_str() + utf8_index);
        SkAssertResult(SkUTF::NextUTF8(&utf8, utf8_end) >= 0);
        cindex++;
    }

    return cindex;
}

size_t utf8_index(const SkString& str, size_t char_index) {
    const char* utf8     = str.c_str();
    const char* utf8_end = utf8 + str.size();

    for (size_t i = 0; i < char_index; ++i) {
        SkAssertResult(SkUTF::NextUTF8(&utf8, utf8_end) >= 0);
    }

    return utf8 - str.c_str();
}

} // namespace

SkottieTextEditor::SkottieTextEditor(
        std::unique_ptr<skottie::TextPropertyHandle>&& prop,
        std::vector<std::unique_ptr<skottie::TextPropertyHandle>>&& deps)
    : fTextProp(std::move(prop))
    , fDependentProps(std::move(deps))
    , fCursorPath(make_cursor_path())
    , fCursorBounds(fCursorPath.computeTightBounds())
{}

SkottieTextEditor::~SkottieTextEditor() = default;

void SkottieTextEditor::toggleEnabled() {
    fEnabled = !fEnabled;

    auto txt = fTextProp->get();
    txt.fDecorator = fEnabled ? sk_ref_sp(this) : nullptr;
    fTextProp->set(txt);

    if (fEnabled) {
        // Reset the cursor position at the end.
        fCursorIndex = SkUTF::CountUTF8(txt.fText.c_str(), txt.fText.size());
    }

    fTimeBase = std::chrono::steady_clock::now();
}

std::tuple<size_t, size_t> SkottieTextEditor::currentSelection() const {
    // Selection can be inverted.
    return std::make_tuple(std::min(std::get<0>(fSelection), std::get<1>(fSelection)),
                           std::max(std::get<0>(fSelection), std::get<1>(fSelection)));
}

size_t SkottieTextEditor::closestGlyph(const SkPoint& pt) const {
    float  min_distance = std::numeric_limits<float>::max();
    size_t min_index    = 0;

    for (size_t i = 0; i < fGlyphData.size(); ++i) {
        const auto dist = (fGlyphData[i].fDevBounds.center() - pt).length();
        if (dist < min_distance) {
            min_distance = dist;
            min_index = i;
        }
    }

    return min_index;
}

void SkottieTextEditor::drawCursor(SkCanvas* canvas, const GlyphInfo glyphs[], size_t size) const {
    constexpr double kCursorHz = 2;
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now() - fTimeBase).count();
    const long cycle = static_cast<long>(static_cast<double>(now_ms) * 0.001 * kCursorHz);
    if (cycle & 1) {
        // blink
        return;
    }

    auto txt_prop  = fTextProp->get();

    const auto glyph_index = [&]() -> size_t {
        if (!fCursorIndex) {
            return 0;
        }

        const auto utf8_cursor_index = utf8_index(txt_prop.fText, fCursorIndex - 1);

        for (size_t i = 0; i < size; ++i) {
            if (glyphs[i].fCluster >= utf8_cursor_index) {
                return i;
            }
        }

        return size - 1;
    }();

    const auto& glyph_bounds = glyphs[glyph_index].fBounds;

    // Cursor index mapping:
    //   0 -> before the first char
    //   1 -> after the first char
    //   2 -> after the second char
    //   ...
    // The cursor is bottom-aligned, and centered to the right/left edge of the glyph bounding box.
    const auto cscale = txt_prop.fTextSize,
                cxpos = (fCursorIndex ? glyph_bounds.fRight : glyph_bounds.fLeft)
                         - fCursorBounds.width() * cscale * 0.5f,
                cypos = glyph_bounds.fBottom - fCursorBounds.height() * cscale;
    const auto cpath  = fCursorPath.makeTransform(SkMatrix::Translate(cxpos, cypos) *
                                                  SkMatrix::Scale(cscale, cscale));

    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeCap(SkPaint::kRound_Cap);

    SkAutoCanvasRestore acr(canvas, true);
    canvas->concat(glyphs[glyph_index].fMatrix);

    p.setColor(SK_ColorWHITE);
    p.setStrokeWidth(3);
    canvas->drawPath(cpath, p);
    p.setColor(SK_ColorBLACK);
    p.setStrokeWidth(2);
    canvas->drawPath(cpath, p);
}

void SkottieTextEditor::updateDeps(const SkString& txt) {
    for (const auto& dep : fDependentProps) {
        auto txt_prop = dep->get();
        txt_prop.fText = txt;
        dep->set(txt_prop);
    }
}

void SkottieTextEditor::insertChar(SkUnichar c) {
    auto txt = fTextProp->get();

    const auto utf8_offset = utf8_index(txt.fText, fCursorIndex++);

    txt.fText.insertUnichar(utf8_offset, c);
    fTextProp->set(txt);
    this->updateDeps(txt.fText);
}

void SkottieTextEditor::deleteChars(size_t offset, size_t count) {
    auto txt = fTextProp->get();

    txt.fText.remove(offset, count);
    fTextProp->set(txt);
    this->updateDeps(txt.fText);

    fCursorIndex = offset < txt.fText.size()
        ? char_index(txt.fText, offset)
        : SkUTF::CountUTF8(txt.fText.c_str(), txt.fText.size());
}

bool SkottieTextEditor::deleteSelection() {
    const auto [glyph_sel_start, glyph_sel_end] = this->currentSelection();
    if (glyph_sel_start == glyph_sel_end) {
        return false;
    }

    const auto utf8_sel_start = fGlyphData[glyph_sel_start].fCluster,
               utf8_sel_end   = fGlyphData[glyph_sel_end  ].fCluster;
    SkASSERT(utf8_sel_start < utf8_sel_end);

    this->deleteChars(utf8_sel_start, utf8_sel_end - utf8_sel_start);

    fSelection = {0,0};

    return true;
}

void SkottieTextEditor::onDecorate(SkCanvas* canvas, const GlyphInfo glyphs[], size_t size) {
    const auto [sel_start, sel_end] = this->currentSelection();

    fGlyphData.clear();

    for (size_t i = 0; i < size; ++i) {
        const auto& ginfo = glyphs[i];

        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(ginfo.fMatrix);

        // Stash some glyph info, for later use.
        fGlyphData.push_back({
            canvas->getLocalToDevice().asM33().mapRect(ginfo.fBounds),
            ginfo.fCluster
        });

        if (i < sel_start || i >= sel_end) {
            continue;
        }

        static constexpr SkColor4f kSelectionColor{0, 0, 1, 0.4f};
        canvas->drawRect(ginfo.fBounds, SkPaint(kSelectionColor));
    }

    // Only draw the cursor when there's no active selection.
    if (sel_start == sel_end) {
        this->drawCursor(canvas, glyphs, size);
    }
}

bool SkottieTextEditor::onMouseInput(SkScalar x, SkScalar y, skui::InputState state,
                                     skui::ModifierKey) {
    if (!fEnabled || fGlyphData.empty()) {
        return false;
    }

    switch (state) {
    case skui::InputState::kDown: {
        fMouseDown = true;

        const auto closest = this->closestGlyph({x, y});
        fSelection = {closest, closest};
    }   break;
    case skui::InputState::kUp:
        fMouseDown = false;
        break;
    case skui::InputState::kMove:
        if (fMouseDown) {
            const auto closest = this->closestGlyph({x, y});
            std::get<1>(fSelection) = closest < std::get<0>(fSelection)
                                            ? closest
                                            : closest + 1;
        }
        break;
    default:
        break;
    }

    return true;
}

bool SkottieTextEditor::onCharInput(SkUnichar c) {
    if (!fEnabled || fGlyphData.empty()) {
        return false;
    }

    // Natural editor bindings are currently intercepted by Viewer, so we use these instead.
    switch (c) {
    case '|':     // commit changes and exit editing mode
        this->toggleEnabled();
        break;
    case ']': {   // move right
        const auto& txt_str = fTextProp->get().fText;
        if (fCursorIndex < SkToSizeT(SkUTF::CountUTF8(txt_str.c_str(), txt_str.size()))) {
            fCursorIndex++;
        }
    } break;
    case '[':     // move left
        if (fCursorIndex > 0) {
            fCursorIndex--;
        }
        break;
    case '\\': {  // delete
        if (!this->deleteSelection() && fCursorIndex) {
            const SkString& txt_str = fTextProp->get().fText;

            // Compute the deletion offset and count in UTF8.
            const size_t utf8_offset  = utf8_index(txt_str, fCursorIndex - 1);
            const char*  utf8_current = txt_str.c_str() + utf8_offset;
            const char*  utf8_next    = utf8_current;
            SkAssertResult(SkUTF::NextUTF8(&utf8_next, txt_str.c_str() + txt_str.size()) >= 0);

            const size_t utf8_count = utf8_next - utf8_current;

            this->deleteChars(utf8_offset, utf8_count);
        }
    }   break;
    default:
        this->deleteSelection();
        this->insertChar(c);
        break;
    }

    // Reset the cursor blink timer on input.
    fTimeBase = std::chrono::steady_clock::now();

    return true;
}
