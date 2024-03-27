/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "imgui.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkSurface.h"
#include "tools/viewer/Slide.h"

#include <functional>

// Implementation in SkBlurMask:
static const SkScalar kBLUR_SIGMA_SCALE = 0.57735f;
static float sk_blur_mask_radius_to_sigma(float radius) {
    return radius > 0 ? kBLUR_SIGMA_SCALE * radius + 0.5f : 0.0f;
}

// Implementation in CreateShadowDrawLooper (from gfx::ShadowValue "blur amount"):
// Actual use-case divides "blur amount" by two, then calls this function:
static float gfx_shadow_blur_to_sigma(float radius) {
    return radius > 0 ? SkDoubleToScalar(0.288675f * radius + 0.5f) : 0;
}

// Implementation in skia_utils.h, for converting CSS blur amounts:
static float css_blur_radius_to_sigma(float radius) {
    return radius * 0.5;
}

class BlurTweakSlide : public Slide {
    float fBlurRadius = 10;

public:
    BlurTweakSlide() { fName = "BlurTweak"; }

    struct BlurMapping {
        std::function<float(float)> fn;
        const char* name;
    };

    void draw(SkCanvas* canvas) override {
        ImGui::Begin("Blur");
        ImGui::SliderFloat("Radius", &fBlurRadius, 0.0f, 40.0f);

        canvas->clear(SK_ColorBLACK);

        const BlurMapping kMappings[] = {
            {sk_blur_mask_radius_to_sigma, "SkBlurMask"},
            {gfx_shadow_blur_to_sigma, "gfx::ShadowValue"},
            {css_blur_radius_to_sigma, "CSS"},
        };

        constexpr int kWidth = 210, kHeight = 50;
        SkRect r = SkRect::MakeXYWH(70.5f, 0.5f, 70, 49);

        auto make_blur_image = [&](float sigma, int* actualRadius) {
            sk_sp<SkSurface> surf =
                    canvas->makeSurface(SkImageInfo::MakeN32Premul(kWidth, kHeight));
            SkCanvas* surfCanvas = surf->getCanvas();
            surfCanvas->clear(SK_ColorBLACK);

            SkPaint blurPaint;
            blurPaint.setColor(SK_ColorWHITE);
            blurPaint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, sigma));
            surfCanvas->drawRect(r, blurPaint);

            SkBitmap bmp;
            bmp.allocPixels(surf->imageInfo());
            surf->readPixels(bmp, 0, 0);

            int firstPixel = 0;
            for (; firstPixel < 75; ++firstPixel) {
                if (SkColorGetR(bmp.getColor(firstPixel, 25))) {
                    break;
                }
            }
            SkASSERT(firstPixel < 75);

            int lastPixel = 209;
            for (; lastPixel > 135; --lastPixel) {
                if (SkColorGetR(bmp.getColor(lastPixel, 25))) {
                    break;
                }
            }
            SkASSERT(lastPixel > 135);

            SkPaint linePaint;
            linePaint.setStyle(SkPaint::kStroke_Style);
            linePaint.setColor(SK_ColorYELLOW);
            linePaint.setAntiAlias(true);
            surfCanvas->drawRect(r, linePaint);

            SkPoint pts[2] = {{firstPixel + 0.5f, 25.5f}, {lastPixel + 0.5f, 25.5f}};
            surfCanvas->drawPoints(SkCanvas::kPoints_PointMode, 2, pts, linePaint);

            *actualRadius = 70 - firstPixel;
            return surf->makeImageSnapshot();
        };

        int y = 10;
        for (const BlurMapping& mapping : kMappings) {
            int actualRadius = 0;
            canvas->drawImage(make_blur_image(mapping.fn(fBlurRadius), &actualRadius), 0, y);
            ImGui::Text("%s : %d", mapping.name, actualRadius);
            y += 60;
        }

        ImGui::End();
    }
};

DEF_SLIDE( return new BlurTweakSlide(); )
