/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "src/core/SkPaintPriv.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
using MaskFormat = skgpu::MaskFormat;
#endif

class DrawAtlasMemoryUsageView : public Sample {

    sk_sp<SkTypeface>       fTypeface;
    SkRandom                fRand;
    uint32_t                fCount = 0;
    SkScalar                fTranslate = 0.0f;

    SkString name() override { return SkString("DrawAtlasMemoryUsageView"); }

    void onDrawContent(SkCanvas* canvas) override {
        if (fCount++ > 1000) {
            return;
        }

        canvas->translate(0, fTranslate);
        fTranslate -= 0.5f;

        canvas->clear(SK_ColorWHITE);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);

        SkString alphabet("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
        float x = 0.0f;
        float y = 0.0f;
        SkTextBlobBuilder builder;
        int letters = 0;
        for (SkScalar fontSize = 10.0f; fontSize <= 150.0f; fontSize += 10) {
            SkFont font(fTypeface, fontSize);
            SkFontMetrics metrics;
            font.getMetrics(&metrics);

            canvas->drawString(alphabet, x, y - metrics.fAscent, font, paint);
            y += metrics.fDescent - metrics.fAscent;
            if (y > -fTranslate && y < this->height() - fTranslate &&
                x < this->width() - metrics.fMaxCharWidth) {
                letters += this->width()/metrics.fAvgCharWidth;
            }
        }

        SkFont font170(fTypeface, 170.0f + fCount%10*10);
        SkFontMetrics metrics;
        font170.getMetrics(&metrics);
        SkPaint paint170;
        paint170.setAntiAlias(true);
        paint170.setColor(SK_ColorDKGRAY);
        for (size_t i = 0; i < alphabet.size(); ++i) {
            auto ii = fRand.nextRangeU(0, alphabet.size());
            auto xx = fRand.nextRangeF(0, this->width() - metrics.fMaxCharWidth);
            auto yy = fRand.nextRangeF(0, this->height() - metrics.fDescent + metrics.fAscent);
            canvas->
                drawString(&alphabet[ii], xx, yy - metrics.fAscent - fTranslate, font170, paint170);
            if (SkIntToScalar(i) > letters/100.0f) {
                break;
            }
        }
        if (letters < 100) {
            fTranslate = 0.0f;
        }
    }

    void onOnceBeforeDraw() override {
        fTypeface = SkTypeface::MakeFromName("Segoe UI", SkFontStyle());
    }
    /*
    char createRandomWord(int lineLength) {
        const char* alphabet =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        auto index = fRand.nextRangeU(0, 52);
        return alphabet[index];
    }
    */
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new DrawAtlasMemoryUsageView(); )
