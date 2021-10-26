/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "experimental/graphite/src/ContextUtils.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkGradientShader.h"

namespace {

SkPaint create_paint(skgpu::Combination combo) {
    SkPoint pts[2] = {{-100, -100},
                      {100,  100}};
    SkColor colors[2] = {SK_ColorRED, SK_ColorGREEN};
    SkScalar offsets[2] = {0.0f, 1.0f};

    sk_sp<SkShader> s;
    switch (combo.fShaderType) {
        case skgpu::ShaderCombo::ShaderType::kNone:
            break;
        case skgpu::ShaderCombo::ShaderType::kLinearGradient:
            s = SkGradientShader::MakeLinear(pts, colors, offsets, 2, combo.fTileMode);
            break;
        case skgpu::ShaderCombo::ShaderType::kRadialGradient:
            s = SkGradientShader::MakeRadial({0, 0}, 100, colors, offsets, 2, combo.fTileMode);
            break;
        case skgpu::ShaderCombo::ShaderType::kSweepGradient:
            s = SkGradientShader::MakeSweep(0, 0, colors, offsets, 2);
            break;
        case skgpu::ShaderCombo::ShaderType::kConicalGradient:
            s = SkGradientShader::MakeTwoPointConical({100, 100}, 100,
                                                      {-100, -100}, 100,
                                                      colors, offsets, 2, combo.fTileMode);
            break;
    }
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setShader(std::move(s));
    p.setBlendMode(combo.fBlendMode);
    return p;
}

} // anonymous namespace

DEF_GRAPHITE_TEST(UniformTest, reporter) {
    for (auto s : { skgpu::ShaderCombo::ShaderType::kNone,
                    skgpu::ShaderCombo::ShaderType::kLinearGradient,
                    skgpu::ShaderCombo::ShaderType::kRadialGradient,
                    skgpu::ShaderCombo::ShaderType::kSweepGradient,
                    skgpu::ShaderCombo::ShaderType::kConicalGradient }) {
        for (auto tm: { SkTileMode::kClamp,
                        SkTileMode::kRepeat,
                        SkTileMode::kMirror,
                        SkTileMode::kDecal }) {
            for (auto bm : { SkBlendMode::kSrc, SkBlendMode::kSrcOver }) {
                skgpu::Combination expected;

                expected.fShaderType = s;
                expected.fTileMode = tm;
                expected.fBlendMode = bm;

                SkPaint p = create_paint(expected);
                skgpu::Combination actual = skgpu::ExtractCombo(p);
                SkASSERT(expected == actual);
            }
        }
    }

}
