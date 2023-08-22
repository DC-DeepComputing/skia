/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "imgui.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/viewer/Slide.h"

class ImageBlurAdjustableSlide : public Slide {
public:
    ImageBlurAdjustableSlide() {
        fName = "ImageBlurAdjustable";
    }

    void load(SkScalar winWidth, SkScalar winHeight) override {
        fImg = GetResourceAsImage("images/mandrill_256.png");
        // fImg = GetResourceAsImage("images/mandrill_1600.png");
    }

    void unload() override {
        fImg = nullptr;
    }

    void draw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setImageFilter(SkImageFilters::Blur(fSigma, fSigma, /*input=*/nullptr));

        canvas->saveLayer(nullptr, &paint);
        canvas->drawImage(fImg, /*left=*/0, /*top=*/0);
        canvas->restore();

        ImGui::Begin("Adjustable Image Blur", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SliderFloat("Sigma", &fSigma, 0.0f, 32.0f);
        ImGui::End();
    }

private:
    sk_sp<SkImage> fImg;
    float fSigma = 3.0f;
};

DEF_SLIDE(return new ImageBlurAdjustableSlide();)
