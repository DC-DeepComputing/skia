/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkGradientShader.h"
#include "tools/Resources.h"

namespace {

sk_sp<SkShader> create_gradient_shader(SkRect r) {
    // TODO: it seems like only the x-component of sk_FragCoord is making it to the shader!
    SkPoint pts[2] = { {r.fLeft, r.fTop}, {r.fRight, r.fTop} };
    SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    float offsets[] = { 0.0f, 0.75f, 1.0f };

    return SkGradientShader::MakeLinear(pts, colors, offsets, SK_ARRAY_COUNT(colors),
                                        SkTileMode::kClamp);
}

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
    // TODO: we'll need a 'makeTextureImage' call here

    return img->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkSamplingOptions());
}

sk_sp<SkShader> create_blend_shader(SkBlendMode bm) {
    constexpr SkColor4f kTransYellow = {1.0f, 1.0f, 0.0f, 0.5f};

    sk_sp<SkShader> dst = SkShaders::Color(kTransYellow, nullptr);
    return SkShaders::Blend(bm, std::move(dst), create_image_shader());
}

sk_sp<SkShader> create_solid_shader(SkColor4f color) {
    return SkShaders::Color(color, nullptr);
}

} // anonymous namespace

#include "include/core/SkRRect.h"

namespace skiagm {

// This is just for bootstrapping Graphite.
class GraphiteStartGM : public GM {
public:
    GraphiteStartGM() {
        this->setBGColor(0xFF000000);
        GetResourceAsBitmap("images/color_wheel.gif", &fBitmap);
    }

protected:
    static const int kWidth = 256;
    static const int kHeight = 64; // 384;
    static const int kTileHeight = 16;
    static const int kTileWidth = 16;

    SkString onShortName() override {
        return SkString("graphitestart");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    struct TestCase {
        SkBlendMode fBM;
    };

    void onDraw(SkCanvas* canvas) override {

        static const SkColor4f kDstColor { 1.0f, 1.0f, 1.0f, 1.0f };
        static const SkColor4f kSrcColor { 0.0f, 0.5f, 1.0f, 0.5f };

        SkPaint dstPaint;
        dstPaint.setColor(kDstColor);

        SkPaint srcPaint;
        srcPaint.setColor(kSrcColor);
        srcPaint.setAntiAlias(false);

        SkRect r {0.0f, 0.0f, kTileWidth, kTileHeight};
        for (int i = 0; i <= (int)SkBlendMode::kLastCoeffMode; ++i) {
            if (r.fLeft+kTileWidth > kWidth) {
                r.offsetTo(0.0f, r.fTop+kTileHeight);
            }

            canvas->drawRect(r.makeInset(1.0f, 1.0f), dstPaint);
            srcPaint.setBlendMode(static_cast<SkBlendMode>(i));
            canvas->drawRect(r.makeInset(2.0f, 2.0f), srcPaint);

            r.offset(kTileWidth, 0.0f);
        }

#if 0
        // UL corner
        {
            SkPaint p;
            p.setColor(SK_ColorRED);
            p.setShader(create_solid_shader(SkColors::kBlue));
            p.setBlendMode(SkBlendMode::kMultiply);
            p.setAntiAlias(false);
            canvas->drawRect({2, 2, 127, 127}, p);
        }
#endif

#if 0
        // UR corner
        {
            SkRect r{129, 2, 255, 127};
            SkPaint p;
            p.setShader(create_gradient_shader(r));
            canvas->drawRect(r, p);
        }

        // LL corner
        {
            SkPaint p;
            p.setShader(create_image_shader());
            canvas->drawRect({2, 129, 127, 255}, p);
        }
#endif

#if 0
        // LR corner
        {
            SkPaint p;
            p.setColor(SK_ColorRED);
            p.setShader(create_blend_shader(SkBlendMode::kModulate));
            p.setBlendMode(SkBlendMode::kMultiply);
            p.setAntiAlias(false);
            canvas->drawRect({129, 129, 255, 255}, p);
        }
#endif

#if 0
#ifdef SK_GRAPHITE_ENABLED
        // TODO: failing serialize test on Linux, not sure what's going on
        canvas->writePixels(fBitmap, 0, 256);
#endif
#endif
    }

private:
    SkBitmap fBitmap;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new GraphiteStartGM;)

}  // namespace skiagm
