/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkString.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu.h"
#endif

#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_libgrapheme.h"
#endif

#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu4x.h"
#endif

// Visual coverage for ellipsis shaping across bidi contexts. Ensures that
// custom ellipsis strings with non-neutral characters (strong-LTR letter,
// strong-RTL Arabic word) render with the surrounding paragraph's directional
// context rather than a hardcoded LTR level. The default "…" (ON-class) rows
// act as regression guards for the common path.

namespace {

sk_sp<SkUnicode> get_unicode() {
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
    if (auto unicode = SkUnicodes::ICU::Make()) {
        return unicode;
    }
#endif
#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
    if (auto unicode = SkUnicodes::Libgrapheme::Make()) {
        return unicode;
    }
#endif
#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
    if (auto unicode = SkUnicodes::ICU4X::Make()) {
        return unicode;
    }
#endif
    return nullptr;
}

constexpr SkScalar kRowHeight = 80;
constexpr SkScalar kLayoutWidth = 280;
constexpr SkScalar kCanvasWidth = 640;
constexpr int kRowCount = 4;
// Visual gap between row baselines and the debug border so descenders aren't
// clipped by the border rectangle drawn around each paragraph's layout box.
constexpr SkScalar kRowBorderInset = 10;

}  // namespace

class EllipsisBidiGM : public skiagm::GM {
public:
    EllipsisBidiGM() = default;

protected:
    SkString getName() const override { return SkString("paragraph_ellipsis_bidi"); }

    SkISize getISize() override {
        return SkISize::Make(static_cast<int>(kCanvasWidth),
                             static_cast<int>(kRowHeight * kRowCount + 20));
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        sk_sp<SkFontMgr> fontmgr = ToolUtils::TestFontMgr();
        if (fontmgr->countFamilies() == 0) {
            *errorMsg = "No fonts available.";
            return DrawResult::kSkip;
        }
        auto collection = sk_make_sp<skia::textlayout::FontCollection>();
        collection->setDefaultFontManager(fontmgr);
        collection->enableFontFallback();

        auto unicode = get_unicode();
        if (!unicode) {
            *errorMsg = "No SkUnicode implementation available.";
            return DrawResult::kSkip;
        }

        canvas->clear(SK_ColorWHITE);

        SkScalar y = 10;

        // Row 1: LTR paragraph, default "…" ellipsis (ON-class, regression guard).
        drawRow(canvas, 10, y, collection, unicode,
                skia::textlayout::TextDirection::kLtr,
                u"The quick brown fox jumps over the lazy dog",
                u"\u2026");
        y += kRowHeight;

        // Row 2: LTR paragraph, custom ellipsis containing strong-LTR letters.
        drawRow(canvas, 10, y, collection, unicode,
                skia::textlayout::TextDirection::kLtr,
                u"The quick brown fox jumps over the lazy dog",
                u"[more]");
        y += kRowHeight;

        // Row 3: RTL paragraph, default "…" ellipsis.
        drawRow(canvas, 10, y, collection, unicode,
                skia::textlayout::TextDirection::kRtl,
                u"\u062A\u0638\u0627\u0647\u0631\u0627\u062A \u0648 "
                u"\u062A\u062C\u0645\u0639\u0627\u062A",
                u"\u2026");
        y += kRowHeight;

        // Row 4: RTL paragraph, custom Arabic-word ellipsis "النهاية".
        // This is the case fixed by using the surrounding cluster's bidi level
        // in shapeEllipsis — previously the affix was shaped as LTR, breaking
        // HarfBuzz's Arabic contextual forms.
        drawRow(canvas, 10, y, collection, unicode,
                skia::textlayout::TextDirection::kRtl,
                u"\u062A\u0638\u0627\u0647\u0631\u0627\u062A \u0648 "
                u"\u062A\u062C\u0645\u0639\u0627\u062A",
                u"\u0627\u0644\u0646\u0647\u0627\u064A\u0629");

        return DrawResult::kOk;
    }

private:
    static void drawRow(SkCanvas* canvas,
                        SkScalar x,
                        SkScalar y,
                        const sk_sp<skia::textlayout::FontCollection>& collection,
                        const sk_sp<SkUnicode>& unicode,
                        skia::textlayout::TextDirection direction,
                        const char16_t* text,
                        const char16_t* ellipsis) {
        skia::textlayout::TextStyle textStyle;
        textStyle.setFontFamilies({SkString("Noto Naskh Arabic"), SkString("Roboto")});
        textStyle.setFontSize(30);
        textStyle.setColor(SK_ColorBLACK);

        skia::textlayout::ParagraphStyle paragraphStyle;
        paragraphStyle.setTextStyle(textStyle);
        paragraphStyle.setTextDirection(direction);
        paragraphStyle.setEllipsis(std::u16string(ellipsis));
        paragraphStyle.setMaxLines(1);

        auto builder = skia::textlayout::ParagraphBuilderImpl::make(
                paragraphStyle, collection, unicode);
        if (!builder) {
            return;
        }
        builder->addText(std::u16string(text));
        auto paragraph = builder->Build();
        paragraph->layout(kLayoutWidth);
        paragraph->paint(canvas, x, y);

        SkPaint border;
        border.setStroke(true);
        border.setColor(0xFFCCCCCC);
        canvas->drawRect(SkRect::MakeXYWH(x, y, kLayoutWidth, kRowHeight - kRowBorderInset),
                         border);
    }
};

DEF_GM(return new EllipsisBidiGM();)
