/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tests/Test.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSurface.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkMask.h"

using BlitterFactory = SkBlitter* (*)(const SkPixmap&, const SkPaint&);

static void compare_antiH_and_mask(skiatest::Reporter* reporter,
                                   SkColor backgroundColor,
                                   SkColor paintColor,
                                   BlitterFactory makeBlitter) {
    // Space for a 1 pixel image
    uint8_t buffer[1 * 1 * 4];
    auto ii = SkImageInfo::Make(
            {1, 1}, SkColorInfo(kN32_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB()));

    SkPixmap device(ii, buffer, ii.minRowBytes());
    auto surface = SkSurfaces::WrapPixels(device);
    SkASSERT(surface);

    SkPaint paint;
    paint.setColor(paintColor);

    auto blitter = makeBlitter(device, paint);

    SkAlpha antiAlias[1];
    int16_t runs[2] = {1, 0};  // Single pixel run

    uint8_t maskImage[1];
    auto maskBounds = SkIRect::MakeXYWH(0, 0, 1, 1);
    for (int alpha = 0; alpha <= 255; ++alpha) {
        antiAlias[0] = static_cast<SkAlpha>(alpha);
        maskImage[0] = static_cast<SkAlpha>(alpha);

        SkMask mask(maskImage, maskBounds, 1, SkMask::kA8_Format);

        surface->getCanvas()->clear(backgroundColor);
        blitter->blitAntiH(0, 0, antiAlias, runs);
        SkColor antiHColor = device.getColor(0, 0);

        surface->getCanvas()->clear(backgroundColor);
        blitter->blitMask(mask, mask.fBounds);
        SkColor maskColor = device.getColor(0, 0);

        REPORTER_ASSERT(reporter,
                        antiHColor == maskColor,
                        "background=%08x, paint=%08x, alpha=%d, "
                        "blitAntiH=%08x, blitMask=%08x, "
                        "diff=%02x %02x %02x %02x",
                        backgroundColor, paintColor, alpha,
                        antiHColor, maskColor,
                        abs((int)(SkColorGetA(antiHColor) - SkColorGetA(maskColor))),
                        abs((int)(SkColorGetR(antiHColor) - SkColorGetR(maskColor))),
                        abs((int)(SkColorGetG(antiHColor) - SkColorGetG(maskColor))),
                        abs((int)(SkColorGetB(antiHColor) - SkColorGetB(maskColor))));
    }

    delete blitter;
}

DEF_TEST(SkARGB32OpaqueBlitter_twoBlitMethodsDrawTheSame, r) {
    SkColor backgroundColors[] = {
            SK_ColorWHITE, SK_ColorBLACK, SK_ColorGRAY,
            SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE,
            SK_ColorYELLOW, SK_ColorCYAN, SK_ColorMAGENTA,
            // arbitrary opaque color with uneven channels
            SkColorSetARGB(255, 255 / 4, 255 / 3, 255 / 2),
    };
    SkColor paintColors[] = {
            SK_ColorWHITE, SK_ColorBLACK, SK_ColorGRAY,
            SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE,
            SK_ColorYELLOW, SK_ColorCYAN, SK_ColorMAGENTA,
            SkColorSetARGB(255, 255 / 4, 255 / 3, 255 / 2),
    };

    for (SkColor backgroundColor : backgroundColors) {
        for (SkColor paintColor : paintColors) {
            compare_antiH_and_mask(r,
                                   backgroundColor,
                                   paintColor,
                                   [](const SkPixmap& device, const SkPaint& paint) -> SkBlitter* {
                                       return new SkARGB32_Opaque_Blitter(device, paint);
                                   });
        }
    }
}
