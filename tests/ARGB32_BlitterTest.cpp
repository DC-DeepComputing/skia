#include "tests/Test.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkMask.h"

static void compare_antiH_and_mask(skiatest::Reporter* reporter,
                                    SkColor backgroundColor,
                                      SkColor paintColor) {
    // Space for a 1 pixel image
    uint8_t buffer[1 * 1 * 4];
    auto ii = SkImageInfo::Make({1, 1},
        SkColorInfo(kN32_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB()));

    SkPixmap device(ii, buffer, ii.minRowBytes());
    auto surface = SkSurfaces::WrapPixels(device);
    SkASSERT(surface);

    SkPaint paint;
    paint.setColor(paintColor);

    SkARGB32_Blitter blitter(device, paint);

    SkAlpha antiAlias[1];
    int16_t runs[2] = {1, 0}; // Single pixel run

    uint8_t maskImage[1];
    auto maskBounds = SkIRect::MakeXYWH(0, 0, 1, 1);
    // Don't test alpha = 255 because blitMask asserts it's not called
    // (and one could just call blitRect)
    for (int alpha = 0; alpha < 255; ++alpha) {
        antiAlias[0] = static_cast<SkAlpha>(alpha);
        maskImage[0] = static_cast<SkAlpha>(alpha);

        SkMask mask(maskImage, maskBounds, 1, SkMask::kA8_Format);

        surface->getCanvas()->clear(backgroundColor);
        blitter.blitAntiH(0, 0, antiAlias, runs);
        SkColor antiHColor = device.getColor(0, 0);

        surface->getCanvas()->clear(backgroundColor);
        blitter.blitMask(mask, mask.fBounds);
        SkColor maskColor = device.getColor(0, 0);

        // Compare the results
        if (antiHColor != maskColor) {
            ERRORF(reporter, "background=%x, paint=%x, alpha=%d, "
                             "blitAntiH=%x, blitMask=%x",
                   backgroundColor, paintColor, alpha, antiHColor, maskColor);
        }
    }
}

DEF_TEST(SkARGB32Blitter_twoBlitMethodsDrawTheSame, r) {
    SkColor backgroundColors[] = {
        SK_ColorWHITE, SK_ColorBLACK, SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE
    };
    SkColor paintColors[] = {
        SK_ColorBLACK, SK_ColorWHITE, SK_ColorBLUE, SK_ColorRED, SK_ColorGREEN
    };

    for (SkColor backgroundColor : backgroundColors) {
        for (SkColor paintColor : paintColors) {
            compare_antiH_and_mask(r, backgroundColor, paintColor);
        }
    }
}