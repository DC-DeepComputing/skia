/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "src/core/SkColorData.h"
#include "tools/ToolUtils.h"

DEF_SIMPLE_GM(blend, canvas, 300, 100) {
    SkPaint p;

    // All three of these blocks should be the same color.
    canvas->save();
        canvas->scale(100,100);

        p.setColor(SK_ColorRED);
        canvas->drawRect(SkRect::MakeXYWH(0,0,1,1), p);
        p.setColor(0xFC208000);
        canvas->drawRect(SkRect::MakeXYWH(0,0,1,1), p);

        p.setColor(SK_ColorRED);
        canvas->drawRect(SkRect::MakeXYWH(1,0,1,1), p);
        canvas->saveLayer(nullptr, nullptr);
            p.setColor(0xFC208000);
            canvas->drawRect(SkRect::MakeXYWH(1,0,1,1), p);
        canvas->restore();

        p.setColor(SK_ColorRED);
        canvas->drawRect(SkRect::MakeXYWH(2,0,1,1), p);
        canvas->saveLayerAlpha(nullptr, 0xFC);
        p.setColor(ToolUtils::color_to_565(0xFF208000));
        canvas->drawRect(SkRect::MakeXYWH(2, 0, 1, 1), p);
        canvas->restore();
    canvas->restore();
}

static SkPMColor accurate_blend_argb32(SkPMColor src, SkPMColor dst, uint32_t aa) {
    uint32_t src_scale = aa;
    uint32_t dst_scale = 255 - (SkGetPackedA32(src) * aa) / 255;

    uint32_t new_alpha = (src_scale * SkGetPackedA32(src) + dst_scale * SkGetPackedA32(dst)) / 255;
    SkASSERT(new_alpha <= 255);

    uint32_t new_red = (src_scale * SkGetPackedR32(src) + dst_scale * SkGetPackedR32(dst)) / 255;
    SkASSERT(new_red <= 255);

    uint32_t new_green = (src_scale * SkGetPackedG32(src) + dst_scale * SkGetPackedG32(dst)) / 255;
    SkASSERT(new_green <= 255);

    uint32_t new_blue = (src_scale * SkGetPackedB32(src) + dst_scale * SkGetPackedB32(dst)) / 255;
    SkASSERT(new_blue <= 255);

    return SkPackARGB32(new_alpha, new_red, new_green, new_blue);
}

static SkPMColor D32_A8_Opaque_Color_neon(SkPMColor src, SkPMColor dst, uint32_t aa) {
    uint32_t src_scale = SkAlpha255To256(aa);
    uint32_t dst_scale = 256 - (SkGetPackedA32(src) * SkAlpha255To256(aa)) / 256;

    uint32_t new_alpha = (src_scale * SkGetPackedA32(src)) / 256 + (dst_scale * SkGetPackedA32(dst)) / 256;
    SkASSERT(new_alpha <= 255);

    uint32_t new_red = (src_scale * SkGetPackedR32(src)) / 256 + (dst_scale * SkGetPackedR32(dst)) / 256;
    SkASSERT(new_red <= 255);

    uint32_t new_green = (src_scale * SkGetPackedG32(src)) / 256 + (dst_scale * SkGetPackedG32(dst)) / 256;
    SkASSERT(new_green <= 255);

    uint32_t new_blue = (src_scale * SkGetPackedB32(src)) / 256 + (dst_scale * SkGetPackedB32(dst)) / 256;
    SkASSERT(new_blue <= 255);

    return SkPackARGB32(new_alpha, new_red, new_green, new_blue);
}

DEF_SIMPLE_GM(blend_translucent_colors_with_alpha, canvas, 300, 300) {
    canvas->clear(SK_ColorWHITE);
    canvas->translate(20, 20);

    SkPaint p;

    float total_errors = 0.f;
    for (int dst_shade = 0; dst_shade <= 255; ++dst_shade) {
        // opaque grey
        const SkPMColor dst = SkPreMultiplyColor(SkColorSetRGB(dst_shade, dst_shade, dst_shade));
        for (int src_shade = 0; src_shade <= 255; ++src_shade) {
            int errors = 0;
                const SkPMColor src = SkPreMultiplyColor(SkColorSetARGB(255, src_shade, src_shade, src_shade));
                for (int blend_alpha = 0; blend_alpha <= 255; ++blend_alpha) {
                    const SkPMColor actual = accurate_blend_argb32(src, dst, blend_alpha);
                    const SkPMColor approx = D32_A8_Opaque_Color_neon(src, dst, blend_alpha);
                    // const SkPMColor approx = SkBlendARGB32(src, dst, blend_alpha);

                    if (actual != approx) {
                        errors += 1;
                    }
                }
            total_errors += errors;
            SkColor error_shade = SkColorSetRGB(errors, 0, 0);
            p.setColor(error_shade);
            canvas->drawRect(SkRect::MakeXYWH(dst_shade, src_shade, 1, 1), p);
        }
    }
    SkDebugf("Total error %f%%\n", (total_errors / 0xffffff) * 100);
}