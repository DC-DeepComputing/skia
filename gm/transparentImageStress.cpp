/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkString.h"
#include "include/effects/SkImageFilters.h"
#include "tools/DecodeUtils.h"

/**
 * Stress test with many blurs and surfaces.
 */
class TransparentImageStressGM : public skiagm::GM {
public:
    TransparentImageStressGM() {}

protected:
    SkString getName() const override { return SkString("transparent-image"); }

    SkISize getISize() override { return {512, 512}; }

    void onDraw(SkCanvas* canvas) override {
        for (int i = 0; i < 64; ++i) {
            canvas->drawImage(fImage.get(), 0, 0, SkFilterMode::kLinear, &fPaint);
        }
    }

    void onOnceBeforeDraw() override {
        fPaint.setColor4f({ 0.0f, 0.0f, 0.0f, 0.5f });
        fPaint.setImageFilter(SkImageFilters::Blur(.6f, .6f, SkTileMode::kDecal, nullptr));
        fImage = ToolUtils::GetResourceAsImage("images/mandrill_512.png")
                          ->makeRasterImage(nullptr)
                          ->makeColorTypeAndColorSpace(nullptr,
                                                       kRGB_888x_SkColorType,
                                                       SkColorSpace::MakeSRGB())
                          ->reinterpretColorSpace(SkColorSpace::MakeSRGB()
                                                               ->makeColorSpin()
                                                               ->makeLinearGamma());

    }

private:
    SkPaint fPaint;
    sk_sp<SkImage> fImage;
};
DEF_GM( return new TransparentImageStressGM; )
