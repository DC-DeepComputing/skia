/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"

namespace {

sk_sp<SkShader> create_image_shader() {
    SkImageInfo ii = SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkBitmap bitmap;

    bitmap.allocPixels(ii);
    bitmap.eraseColor(SK_ColorWHITE);

    SkCanvas canvas(bitmap);

    SkPaint redPaint;
    redPaint.setColor(SK_ColorRED);
    canvas.drawCircle(50, 50, 50, redPaint);

    bitmap.setAlphaType(kOpaque_SkAlphaType);
    bitmap.setImmutable();

    sk_sp<SkImage> img = SkImage::MakeFromBitmap(bitmap);

    return img->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkSamplingOptions());
}

sk_sp<SkShader> create_blend_shader(SkBlendMode bm) {
    constexpr SkColor4f kTransYellow = {1.0f, 1.0f, 0.0f, 0.5f};

    sk_sp<SkShader> solid = SkShaders::Color(kTransYellow, nullptr);
    return SkShaders::Blend(bm, std::move(solid), create_image_shader());
}

} // anonymous namespace

namespace skiagm {

// This is just for bootstrapping Graphite.
class GraphiteStartGM : public GM {
public:
    GraphiteStartGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString onShortName() override {
        return SkString("graphitestart");
    }

    SkISize onISize() override {
        return SkISize::Make(256, 256);
    }

    void onDraw(SkCanvas* canvas) override {

        {
            SkPaint p;
            p.setColor(SK_ColorRED);
            canvas->drawRect({2, 2, 127, 127}, p);
        }

        {
            SkPaint p;
            p.setShader(create_blend_shader(SkBlendMode::kDstOver));
            canvas->drawRect({129, 129, 255, 255}, p);
        }

        {
            SkPaint p;
            p.setShader(create_image_shader());
            canvas->drawRect({2, 129, 127, 255}, p);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new GraphiteStartGM;)

}  // namespace skiagm
