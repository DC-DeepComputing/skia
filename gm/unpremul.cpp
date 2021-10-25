/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "tools/ToolUtils.h"

#include <string.h>

DEF_SIMPLE_GM(unpremul, canvas, 200, 200) {
    const SkColor color = 0xbf400000;

    auto grade = [&](int x, int y){
        SkBitmap bm;
        bm.allocPixels(SkImageInfo::Make(1,1,
                                         kBGRA_8888_SkColorType,
                                         kUnpremul_SkAlphaType,
                                         SkColorSpace::MakeSRGB()));
        if (!canvas->readPixels(bm, x,y)) {
            // Picture-backed canvases, that sort of thing.  Just assume they're good.
            MarkGMGood(canvas, 140,40);
            return;
        }

        SkColor pixel;
        memcpy(&pixel, bm.getAddr(0,0), sizeof(pixel));

        auto close = [](int x, int y) {
            return x-y < 2
                && y-x < 2;
        };

        if (close(SkColorGetR(pixel), SkColorGetR(color)) &&
            close(SkColorGetG(pixel), SkColorGetG(color)) &&
            close(SkColorGetB(pixel), SkColorGetB(color)) &&
            close(SkColorGetA(pixel), SkColorGetA(color))) {

            MarkGMGood(canvas, 140,40);
        } else {
            MarkGMBad(canvas, 140,40);
        }
    };

    {
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        paint.setColor(color);

        canvas->drawRect({0,0,100,100}, paint);
        grade(50,50);
    }

    canvas->translate(0,100);

    {
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);

        SkBitmap bm;
        bm.allocPixels(SkImageInfo::Make(100,100, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType));
        bm.eraseColor(color);

        canvas->drawImage(bm.asImage(), 0,0, SkSamplingOptions(), &paint);
        grade(50,150);
    }

}

static sk_sp<SkImage> make_img(SkCanvas* canvas, SkColorType dst, SkColorType src) {
    // We demonstrate the bug on an off-screen surface:
    SkImageInfo info = SkImageInfo::Make(64, 64, dst, kPremul_SkAlphaType);
    sk_sp<SkSurface> surf = canvas->makeSurface(info);
    if (!surf) {
        surf = SkSurface::MakeRaster(info);
    }

    // Build some image data that includes "invalid" premul pixels:

    // Top-left     is faint,       valid   premul blue
    // Top-right    is faint,       valid   premul black
    // Bottom-left  is transparent, invalid premul blue
    // Bottom-right is transparent, valid   premul black
    // [Note that for some color-types, blue -> red]

    // A: 32 | A: 32
    // B: 32 | B:  0
    // ------+------
    // A:  0 | A:  0
    // B: 32 | B:  0
    uint32_t pixels[64 * 64];
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            pixels[y * 64 + x] = SkColorSetARGB(y < 32 ? 32 : 0, 0, 0, x < 32 ? 32 : 0);
        }
    }
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::Make(64, 64, src, kPremul_SkAlphaType));
    bm.setPixels(pixels);

    // Clear the offscreen to opaque white, to start:
    surf->getCanvas()->clear(SK_ColorWHITE);
    // Then draw our not-really-premul image:
    surf->getCanvas()->drawImage(bm.asImage(), 0, 0);

    return surf->makeImageSnapshot();
}

DEF_SIMPLE_GM(crbug_1252077, canvas, 128, 128) {
    ToolUtils::draw_checkerboard(canvas);

    // Draw the offscreen image over it. In theory, it should still be fully opaque
    // everywhere (we did src-over blending on a surface that started out opaque).
    // Surprise! We've made the bottom-left pixels transparent.
    canvas->drawImage(make_img(canvas, kRGBA_8888_SkColorType, kRGBA_8888_SkColorType), 0, 0);
    canvas->drawImage(make_img(canvas, kRGBA_8888_SkColorType, kBGRA_8888_SkColorType), 64, 0);
    canvas->drawImage(make_img(canvas, kBGRA_8888_SkColorType, kRGBA_8888_SkColorType), 0, 64);
    canvas->drawImage(make_img(canvas, kBGRA_8888_SkColorType, kBGRA_8888_SkColorType), 64, 64);
}
