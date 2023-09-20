/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkPaint.h"
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkColorFilterPriv.h"

sk_sp<SkShader> color_shader(SkColor4f color) {
    SkBitmap bmp;
    bmp.allocPixels(SkImageInfo::Make(1, 1, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bmp.eraseColor(color);
    return bmp.makeShader(SkFilterMode::kNearest);
}

sk_sp<SkColorFilter> raw_cf(SkColor4f color) {
    return SkRuntimeEffect::MakeForColorFilter(SkString("uniform half4 c;"
                                                        "half4 main(half4 color) { return c; }"))
            .effect->makeColorFilter(SkData::MakeWithCopy(&color, sizeof(SkColor4f)));
}

sk_sp<SkColorFilter> managed_cf(SkColor4f color) {
    return SkRuntimeEffect::MakeForColorFilter(SkString("layout(color) uniform half4 c;"
                                                        "half4 main(half4 color) { return c; }"))
            .effect->makeColorFilter(SkData::MakeWithCopy(&color, sizeof(SkColor4f)));
}

sk_sp<SkColorFilter> indirect_cf(SkColor4f color) {
    SkRuntimeEffect::ChildPtr children[] = {color_shader(color)};
    return SkRuntimeEffect::MakeForColorFilter(
                   SkString("uniform shader s;"
                            "half4 main(half4 color) { return s.eval(float2(0)); }"))
            .effect->makeColorFilter(nullptr, children);
}

sk_sp<SkColorFilter> mode_cf(SkColor4f color) {
    return SkColorFilters::Blend(color, nullptr, SkBlendMode::kSrc);
}

DEF_SIMPLE_GM(workingspace, canvas, 200, 200) {
    // This GM checks that changing the working color space of a color filter does the right thing.
    // We create a variety of color filters that are sensitive to the working space, and test them
    // with both fixed-input color (so that paint optimization can apply the filter on the CPU),
    // and with a shader input (so they're forced to evaluate in the drawing pipeline).
    //
    // In all cases, the tests are designed to draw green if implemented correctly. Any other color
    // (red or blue, most likely) is an error.

    canvas->translate(5, 5);
    canvas->save();

    auto cell = [&](sk_sp<SkShader> shader, sk_sp<SkColorFilter> colorFilter) {
        SkPaint paint;
        paint.setShader(shader);
        paint.setColorFilter(colorFilter);
        canvas->drawRect({0, 0, 40, 40}, paint);
        canvas->translate(50, 0);
    };

    auto nextRow = [&]() {
        canvas->restore();
        canvas->translate(0, 50);
        canvas->save();
    };

    auto black_shader = color_shader(SkColors::kBlack);

    auto spinCS = SkColorSpace::MakeSRGB()->makeColorSpin();
    skcms_Matrix3x3 spinMtx;
    SkAssertResult(spinCS->toXYZD50(&spinMtx));
    auto spin = [&](sk_sp<SkColorFilter> cf) {
        return SkColorFilterPriv::WithWorkingFormat(std::move(cf), nullptr, &spinMtx, nullptr);
    };

    cell(nullptr, raw_cf(SkColors::kGreen));
    cell(nullptr, managed_cf(SkColors::kGreen));
    cell(nullptr, indirect_cf(SkColors::kGreen));
    cell(nullptr, mode_cf(SkColors::kGreen));

    nextRow();

    cell(black_shader, raw_cf(SkColors::kGreen));
    cell(black_shader, managed_cf(SkColors::kGreen));
    cell(black_shader, indirect_cf(SkColors::kGreen));
    cell(black_shader, mode_cf(SkColors::kGreen));

    nextRow();

    cell(nullptr, spin(raw_cf(SkColors::kRed)));  // Un-managed red turns into green
    cell(nullptr, spin(managed_cf(SkColors::kGreen)));
    cell(nullptr, spin(indirect_cf(SkColors::kGreen)));
    cell(nullptr, spin(mode_cf(SkColors::kGreen)));

    nextRow();

    cell(black_shader, spin(raw_cf(SkColors::kRed)));  // Un-managed red turns into green
    cell(black_shader, spin(managed_cf(SkColors::kGreen)));
    cell(black_shader, spin(indirect_cf(SkColors::kGreen)));
    cell(black_shader, spin(mode_cf(SkColors::kGreen)));
}
