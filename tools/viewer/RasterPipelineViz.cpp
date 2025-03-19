/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkCoreBlitters.h"
#include "tools/viewer/Slide.h"

#include <vector>

class RPVizSlide : public Slide {
public:
    RPVizSlide() { fName = "RasterPipelineViz"; }

    void draw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorWHITE);

        SkPaint paint;
        paint.setColor(SK_ColorRED);
        //paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1.0));
        SkColor colors[] = {
            SkColorSetRGB(0xE4, 0x03, 0x03), // Red
            SkColorSetRGB(0xFF, 0x8C, 0x00), // Orange
            SkColorSetRGB(0xFF, 0xED, 0x00), // Yellow
            SkColorSetRGB(0x00, 0x80, 0x26), // Green
            SkColorSetRGB(0x00, 0x4D, 0xFF), // Blue
            SkColorSetRGB(0x75, 0x07, 0x87)  // Violet
        };
        SkPoint points[] = {
            {5, 5}, {95, 95},
        };
        auto shader = SkGradientShader::MakeLinear(
            points, colors, nullptr, std::size(colors), SkTileMode::kClamp);
        paint.setShader(shader);
        paint.setBlendMode(SkBlendMode::kSrc);

        SkBitmap dst;
        dst.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kOpaque_SkAlphaType));
        dst.allocPixels();
        dst.eraseColor(SK_ColorBLACK);

        constexpr size_t kNumPanels = 4;
        std::vector<SkBitmap> panels;
        for (auto i = 0; i < kNumPanels; i++) {
            SkBitmap panel;
            panel.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kOpaque_SkAlphaType));
            panel.allocPixels();
            panel.eraseColor(SK_ColorBLACK);
            panels.push_back(panel);
        }

        static constexpr size_t kStackMemory = 1024; // arbitrary
        SkSTArenaAlloc<kStackMemory> alloc;
        auto blitter = SkCreateRasterPipelineBlitterVisualizer(dst.pixmap(),
                                                     panels,
                                                     paint,
                                                     SkMatrix::I(),
                                                     &alloc,
                                                     nullptr,
                                                     {});

        blitter->blitRect(0, 0, 100, 100);

        for (auto i = 0; i < panels.size(); i++) {
            auto panelImg = SkImages::RasterFromBitmap(panels[i]);
            canvas->drawImage(panelImg, 110 * i + 10, 10);
        }

        auto dstImg = SkImages::RasterFromBitmap(dst);
        canvas->drawImage(dstImg, 100, 400);


        // RasterFromPixmapCopy



        // auto blitter =



        // SkCreateRasterPipelineBlitterVisualizer
    }

};

DEF_SLIDE( return new RPVizSlide(); )
