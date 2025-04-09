/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFont.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkBlurMask.h"
#include "src/effects/SkEmbossMaskFilter.h"
#include "tools/fonts/FontToolUtils.h"

#include "include/core/SkBlurTypes.h"
#include "include/core/SkPoint3.h"
#include "include/effects/SkImageFilters.h"
// #include "src/shaders/SkColorShader.h"

static sk_sp<SkImage> make_bm() {
    auto surf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100));

    SkPaint paint;
    paint.setAntiAlias(true);
    surf->getCanvas()->drawCircle(50, 50, 50, paint);
    return surf->makeImageSnapshot();
}

sk_sp<SkImageFilter> make_imagefilter(SkScalar fBlurSigma, const SkEmbossMaskFilter::Light& fLight) {
    sk_sp<SkImageFilter> blend = SkImageFilters::Blur(fBlurSigma, fBlurSigma, nullptr);
    sk_sp<SkImageFilter> background = SkImageFilters::Blend(SkBlendMode::kDstIn, nullptr, std::move(blend));
    SkPoint3 direction = SkPoint3::Make(-fLight.fDirection[0], -fLight.fDirection[1], fLight.fDirection[2]);
    if (!direction.normalize()) {
        return nullptr;
    }
    const int surfaceScale = 32;

    sk_sp<SkImageFilter> diffuse = SkImageFilters::DistantLitDiffuse(direction,
                                                                     SK_ColorWHITE,
                                                                     surfaceScale,
                                                                     1,
                                                                     background);


    float ambientf = static_cast<float>(fLight.fAmbient);
    uint8_t rgbTable[256];
    uint8_t alphaTable[256];
    for (int i = 0; i < 256; ++i) {
        rgbTable[i] = (i * ambientf) / 255;
        alphaTable[i] = i;
    }
    sk_sp<SkColorFilter> ambientCF = SkColorFilters::TableARGB(alphaTable, rgbTable, rgbTable, rgbTable);
    sk_sp<SkImageFilter> ambient = SkImageFilters::ColorFilter(ambientCF, nullptr);

    sk_sp<SkImageFilter> ambientdiffuse = SkImageFilters::Blend(SkBlendMode::kPlus, diffuse, ambient);
    sk_sp<SkImageFilter> ambientdiffuseBlend = SkImageFilters::Blend(SkBlendMode::kModulate, nullptr, ambientdiffuse);
    float test = pow(2, (fLight.fSpecular >> 4) + 1)/ 255;
    // SkDebugf("test %f\n", test);

    SkPoint3 direction2 = SkPoint3::Make(-fLight.fDirection[0], -fLight.fDirection[1], -fLight.fDirection[2]);
    sk_sp<SkImageFilter> specular = SkImageFilters::DistantLitSpecular(direction2,
                                                                       SK_ColorWHITE,
                                                                       32,
                                                                       0.5,
                                                                       (fLight.fSpecular >> 4) + 1,
                                                                       background);
    sk_sp<SkImageFilter> finalFilter = SkImageFilters::Blend(SkBlendMode::kPlus, ambientdiffuseBlend, specular);

    return SkImageFilters::Blend(SkBlendMode::kDstIn, std::move(finalFilter), nullptr);
}

class EmbossGM : public skiagm::GM {
public:
    EmbossGM() {
    }

protected:
    SkString getName() const override { return SkString("emboss"); }

    SkISize getISize() override { return SkISize::Make(600, 120); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        sk_sp<SkImageFilter> imgfilter1 = make_imagefilter(   SkBlurMask::ConvertRadiusToSigma(3),
        { { 1, 1, 1 }, 0, 128, 32});
        auto img = make_bm();
        canvas->drawImage(img, 10, 10);
        canvas->translate(img->width() + SkIntToScalar(10), 0);

        // paint.setMaskFilter(SkEmbossMaskFilter::Make(SkBlurMask::ConvertRadiusToSigma(3),{  { 1, 1, 1 }, 0, 128, 128 }));
        // paint.setMaskFilter(SkEmbossMaskFilter::Make(SkBlurMask::ConvertRadiusToSigma(3),{ { 0,0, -1  }, 0, 128, 0 }));
        // paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kInner_SkBlurStyle,  SkBlurMask::ConvertRadiusToSigma(3)));
        paint.setImageFilter(imgfilter1);
        canvas->drawImage(img, 10, 10, SkSamplingOptions(), &paint);
        canvas->translate(img->width() + SkIntToScalar(10), 0);

        // this combination of emboss+colorfilter used to crash -- so we exercise it to
        // confirm that we have a fix.
        paint.setColorFilter(SkColorFilters::Blend(0xFFFF0000, SkBlendMode::kSrcOver));
        canvas->drawImage(img, 10, 10, SkSamplingOptions(), &paint);
        canvas->translate(img->width() + SkIntToScalar(10), 0);

        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(10));
        sk_sp<SkImageFilter> imgfilter2 = make_imagefilter(   SkBlurMask::ConvertRadiusToSigma(4),
        {{ 1, 1, 1 }, 0, 128, 32});

        paint.setImageFilter(imgfilter2);
        // paint.setMaskFilter(SkEmbossMaskFilter::Make(SkBlurMask::ConvertRadiusToSigma(4),{  { 1, 1, 1 }, 0, 128, 128 }));
        // paint.setMaskFilter(SkEmbossMaskFilter::Make(SkBlurMask::ConvertRadiusToSigma(4),{ { 0, 0, 1  }, 0, 128, 0 }));
        paint.setColorFilter(nullptr);
        paint.setShader(SkShaders::Color(SK_ColorBLUE));
        paint.setDither(true);
        canvas->drawCircle(SkIntToScalar(50), SkIntToScalar(50),
                           SkIntToScalar(30), paint);
        canvas->translate(SkIntToScalar(100), 0);

        SkFont font = SkFont(ToolUtils::DefaultPortableTypeface(), 50);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawString("Hello", 0, 50, font, paint);

        paint.setShader(nullptr);
        paint.setColor(SK_ColorGREEN);
        canvas->drawString("World", 0, 100, font, paint);
    }

private:
    using INHERITED = skiagm::GM;
};

DEF_GM(return new EmbossGM;)
