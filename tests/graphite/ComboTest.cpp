/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/Encoding.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/SkPaintParamsKey.h"
#include "include/private/SkShaderCodeDictionary.h"
#include "src/core/SkPaintPriv.h"
#include "tests/Test.h"

static sk_sp<SkShader> make_image_shader(int imageWidth, int imageHeight,
                                         SkTileMode xTile, SkTileMode yTile,
                                         SkColor color) {
    auto surface = SkSurface::MakeRasterN32Premul(imageWidth, imageHeight);
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(color);
    return surface->makeImageSnapshot()->makeShader(xTile, yTile, SkSamplingOptions());
}

static sk_sp<SkShader> make_gradient_shader() {
    SkPoint pts[2];
    SkColor colors[2];

    pts[0].set(0, 0);
    pts[1].set(SkIntToScalar(100), 0);
    colors[0] = SK_ColorRED;
    colors[1] = SK_ColorBLUE;
    auto grad = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
    return grad;
}

static sk_sp<SkShader> make_blend(sk_sp<SkShader> shaderA,
                                  sk_sp<SkShader> shaderB,
                                  SkBlendMode mode) {
    return SkShaders::Blend(mode, std::move(shaderA), std::move(shaderB));
}

static void doit(SkShaderCodeDictionary* dict, const SkPaint& paint) {
    auto keys = SkPaintPriv::ToKeys(paint, dict, SkBackend::kGanesh, nullptr);

    for (auto k : keys) {
        k.dump();
    }
}

DEF_GRAPHITE_TEST(ComboTest, r) {
    SkShaderCodeDictionary dict;

#if 0
    {
        SkPaint paint;
        paint.setShader(make_image_shader(16, 16, SkTileMode::kClamp, SkTileMode::kRepeat, SK_ColorRED));
        doit(&dict, paint);
    }
#endif

    {
        SkPaint paint;
        auto shaderA = make_image_shader(16, 16, SkTileMode::kDecal, SkTileMode::kRepeat, SK_ColorBLUE);
        auto shaderB = make_image_shader(16, 16, SkTileMode::kRepeat, SkTileMode::kClamp, SK_ColorGREEN);
        paint.setShader(make_blend(std::move(shaderA), std::move(shaderB), SkBlendMode::kDstIn));
        doit(&dict, paint);
    }
}
