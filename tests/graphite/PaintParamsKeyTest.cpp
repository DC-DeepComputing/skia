/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkM44.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/SkUniquePaintParamsID.h"
#include "src/core/SkKeyContext.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkShaderCodeDictionary.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/shaders/SkImageShader.h"

namespace {
using namespace skgpu::graphite;

sk_sp<SkImage> make_image() {

}

std::tuple<SkPaint, int> create_paint(ShaderType shaderType,
                                      SkTileMode tm,
                                      SkBlendMode bm) {
    SkPoint pts[2] = {{-100, -100},
                      {100,  100}};
    SkColor colors[2] = {SK_ColorRED, SK_ColorGREEN};
    SkScalar offsets[2] = {0.0f, 1.0f};

    sk_sp<SkShader> s;
    int numTextures = 0;
    switch (shaderType) {
        case ShaderType::kSolidColor:
            break;
        case ShaderType::kLinearGradient:
            s = SkGradientShader::MakeLinear(pts, colors, offsets, 2, tm);
            break;
        case ShaderType::kRadialGradient:
            s = SkGradientShader::MakeRadial({0, 0}, 100, colors, offsets, 2, tm);
            break;
        case ShaderType::kSweepGradient:
            s = SkGradientShader::MakeSweep(0, 0, colors, offsets, 2, tm,
                                            0, 359, 0, nullptr);
            break;
        case ShaderType::kConicalGradient:
            s = SkGradientShader::MakeTwoPointConical({100, 100}, 100,
                                                      {-100, -100}, 100,
                                                      colors, offsets, 2, tm);
            break;
        case ShaderType::kLocalMatrix: {
            s = SkShaders::Color(SK_ColorYELLOW);
            s = s->makeWithLocalMatrix(SkMatrix::Scale(1.5f, 2.0f));
            break;
        }
        case ShaderType::kImage: {
            ++numTextures;
            sk_sp<SkImage> img = make_image();
            s = SkImageShader::Make();
            break;
        }
        case ShaderType::kBlendShader: {
            sk_sp<SkShader> src = SkShaders::Color(SK_ColorGREEN);
            sk_sp<SkShader> dst = SkShaders::Color(SK_ColorLTGRAY);
            s = SkShaders::Blend(SkBlendMode::kColorDodge, std::move(dst), std::move(src));
            break;
        }
    }
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setShader(std::move(s));
    p.setBlendMode(bm);
    return { p, numTextures };
}

} // anonymous namespace

// This is intended to be a smoke test for the agreement between the two ways of creating a
// PaintParamsKey:
//    via ExtractPaintData (i.e., from an SkPaint)
//    and via CreateKey (i.e., via the pre-compilation system)
// TODO: we could turn this into a fuzzer
// TODO: rather than what we're doing here we should:
//   create a PaintCombinations object for a single combination
//   traverse it to create an SkPaint
//   call Context::precompile and, somehow, get the created SkPaintParamsKey
//   draw w/ the SkPaint and, again, somehow, intercept the created SkPaintParamsKey
DEF_GRAPHITE_TEST_FOR_CONTEXTS(PaintParamsKeyTest, reporter, context) {
    using namespace skgpu::graphite;

    auto recorder = context->makeRecorder();
    SkKeyContext keyContext(recorder.get(), {});
    auto dict = keyContext.dict();
    auto tCache = recorder->priv().textureDataCache();

    SkPaintParamsKeyBuilder builder(dict, SkBackend::kGraphite);
    SkPipelineDataGatherer gatherer(Layout::kMetal);

    for (auto s : { ShaderType::kSolidColor,
                    ShaderType::kLinearGradient,
                    ShaderType::kRadialGradient,
                    ShaderType::kSweepGradient,
                    ShaderType::kConicalGradient,
                    ShaderType::kLocalMatrix,
                    ShaderType::kImage,
                    ShaderType::kBlendShader }) {
        for (auto tm: { SkTileMode::kClamp,
                        SkTileMode::kRepeat,
                        SkTileMode::kMirror,
                        SkTileMode::kDecal }) {
            if (s == ShaderType::kSolidColor) {
                tm = SkTileMode::kClamp;  // the TileMode doesn't matter for this case
            }

            for (auto bm : { SkBlendMode::kSrc, SkBlendMode::kSrcOver }) {
                auto [ p, expectedNumTextures ] = create_paint(s, tm, bm);

                auto [ uniqueID1, uIndex, tIndex] = ExtractPaintData(recorder.get(), &gatherer,
                                                                     &builder, {}, PaintParams(p));

                SkUniquePaintParamsID uniqueID2 = CreateKey(keyContext, &builder, s, tm, bm);
                // ExtractPaintData and CreateKey agree
                REPORTER_ASSERT(reporter, uniqueID1 == uniqueID2);

                {
                    const SkTextureDataBlock* textureData = tCache->lookup(tIndex);
                    int actualNumTextures = textureData ? textureData->numTextures() : 0;

                    REPORTER_ASSERT(reporter, expectedNumTextures == actualNumTextures);
                }

                {
                    // TODO: check the blendInfo here too
                }
            }
        }
    }
}
