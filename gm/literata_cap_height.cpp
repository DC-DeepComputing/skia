/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPaint.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

#include <cstdio>

namespace skiagm {

class LiterataCapHeightGM : public GM {
public:
    LiterataCapHeightGM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString getName() const override {
        return SkString("literata_cap_height");
    }

    SkISize getISize() override {
        return SkISize::Make(820, 560);
    }

    void printCapHeightTable() {
        if (fPrinted) {
            return;
        }
        fPrinted = true;

        if (!fBaseTypeface) {
            SkDebugf("Error: Failed to load fonts/Literata-Italic-VariableFont_opsz,wght.ttf\n");
            return;
        }

        int upm = fBaseTypeface->getUnitsPerEm();
        if (upm <= 0) {
            upm = 1000;
        }

        SkDebugf("Skia reported cap height for Literata-Italic-VariableFont_opsz,wght.ttf (UPM = %d):\n", upm);
        SkDebugf("%-10s | %-22s | %-18s\n", "opsz", "SkFont fCapHeight", "fCapHeight / UPM");
        SkDebugf("-----------+------------------------+------------------\n");

        for (int opsz = 7; opsz <= 72; ++opsz) {
            SkFourByteTag opszTag = SkSetFourByteTag('o', 'p', 's', 'z');
            SkFontArguments::VariationPosition::Coordinate coord = {opszTag, SkIntToScalar(opsz)};
            SkFontArguments::VariationPosition position = {&coord, 1};
            sk_sp<SkTypeface> tf = fBaseTypeface->makeClone(
                    SkFontArguments().setVariationDesignPosition(position));
            if (!tf) {
                tf = fBaseTypeface;
            }

            SkFont font(tf);
            font.setSize(SkIntToScalar(upm));
            SkFontMetrics metrics;
            font.getMetrics(&metrics);

            SkScalar capHeight = metrics.fCapHeight;
            SkDebugf("%-10d | %-22.2f | %-18.4f\n", opsz, capHeight, capHeight / SkIntToScalar(upm));
        }
    }

    void onOnceBeforeDraw() override {
        fBaseTypeface = ToolUtils::CreateTypefaceFromResource(
                "fonts/Literata-Italic-VariableFont_opsz,wght.ttf");
        if (!fBaseTypeface) {
            fBaseTypeface = ToolUtils::CreateTypefaceFromResource(
                    "Literata-Italic-VariableFont_opsz,wght.ttf");
        }
        this->printCapHeightTable();
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        if (!fBaseTypeface) {
            *errorMsg = "Failed to load fonts/Literata-Italic-VariableFont_opsz,wght.ttf";
            return DrawResult::kFail;
        }

        this->printCapHeightTable();

        int upm = fBaseTypeface->getUnitsPerEm();
        if (upm <= 0) {
            upm = 1000;
        }

        SkPaint paint;
        paint.setColor(SK_ColorBLACK);

        SkFont titleFont = ToolUtils::DefaultFont();
        titleFont.setSize(16);
        canvas->drawString("Literata Italic - Skia Reported Cap Height vs opsz (7 to 72)",
                           20, 30, titleFont, paint);

        SkFont labelFont = ToolUtils::DefaultFont();
        labelFont.setSize(11);

        const int rowsPerCol = 22;
        for (int i = 0; i <= (72 - 7); ++i) {
            int opsz = 7 + i;
            int col = i / rowsPerCol;
            int row = i % rowsPerCol;

            SkScalar x = 20.0f + col * 260.0f;
            SkScalar y = 60.0f + row * 22.0f;

            SkFourByteTag opszTag = SkSetFourByteTag('o', 'p', 's', 'z');
            SkFontArguments::VariationPosition::Coordinate coord = {opszTag, SkIntToScalar(opsz)};
            SkFontArguments::VariationPosition position = {&coord, 1};
            sk_sp<SkTypeface> tf = fBaseTypeface->makeClone(
                    SkFontArguments().setVariationDesignPosition(position));
            if (!tf) {
                tf = fBaseTypeface;
            }

            SkFont metricFont(tf);
            metricFont.setSize(SkIntToScalar(upm));
            SkFontMetrics metrics;
            metricFont.getMetrics(&metrics);

            SkString text = SkStringPrintf("opsz %2d: capHeight = %.2f", opsz, metrics.fCapHeight);
            canvas->drawString(text, x, y, labelFont, paint);

            SkFont sampleFont(tf, 14.0f);
            canvas->drawString("H", x + 190.0f, y, sampleFont, paint);
        }

        return DrawResult::kOk;
    }

private:
    sk_sp<SkTypeface> fBaseTypeface;
    bool fPrinted = false;
};

DEF_GM(return new LiterataCapHeightGM;)

}  // namespace skiagm
