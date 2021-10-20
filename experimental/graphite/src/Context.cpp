/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/Context.h"

#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/Gpu.h"
#include "experimental/graphite/src/Recorder.h"
#include "experimental/graphite/src/Recording.h"
#include "experimental/graphite/src/Uniform.h"
#include "include/core/SkSpan.h"

#ifdef SK_METAL
#include "experimental/graphite/src/mtl/MtlTrampoline.h"
#endif

// TODO: these are only included to play with shader combinations
#include "include/core/SkPaint.h"
#include "include/effects/SkGradientShader.h"


namespace skgpu {

namespace {

// This captures a single SkPaint state
struct Combo {
    ShaderCombo::ShaderType fShaderType = ShaderCombo::ShaderType::kNone;
    SkTileMode fTileMode = SkTileMode::kRepeat;
    SkBlendMode fBlendMode = SkBlendMode::kSrc;
};
static constexpr int kMaxStops = 8;

class UniformData {
public:

protected:
    /**
     * Specifies the uniforms used by this processor. Should be called when the processor is made
     * (i.e. constructor or factory function). Any uniforms with type void are ignored. This allows
     * a processor to have a contiguous array of data member uniforms where some are conditionally
     * initialized.
     */
    void setUniforms(SkSpan<const Uniform> uniforms) { fUniforms = uniforms; }

private:
    SkSpan<const Uniform> fUniforms;
};

struct LinearGradientUniformData : public UniformData {
    LinearGradientUniformData(SkPoint points[2],
                              SkColor colors[kMaxStops],
                              float offset[kMaxStops],
                              int numStops) {
//        GR_BEGIN_UNIFORM_DEFINITIONS
        static constexpr Uniform kUniforms[3] {
            {kFloat2_GrSLType, 2, offsetof(LinearGradientUniformData, fPoints)},
            {kByte4_GrSLType , kMaxStops, offsetof(LinearGradientUniformData, fColors)},
            {kFloat_GrSLType , kMaxStops, offsetof(LinearGradientUniformData, fOffsets)},
        };
//        GR_END_UNIFORM_DEFINITIONS

        this->setUniforms(SkMakeSpan(kUniforms));
    }

    SkPoint fPoints[2];
    SkColor fColors[kMaxStops];
    float   fOffsets[kMaxStops];
};

struct RadialGradientUniformData : public UniformData {
    RadialGradientUniformData(SkPoint point,
                              float radius,
                              SkColor colors[kMaxStops],
                              float offset[kMaxStops],
                              int numStops)
            : fPoint(point)
            , fRadius(radius) {
//        GR_BEGIN_UNIFORM_DEFINITIONS
        static constexpr Uniform kUniforms[4] {
            {kFloat2_GrSLType, offsetof(RadialGradientUniformData, fPoint)},
            {kFloat_GrSLType,  offsetof(RadialGradientUniformData, fRadius)},
            {kByte4_GrSLType , kMaxStops, offsetof(RadialGradientUniformData, fColors)},
            {kFloat_GrSLType , kMaxStops, offsetof(RadialGradientUniformData, fOffsets)},
        };
//        GR_END_UNIFORM_DEFINITIONS

        this->setUniforms(SkMakeSpan(kUniforms));
    }

    SkPoint fPoint;
    float   fRadius;
    SkColor fColors[kMaxStops];
    float   fOffsets[kMaxStops];
};

struct SweepGradientUniformData : public UniformData {
    SweepGradientUniformData(SkPoint point,
                             SkColor colors[kMaxStops],
                             float offset[kMaxStops],
                             int numStops)
            : fPoint(point) {
//        GR_BEGIN_UNIFORM_DEFINITIONS
        static constexpr Uniform kUniforms[4] {
            {kFloat2_GrSLType, offsetof(SweepGradientUniformData, fPoint)},
            {kByte4_GrSLType , kMaxStops, offsetof(RadialGradientUniformData, fColors)},
            {kFloat_GrSLType , kMaxStops, offsetof(RadialGradientUniformData, fOffsets)},
        };
//        GR_END_UNIFORM_DEFINITIONS

        this->setUniforms(SkMakeSpan(kUniforms));
    }

