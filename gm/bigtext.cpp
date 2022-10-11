/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "tools/ToolUtils.h"

/**
 *  Skia may draw from outlines when the size is very large, so we exercise that
 *  here.
*/

class BigTextGM : public skiagm::GM {
public:
    BigTextGM() {}

protected:

    SkString onShortName() override {
        return SkString("bigtext");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        SkFont font(ToolUtils::create_portable_typeface(), 1500);

        SkRect r;
        (void)font.measureText("/", 1, SkTextEncoding::kUTF8, &r);
        SkPoint pos = {
            this->width()/2 - r.centerX(),
            this->height()/2 - r.centerY()
        };

        paint.setColor(SK_ColorRED);
        canvas->drawSimpleText("/", 1, SkTextEncoding::kUTF8, pos.fX, pos.fY, font, paint);

        paint.setColor(SK_ColorBLUE);
        canvas->drawSimpleText("\\", 1, SkTextEncoding::kUTF8, pos.fX, pos.fY, font, paint);
    }

private:
    using INHERITED = skiagm::GM;
};

DEF_GM(return new BigTextGM;)

// This exercises the case where the glyph is sufficiently large that we should just draw with a
// path, but if we incorrectly query the platform scaler context, it can report that the glyph has
// empty bounds (because it's too big to smooth as an image). Seeing empty bounds, we would
// incorrectly discard the glyph before we let path rendering handle it. See crbug.com/1370488
DEF_SIMPLE_GM(bigtext_crbug_1370488, canvas, 512, 512) {
    auto typeface = SkTypeface::MakeFromName("Times New Roman", SkFontStyle::Normal());
    SkFont font(typeface, 12.f);

    canvas->translate(-250.f, 300.f);
    canvas->scale(437.5f, 437.5f);
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas->drawString("S", 0.f, 0.f, font, paint);
}
