/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "gm/long_render_text.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkTypeface_fontations.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/ports/SkTypeface_FreeType.h"
#include "tools/Resources.h"
#include "tools/TestFontDataProvider.h"
#include "tools/fonts/FontToolUtils.h"

namespace skiagm {

namespace {

constexpr int kGmWidth = 3000;
constexpr int kMargin = 30;
constexpr float kFontSize = 6;

}  // namespace

class FontationsRenderLarge : public GM {
public:
    FontationsRenderLarge(SkFontHinting hintingMode = SkFontHinting::kNone)
            : fTestDataIterator("Noto Sans", "en_Latn"), fHintingMode(hintingMode) {
        this->setBGColor(SK_ColorWHITE);
    }

protected:
    SkString getName() const override {
        SkString testName("render_large");
        switch (fHintingMode) {
            case SkFontHinting::kNormal: {
                testName.append("_hint_normal");
                break;
            }
            case SkFontHinting::kSlight: {
                testName.append("_hint_slight");
                break;
            }
            case SkFontHinting::kFull: {
                testName.append("_hint_full");
                break;
            }
            case SkFontHinting::kNone: {
                testName.append("_hint_none");
                break;
            }
        }
        return testName;
    }

    SkISize getISize() override { return SkISize::Make(kGmWidth, kGmWidth * 2); }

    bool runAsBench() const override { return true; }

    void onOnceBeforeDraw() override {
        fTestDataIterator.rewind();
        TestFontDataProvider::TestSet testSet;
        fTestDataIterator.next(&testSet);

        fTestTypeface = ToolUtils::TestFontMgr()->makeFromStream(
                SkStream::MakeFromFile(testSet.fontFilename.c_str()), SkFontArguments());

        auto configureFont = [this](SkFont& font) {
            font.setSize(kFontSize);
            font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
            font.setSubpixel(true);
            font.setHinting(fHintingMode);
        };

        SkFont font(fTestTypeface);
        configureFont(font);

        SkPaint paint;
        paint.setColor(SK_ColorBLACK);

        auto shape = [paint](const SkFont& font) {
            SkTextBlobBuilderRunHandler textBlobBuilder(long_sample_text, SkPoint::Make(0,0));
            std::unique_ptr<SkShaper> shaper = SkShaper::Make();
            shaper->shape(long_sample_text,
                          std::size(long_sample_text),
                          font,
                          true,
                          kGmWidth - 2 * kMargin, /* Don't linebreak. */
                          &textBlobBuilder);
            return textBlobBuilder.makeBlob();
        };

        fBlob = shape(font);
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);

        if (!fTestTypeface) {
            *errorMsg = "Unable to initialize typeface.";
            return DrawResult::kSkip;
        }

        canvas->drawTextBlob(fBlob.get(), kMargin, kMargin, paint);

        return DrawResult::kOk;
    }

private:
    using INHERITED = GM;

    TestFontDataProvider fTestDataIterator;
    SkString fTestName;
    SkFont fFont;
    SkFontHinting fHintingMode;
    sk_sp<SkTextBlob> fBlob;
    sk_sp<SkTypeface> fTestTypeface;
};

DEF_GM(return new FontationsRenderLarge(SkFontHinting::kSlight));
DEF_GM(return new FontationsRenderLarge(SkFontHinting::kNone));
DEF_GM(return new FontationsRenderLarge(SkFontHinting::kNormal));

}  // namespace skiagm