    SkPoint fPoint;
    SkColor fColors[kMaxStops];
    float   fOffsets[kMaxStops];
};

struct ConicalGradientUniformData : public UniformData {
    ConicalGradientUniformData(SkPoint point[2],
                               float radius[2],
                               SkColor colors[kMaxStops],
                               float offset[kMaxStops],
                               int numStops) {
//        GR_BEGIN_UNIFORM_DEFINITIONS
        static constexpr Uniform kUniforms[4] {
                {kFloat2_GrSLType, 2, offsetof(ConicalGradientUniformData, fPoint)},
                {kFloat_GrSLType, 2, offsetof(ConicalGradientUniformData, fRadius)},
                {kByte4_GrSLType , kMaxStops, offsetof(ConicalGradientUniformData, fColors)},
                {kFloat_GrSLType , kMaxStops, offsetof(ConicalGradientUniformData, fOffsets)},
        };
//        GR_END_UNIFORM_DEFINITIONS

        this->setUniforms(SkMakeSpan(kUniforms));
    }

    SkPoint fPoint[2];
    float   fRadius[2];
    SkColor fColors[kMaxStops];
    float   fOffsets[kMaxStops];
};

void extract_combo(const SkPaint& p) {
    Combo result;
    std::unique_ptr<UniformData> uniforms;

    if (auto s = p.getShader()) {
        SkColor colors[kMaxStops];
        float   offsets[kMaxStops];
        SkShader::GradientInfo gradInfo;

        gradInfo.fColorCount = kMaxStops;
        gradInfo.fColors = colors;
        gradInfo.fColorOffsets = offsets;

        SkShader::GradientType type = s->asAGradient(&gradInfo);
        switch (type) {
            case SkShader::kLinear_GradientType: {
                result.fShaderType = ShaderCombo::ShaderType::kLinearGradient;
                result.fTileMode = gradInfo.fTileMode;

                uniforms = std::unique_ptr<UniformData>(
                        new LinearGradientUniformData(gradInfo.fPoint,
                                                      colors,
                                                      offsets,
                                                      gradInfo.fColorCount));
            } break;
            case SkShader::kRadial_GradientType: {
                result.fShaderType = ShaderCombo::ShaderType::kRadialGradient;
                result.fTileMode = gradInfo.fTileMode;

                uniforms = std::unique_ptr<UniformData>(
                        new RadialGradientUniformData(gradInfo.fPoint[0],
                                                      gradInfo.fRadius[0],
                                                      colors,
                                                      offsets,
                                                      gradInfo.fColorCount));
            } break;
            case SkShader::kSweep_GradientType:
                result.fShaderType = ShaderCombo::ShaderType::kSweepGradient;
                result.fTileMode = gradInfo.fTileMode;

                uniforms = std::unique_ptr<UniformData>(
                        new SweepGradientUniformData(gradInfo.fPoint[0],
                                                     colors,
                                                     offsets,
                                                     gradInfo.fColorCount));
                break;
            case SkShader::GradientType::kConical_GradientType:
                result.fShaderType = ShaderCombo::ShaderType::kConicalGradient;
                result.fTileMode = gradInfo.fTileMode;

                uniforms = std::unique_ptr<UniformData>(
                        new ConicalGradientUniformData(gradInfo.fPoint,
                                                       gradInfo.fRadius,
                                                       colors,
                                                       offsets,
                                                       gradInfo.fColorCount));
                break;
            case SkShader::GradientType::kColor_GradientType:
            case SkShader::GradientType::kNone_GradientType:
            default:
                ;
        }
    }

    result.fBlendMode = p.getBlendMode_or(SkBlendMode::kSrcOver);
}

#if 0
fragment float4 fragmentShader(VertexOut interpolated [[stage_in]],
constant FragmentUniforms &uniforms [[buffer(0)]])
{
float2 screenPos = float2(2*interpolated.pos.x/uniforms.rtSize[0] - 1,
                          2*interpolated.pos.y/uniforms.rtSize[1] - 1);
float2 delta = uniforms.endPos - uniforms.startPos;
float2 pt = screenPos - uniforms.startPos;
float t = dot(pt, delta) / dot(delta, delta);
float4 result = uniforms.color0;
result = mix(result, uniforms.color1, clamp((t-uniforms.stops.x)/(uniforms.stops.y-uniforms.stops.x),
                                            0.0, 1.0));
result = mix(result, uniforms.color2, clamp((t-uniforms.stops.y)/(uniforms.stops.z-uniforms.stops.y),
                                            0.0, 1.0));
result = mix(result, uniforms.color3, clamp((t-uniforms.stops.z)/(uniforms.stops.w-uniforms.stops.z),
                                            0.0, 1.0));
return result;
//      return float4(uniforms.color1);
}
#endif

void build_SKSL(const Combo& combo) {
    SkPoint pts[2] = { { -100, -100 }, { 100, 100 } };
    SkColor colors[2] = { SK_ColorRED, SK_ColorGREEN };
    SkScalar offsets[2] = { 0.0f, 1.0f };

    sk_sp<SkShader> s;
    switch (combo.fShaderType) {
        case ShaderCombo::ShaderType::kNone:
            break;
        case ShaderCombo::ShaderType::kLinearGradient:
            s = SkGradientShader::MakeLinear(pts, colors, offsets, 2, combo.fTileMode);
            break;
        case ShaderCombo::ShaderType::kRadialGradient:
            s = SkGradientShader::MakeRadial({0, 0}, 100, colors, offsets, 2, combo.fTileMode);
            break;
        case ShaderCombo::ShaderType::kSweepGradient:
            s = SkGradientShader::MakeSweep(0, 0, colors, offsets, 2);
            break;
        case ShaderCombo::ShaderType::kConicalGradient:
            s = SkGradientShader::MakeTwoPointConical({100, 100}, 100,
                                                      {-100, -100}, 100,
                                                      colors, offsets, 2, combo.fTileMode);
            break;
    }
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setShader(std::move(s));
    p.setBlendMode(combo.fBlendMode);

    extract_combo(p);
}

// TODO: Going forward this will change into:
//    a method that takes a single combination and builds the SkSL (i.e., buildSKSL(combo))
//    a method that extracts a combination from an SkPaint (i.e., extractCombo(SkPaint))
// buildSKSL will have to collect the number of uniforms and their types required by the SkSL
// extractCombo will have to collect the actual uniforms
void expand(const PaintCombo& c) {
    for (auto bm : c.fBlendModes) {
        for (auto& shaderCombo : c.fShaders) {
            for (auto shaderType : shaderCombo.fTypes) {
                for (auto tm: shaderCombo.fTileModes) {
                    build_SKSL({shaderType, tm, bm});
                }
            }
        }
    }
}

} // anonymous namespace

Context::Context(sk_sp<Gpu> gpu) : fGpu(std::move(gpu)) {}
Context::~Context() {}

#ifdef SK_METAL
sk_sp<Context> Context::MakeMetal(const mtl::BackendContext& backendContext) {
    sk_sp<Gpu> gpu = mtl::Trampoline::MakeGpu(backendContext);
    if (!gpu) {
        return nullptr;
    }

    return sk_sp<Context>(new Context(std::move(gpu)));
}
#endif


sk_sp<Recorder> Context::createRecorder() {
    return sk_make_sp<Recorder>(sk_ref_sp(this));
}

void Context::insertRecording(std::unique_ptr<Recording> recording) {
    fRecordings.emplace_back(std::move(recording));
}

void Context::submit(SyncToCpu syncToCpu) {
    // TODO: we want Gpu::submit to take an array of command buffers but, for now, it just takes
    // one. Once we have more than one recording queued up we will need to extract the
    // command buffers and submit them as a block.
    SkASSERT(fRecordings.size() == 1);
    fGpu->submit(fRecordings[0]->fCommandBuffer);

    fGpu->checkForFinishedWork(syncToCpu);
    fRecordings.clear();
}

void Context::preCompile(const PaintCombo& c) {
    expand(c);
}

} // namespace skgpu
