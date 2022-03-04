/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <string.h>
#include <initializer_list>

namespace skiagm {
class ScaledEmojiRenderingGM : public GM {
public:
    ScaledEmojiRenderingGM() {}

protected:
    struct Test {
        char const * const fontName;
        enum class Type { Resource, Portable };
        Type const fontType;
        char const * const text;
        sk_sp<SkTypeface> typeface;
    };
    Test tests[6] = {
        { "fonts/colr.ttf",      Test::Type::Resource, ToolUtils::emoji_sample_text(), nullptr },
        { "fonts/sbix.ttf",      Test::Type::Resource, ToolUtils::emoji_sample_text(), nullptr },
        { "fonts/cbdt.ttf",      Test::Type::Resource, ToolUtils::emoji_sample_text(), nullptr },
        { "Emoji",               Test::Type::Portable, ToolUtils::emoji_sample_text(), nullptr },
        { "fonts/colr.ttf",      Test::Type::Resource, ToolUtils::emoji_sample_text(), nullptr },
        { "fonts/SampleSVG.ttf", Test::Type::Resource, "abcdefghi"                   , nullptr },
    };

    void onOnceBeforeDraw() override {
        for (auto&& test : tests) {
            if (test.fontType == Test::Type::Resource) {
                test.typeface = MakeResourceAsTypeface(test.fontName);
            } else if (test.fontType == Test::Type::Portable) {
                test.typeface = ToolUtils::create_portable_typeface(test.fontName, SkFontStyle());
            } else {
                SK_ABORT("Unknown test type");
            }
        }
    }

    SkString onShortName() override {
        return SkString("scaledemoji_rendering");
    }

    SkISize onISize() override { return SkISize::Make(1200, 1200); }

    void onDraw(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorGRAY);
        SkPaint paint;
        paint.setColor(SK_ColorCYAN);

        SkPaint strokePaint;
        strokePaint.setStrokeWidth(5);

        SkScalar y = 0;
        for (auto&& test : tests) {
            SkFont font(test.typeface);
            font.setEdging(SkFont::Edging::kAlias);

            const char* text = test.text;
            SkFontMetrics metrics;

            for (SkScalar textSize : { 70, 150 }) {
                font.setSize(textSize);
                font.getMetrics(&metrics);
                // All typefaces should support subpixel mode
                font.setSubpixel(true);

                y += -metrics.fAscent;

                SkScalar x = 0;
                for (bool fakeBold : { false, true }) {
                    font.setEmbolden(fakeBold);
                    SkRect bounds;
                    font.measureText(text, strlen(text), SkTextEncoding::kUTF8, &bounds, &paint);
                    canvas->drawSimpleText(text, strlen(text), SkTextEncoding::kUTF8,
                                           x - bounds.left(), y, font, paint);
                    //canvas->drawLine(x, y, x + bounds.width(), y, strokePaint);
                    x += bounds.width() * 1.2;
                }
                y += metrics.fDescent + metrics.fLeading;
                x = 0;
            }
        }
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ScaledEmojiRenderingGM;)
}  // namespace skiagm
