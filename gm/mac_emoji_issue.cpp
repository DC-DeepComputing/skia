/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/private/chromium/Slug.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <string.h>

static sk_sp<SkTextBlob> make_hpos_test_blob_utf8(const char* text, const SkFont& font) {
    constexpr SkTextEncoding enc = SkTextEncoding::kUTF8;
    SkTextBlobBuilder builder;
    size_t len = strlen(text);
    int glyphCount = font.countText(text, len, enc);
    const auto& buffer = builder.allocRunPosH(font, glyphCount, 0);
    (void)font.textToGlyphs(text, len, enc, buffer.glyphs, glyphCount);
    font.getXPos(buffer.glyphs, glyphCount, buffer.pos);
    return builder.make();
}

namespace skiagm {

class MacEmojiIssueFragmentBoxGM : public GM {
public:
    MacEmojiIssueFragmentBoxGM() { this->setBGColor(0xFFFFFFFF); }

protected:
    struct EmojiFont {
        sk_sp<SkTypeface> fTypeface;
        const char* fText;
    } fEmojiFont;

    void onOnceBeforeDraw() override {
        fEmojiFont.fTypeface = MakeResourceAsTypeface("fonts/sbix.ttf");
        fEmojiFont.fText = "\xF0\x9F\x98\x80";
    }

    SkString onShortName() override { return SkString("mac_emoji_issue_fragment_box"); }

    SkISize onISize() override { return SkISize::Make(1600, 1200); }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorGRAY);

        SkPaint paint;
        paint.setAntiAlias(true);

        SkPaint blue_paint;
        blue_paint.setColor(SK_ColorBLUE);
        blue_paint.setAntiAlias(false);

        SkFont font = SkFont(fEmojiFont.fTypeface, 8);
        font.setSubpixel(true);
        const char* text = fEmojiFont.fText;
        sk_sp<SkTextBlob> blob = make_hpos_test_blob_utf8(text, font);

        std::vector<float> x_coords{8.09375,
                                    8.1875,
                                    8.29688,
                                    8.39063,
                                    8.5,
                                    8.59375,
                                    8.6875,
                                    8.79688,
                                    8.89063,
                                    9.09375,
                                    9.1875,
                                    9.29688,
                                    9.39063,
                                    9.5,
                                    9.59375,
                                    9.6875,
                                    9.79688,
                                    9.89063};
        float y = 100;
        int rect_width = 7;
        int rect_height = 8;

        for (auto x : x_coords) {
            canvas->drawRect(
                    SkRect::MakeXYWH((int)(x + 0.5), y - rect_height, rect_width, rect_height),
                    blue_paint);
            canvas->drawTextBlob(blob, x, y, paint);
            y += rect_height + 1;
        }
    }

private:
    using INHERITED = GM;
};

class MacEmojiIssuePositionsIterationGM : public GM {
public:
    MacEmojiIssuePositionsIterationGM() { this->setBGColor(0xFFFFFFFF); }

protected:
    struct EmojiFont {
        sk_sp<SkTypeface> fTypeface;
        const char* fText;
    } fEmojiFont;

    void onOnceBeforeDraw() override {
        fEmojiFont.fTypeface = MakeResourceAsTypeface("fonts/sbix.ttf");
        fEmojiFont.fText = "\xF0\x9F\x98\x80";
    }

    SkString onShortName() override { return SkString("mac_emoji_issue_positions_iteration"); }

    SkISize onISize() override { return SkISize::Make(1600, 1200); }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorGRAY);

        SkPaint paint;
        paint.setAntiAlias(true);
        SkFont font = SkFont(fEmojiFont.fTypeface, 8);
        SkFont subpixel_font = SkFont(fEmojiFont.fTypeface, 8);
        subpixel_font.setSubpixel(true);
        const char* text = fEmojiFont.fText;
        sk_sp<SkTextBlob> blob = make_hpos_test_blob_utf8(text, font);
        sk_sp<SkTextBlob> subpixel_blob = make_hpos_test_blob_utf8(text, subpixel_font);

        double subpixel_x = 7;
        double x = 20;
        double y = 100;
        for (double i = 0; i < 1; i += 0.05) {
            canvas->drawTextBlob(blob, x + i, y, paint);
            canvas->drawTextBlob(subpixel_blob, subpixel_x + i, y, paint);
            y += 10;
        }
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new MacEmojiIssueFragmentBoxGM;)
DEF_GM(return new MacEmojiIssuePositionsIterationGM;)

}  // namespace skiagm
