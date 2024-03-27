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
#include "include/core/SkColor.h"
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

    static constexpr int kRectWidth = 200;
    static constexpr int kRectHeight = 50;

    void draw(SkCanvas* topCanvas) override {
        ImGui::Begin("Blur");
        ImGui::SliderFloat("Radius", &fBlurRadius, 0.0f, 40.0f);

        topCanvas->clear(SK_ColorBLACK);

        const BlurMapping kMappings[] = {
            {sk_blur_mask_radius_to_sigma, "SkBlurMask"},
            {gfx_shadow_blur_to_sigma, "gfx::ShadowValue"},
            {css_blur_radius_to_sigma, "CSS"},
        };

        SkRect r = SkRect::MakeXYWH(kRectWidth, -kRectWidth, kRectWidth, 2 * kRectWidth);
        const int surfWidth = kRectWidth * 3;

        auto make_blur_image = [&](float sigma, int* actualRadius) {
            sk_sp<SkSurface> surf =
                    topCanvas->makeSurface(SkImageInfo::MakeN32Premul(surfWidth, kRectHeight));
            SkCanvas* canvas = surf->getCanvas();
            canvas->clear(SK_ColorBLACK);

            SkPaint blurPaint;
            blurPaint.setColor(SK_ColorWHITE);
            blurPaint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, sigma));
            canvas->drawRect(r, blurPaint);

            SkBitmap bmp;
            bmp.allocPixels(surf->imageInfo());
            surf->readPixels(bmp, 0, 0);

            int blurBounds[4];
            for (blurBounds[0] = 0; blurBounds[0] < surfWidth; ++blurBounds[0]) {
                if (SkColorGetR(bmp.getColor(blurBounds[0], 25))) {
                    break;
                }
            }
            SkASSERT(blurBounds[0] < surfWidth);

            for (blurBounds[1] = blurBounds[0]; blurBounds[1] < surfWidth; ++blurBounds[1]) {
                if (SkColorGetR(bmp.getColor(blurBounds[1], 25)) == 255) {
                    break;
                }
            }
            SkASSERT(blurBounds[1] < surfWidth);

            for (blurBounds[3] = surfWidth - 1; blurBounds[3] >= 0; --blurBounds[3]) {
                if (SkColorGetR(bmp.getColor(blurBounds[3], 25))) {
                    break;
                }
            }
            SkASSERT(blurBounds[3] > 0);

            for (blurBounds[2] = blurBounds[3]; blurBounds[2] >= 0; --blurBounds[2]) {
                if (SkColorGetR(bmp.getColor(blurBounds[2], 25)) == 255) {
                    break;
                }
            }
            SkASSERT(blurBounds[2] > 0);

            SkPaint linePaint;
            linePaint.setStyle(SkPaint::kStroke_Style);
            linePaint.setAntiAlias(true);

            linePaint.setColor(SK_ColorYELLOW);
            canvas->drawCircle(blurBounds[0] + 0.5f, 25.5f, 1.5f, linePaint);
            canvas->drawCircle(blurBounds[3] + 0.5f, 25.5f, 1.5f, linePaint);
            canvas->drawRect(r.makeInset(0.5f, 0.5f), linePaint);

            linePaint.setColor(SK_ColorMAGENTA);
            canvas->drawCircle(blurBounds[1] + 0.5f, 25.5f, 1.5f, linePaint);
            canvas->drawCircle(blurBounds[2] + 0.5f, 25.5f, 1.5f, linePaint);

            *actualRadius = kRectWidth - blurBounds[0];
            return surf->makeImageSnapshot();
        };

        ImGui::Text("%20s : %8s : %8s : %s", "", "sigma", "estimate", "actual");

        int y = 10;
        for (const BlurMapping& mapping : kMappings) {
            float sigma = mapping.fn(fBlurRadius);
            float fastBounds = 3.0f * sigma;
            int actualRadius = 0;
            topCanvas->drawImage(make_blur_image(sigma, &actualRadius), 0, y);
            ImGui::Text("%20s : %8g : %8g : %d", mapping.name, sigma, fastBounds, actualRadius);
            y += 60;
        }

        ImGui::End();
    }
};

DEF_SLIDE( return new BlurTweakSlide(); )
