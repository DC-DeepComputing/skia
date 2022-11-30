/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#ifdef SK_GRAPHITE_ENABLED

#include "include/core/SkBitmap.h"
#include "include/core/SkM44.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/SkUniquePaintParamsID.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/FactoryFunctions.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/Precompile.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/shaders/SkImageShader.h"

using namespace skgpu::graphite;

namespace {

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_random_shader(SkRandom*, Recorder*);
std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> create_random_blender(SkRandom*);
sk_sp<SkImage> make_image(SkRandom*, Recorder*);

enum class ShaderType {
    kSolidColor,
    kLinearGradient,
    kRadialGradient,
    kSweepGradient,
    kConicalGradient,
    kLocalMatrix,
    kImage,
    kBlend,

    kLast          = kBlend
};

static constexpr int kShaderTypeCount = static_cast<int>(ShaderType::kLast) + 1;

SkColor random_opaque_color(SkRandom* rand) {
    return 0xff000000 | rand->nextU();
}

SkBlendMode random_blendmode(SkRandom* rand) {
    return static_cast<SkBlendMode>(rand->nextULessThan(kSkBlendModeCount));
}

SkTileMode random_tilemode(SkRandom* rand) {
    return static_cast<SkTileMode>(rand->nextULessThan(kSkTileModeCount));
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_solid_shader(SkRandom* rand) {
    sk_sp<SkShader> s = SkShaders::Color(random_opaque_color(rand));
    sk_sp<PrecompileShader> o = PrecompileShaders::Color();

    return { s, o };
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_gradient(SkRandom* rand,
                                                                    SkShaderBase::GradientType type) {
    // TODO: fuzz the gradient parameters
    SkPoint pts[2] = {{-100, -100},
                      {100,  100}};
    SkColor colors[2] = {SK_ColorRED, SK_ColorGREEN};
    SkScalar offsets[2] = {0.0f, 1.0f};

    sk_sp<SkShader> s;
    sk_sp<PrecompileShader> o;

    SkTileMode tm = random_tilemode(rand);

    switch (type) {
        case SkShaderBase::GradientType::kLinear:
            s = SkGradientShader::MakeLinear(pts, colors, offsets, 2, tm);
            o = PrecompileShaders::LinearGradient();
            break;
        case SkShaderBase::GradientType::kRadial:
            s = SkGradientShader::MakeRadial({0, 0}, 100, colors, offsets, 2, tm);
            o = PrecompileShaders::RadialGradient();
            break;
        case SkShaderBase::GradientType::kSweep:
            s = SkGradientShader::MakeSweep(0, 0, colors, offsets, 2, tm,
                                            0, 359, 0, nullptr);
            o = PrecompileShaders::SweepGradient();
            break;
        case SkShaderBase::GradientType::kConical:
            s = SkGradientShader::MakeTwoPointConical({100, 100}, 100,
                                                      {-100, -100}, 100,
                                                      colors, offsets, 2, tm);
            o = PrecompileShaders::TwoPointConicalGradient();
            break;
        case SkShaderBase::GradientType::kNone:
        case SkShaderBase::GradientType::kColor:
            break;
    }

    return { s, o };
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_localmatrix(SkRandom* rand,
                                                                       Recorder* recorder) {
    auto [s, o] = create_random_shader(rand, recorder);

    SkMatrix tmp = SkMatrix::Scale(1.5f, 2.0f); // TODO: fuzz

    return { s->makeWithLocalMatrix(tmp), o->makeWithLocalMatrix() };
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_image(SkRandom* rand,
                                                                 Recorder* recorder) {
    SkTileMode tmX = random_tilemode(rand);
    SkTileMode tmY = random_tilemode(rand);

    sk_sp<SkShader> s = SkImageShader::Make(make_image(rand, recorder), tmX, tmY,
                                            SkSamplingOptions(), nullptr);
    sk_sp<PrecompileShader> o = PrecompileShaders::Image();

    return { s, o };
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_blend(SkRandom* rand,
                                                                 Recorder* recorder) {
    auto [blender, blenderO] = create_random_blender(rand);
    auto [dstS, dstO] = create_random_shader(rand, recorder);
    auto [srcS, srcO] = create_random_shader(rand, recorder);

    SkBlendMode bms[] = { random_blendmode(rand) };
    auto s = SkShaders::Blend(std::move(blender), std::move(dstS), std::move(srcS));
    auto o = PrecompileShaders::Blend({ blenderO }, { dstO }, { srcO });

    return { s, o };
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>>  create_shader(SkRandom* rand,
                                                                   Recorder* recorder,
                                                                   ShaderType shaderType) {
    switch (shaderType) {
        case ShaderType::kSolidColor:
            return create_solid_shader(rand);
        case ShaderType::kLinearGradient:
            return create_gradient(rand, SkShaderBase::GradientType::kLinear);
        case ShaderType::kRadialGradient:
            return create_gradient(rand, SkShaderBase::GradientType::kRadial);
        case ShaderType::kSweepGradient:
            return create_gradient(rand, SkShaderBase::GradientType::kSweep);
        case ShaderType::kConicalGradient:
            return create_gradient(rand, SkShaderBase::GradientType::kConical);
        case ShaderType::kLocalMatrix:
            return create_localmatrix(rand, recorder);
        case ShaderType::kImage:
            return create_image(rand, recorder);
        case ShaderType::kBlend:
            return create_blend(rand, recorder);
    }
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_random_shader(SkRandom* rand,
                                                                         Recorder* recorder) {
    int shaderID = rand->nextULessThan(kShaderTypeCount);

    return create_shader(rand, recorder, static_cast<ShaderType>(shaderID));
}

sk_sp<SkImage> make_image(SkRandom* rand, Recorder* recorder) {
    SkImageInfo info = SkImageInfo::Make(32, 32, SkColorType::kRGBA_8888_SkColorType,
                                         kPremul_SkAlphaType);

    SkBitmap bitmap;
    bitmap.allocPixels(info);
    bitmap.eraseColor(SK_ColorBLACK);

    sk_sp<SkImage> img = bitmap.asImage();

    return img->makeTextureImage(recorder);
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> create_blender(SkRandom* rand,
                                                                     SkBlendMode bm) {
    return { SkBlender::Mode(bm), PrecompileBlender::Mode(bm) };
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> create_random_blender(SkRandom* rand) {
    SkBlendMode bm = random_blendmode(rand);

    return { SkBlender::Mode(bm), PrecompileBlender::Mode(bm) };
}

std::pair<SkPaint, PaintOptions> create_paint(SkRandom* rand,
                                              Recorder* recorder,
                                              ShaderType shaderType,
                                              SkBlendMode bm) {
    SkPaint paint;
    paint.setColor(random_opaque_color(rand));

    PaintOptions paintOptions;

    {
        auto [s, o] = create_shader(rand, recorder, shaderType);

        paint.setShader(std::move(s));
        paintOptions.setShaders({ o });
    }

    {
        auto [b, o] = create_blender(rand, bm);

        paint.setBlender(std::move(b));
        paintOptions.setBlenders({ o });
    }

    return { paint, paintOptions };
}

SkUniquePaintParamsID create_key(Context* context, const PaintOptions& paintOptions) {
    context->precompile(paintOptions);
    return {};
}

} // anonymous namespace

// This is intended to be a smoke test for the agreement between the two ways of creating a
// PaintParamsKey:
//    via ExtractPaintData (i.e., from an SkPaint)
//    and via CreateKey (i.e., via the pre-compilation system)
// TODO: we could turn this into a fuzzer
// TODO: rather than what we're doing here we should:
//   create a PaintCombinations object for a single combination
//   traverse it to create an SkPaint (or create it in parallel)
//   call Context::precompile and, somehow, get the created PaintParamsKey
//           - maybe via a testing only callback on ShaderCodeDictionary::findOrCreate
//   draw w/ the SkPaint and, again, somehow, intercept the created PaintParamsKey
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(PaintParamsKeyTest, reporter, context) {
    auto recorder = context->makeRecorder();
    KeyContext keyContext(recorder.get(), {});
    auto dict = keyContext.dict();

    SkRandom rand;

    PaintParamsKeyBuilder builder(dict);
    PipelineDataGatherer gatherer(Layout::kMetal);

    for (auto s : { ShaderType::kSolidColor,
                    ShaderType::kLinearGradient,
                    ShaderType::kRadialGradient,
                    ShaderType::kSweepGradient,
                    ShaderType::kConicalGradient,
                    ShaderType::kLocalMatrix,
                    ShaderType::kImage,
                    ShaderType::kBlend }) {

        // TODO: test out a runtime SkBlender here
        for (auto bm : { SkBlendMode::kSrc, SkBlendMode::kSrcOver }) {
            auto [paint, paintOptions] = create_paint(&rand, recorder.get(), s, bm);

            auto [uniqueID1, uData, tData] = ExtractPaintData(
                    recorder.get(), &gatherer, &builder, Layout::kMetal, {}, PaintParams(paint));

            SkUniquePaintParamsID uniqueID2 = create_key(context, paintOptions);

            // ExtractPaintData and CreateKey need to agree
            REPORTER_ASSERT(reporter, uniqueID1 == uniqueID2);
        }
    }
}

#endif // SK_GRAPHITE_ENABLED
