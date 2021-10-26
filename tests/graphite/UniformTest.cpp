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

std::tuple<SkPaint, int> create_paint(skgpu::Combination combo) {
    SkPoint pts[2] = {{-100, -100},
                      {100,  100}};
    SkColor colors[2] = {SK_ColorRED, SK_ColorGREEN};
    SkScalar offsets[2] = {0.0f, 1.0f};

    sk_sp<SkShader> s;
    int numUniforms = 0;
    switch (combo.fShaderType) {
        case skgpu::ShaderCombo::ShaderType::kNone:
            numUniforms += 1;
            break;
        case skgpu::ShaderCombo::ShaderType::kLinearGradient:
            s = SkGradientShader::MakeLinear(pts, colors, offsets, 2, combo.fTileMode);
            numUniforms += 6;
            break;
        case skgpu::ShaderCombo::ShaderType::kRadialGradient:
            s = SkGradientShader::MakeRadial({0, 0}, 100, colors, offsets, 2, combo.fTileMode);
            numUniforms += 6;
            break;
        case skgpu::ShaderCombo::ShaderType::kSweepGradient:
            s = SkGradientShader::MakeSweep(0, 0, colors, offsets, 2);
            numUniforms += 6;
            break;
        case skgpu::ShaderCombo::ShaderType::kConicalGradient:
            s = SkGradientShader::MakeTwoPointConical({100, 100}, 100,
                                                      {-100, -100}, 100,
                                                      colors, offsets, 2, combo.fTileMode);
            numUniforms += 6;
            break;
    }
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setShader(std::move(s));
    p.setBlendMode(combo.fBlendMode);
    return { p, numUniforms };
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

                auto [ p, numUniforms ] = create_paint(expected);
                auto [ actual, uniforms] = skgpu::ExtractCombo(p);
                SkASSERT(expected == actual);
                SkASSERT(numUniforms == uniforms->fCount);
                for (int i = 0; uniforms->fCount; ++i) {
                    SkASSERT(uniforms->fOffsets[i] >= 0 &&
                             uniforms->fOffsets[i] < uniforms->fDataSize);
                }
            }
        }
    }

}
