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
#include "experimental/graphite/src/UniformDataManager.h"
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
static constexpr int kMaxStops = 4;

struct UniformData {
    UniformData(int count,
                const Uniform* uniforms,
                uint32_t* offsets,
                char* data)
            : fCount(count)
            , fUniforms(uniforms)
            , fOffsets(offsets)
            , fData(data) {
    }

    ~UniformData() {
        // TODO: fOffsets and fData should just be allocated right after UniformData in an arena
        delete [] fOffsets;
        delete [] fData;
    }

    const int fCount;
    const Uniform* fUniforms;
    uint32_t* fOffsets; // offset of each uniform in 'fData'
    char* fData;
};

std::unique_ptr<UniformData> allocate_uniform_data(int count,
                                                   const Uniform* uniforms,
                                                   size_t dataSize) {
    // TODO: the offsets and data should just be allocated right after UniformData in an arena
    uint32_t* offsets = new uint32_t[count];
    // TODO: do we need to worry about the alignment of 'data'?
    char* data = new char[dataSize];

    return std::unique_ptr<UniformData>(new UniformData(count, uniforms, offsets, data));
}

std::unique_ptr<UniformData> make_gradient_uniform_data_common(int count,
                                                               const Uniform* uniforms,
                                                               const void* srcs) {
    UniformManager mgr(Layout::kMetal);

    size_t dataSize = mgr.writeUniforms(count, uniforms, nullptr, nullptr, nullptr);

    std::unique_ptr<UniformData> result = allocate_uniform_data(count,
                                                                uniforms,
                                                                dataSize);

    mgr.writeUniforms(count, uniforms, srcs, result->fOffsets, result->fData);
    return result;
}

std::unique_ptr<UniformData> make_linear_gradient_uniform_data(SkPoint startPoint,
                                                               SkPoint endPoint,
                                                               SkColor4f colors[kMaxStops],
                                                               float offsets[kMaxStops]) {
    static constexpr int kNumUniforms = 4;
    static constexpr Uniform kUniforms[kNumUniforms] {
        {"startPoint", SLType::kFloat2},
        {"endPoint",   SLType::kFloat2},
        {"colors",     SLType::kFloat4, kMaxStops},
        {"offsets",    SLType::kFloat, kMaxStops},
    };

    void* srcs[kNumUniforms] = {
        &startPoint,
        &endPoint,
        colors,
        offsets,
    };

    return make_gradient_uniform_data_common(kNumUniforms, kUniforms, srcs);
};

std::unique_ptr<UniformData> make_radial_gradient_uniform_data(SkPoint point,
                                                               float radius,
                                                               SkColor4f colors[kMaxStops],
                                                               float offsets[kMaxStops]) {
    static constexpr int kNumUniforms = 4;
    static constexpr Uniform kUniforms[kNumUniforms] {
        {"point",   SLType::kFloat2 },
        {"radius",  SLType::kFloat },
        {"colors",  SLType::kFloat4 , kMaxStops },
        {"offsets", SLType::kFloat, kMaxStops },
    };

    void* srcs[kNumUniforms] = {
        &point,
        &radius,
        colors,
        offsets,
    };

    return make_gradient_uniform_data_common(kNumUniforms, kUniforms, srcs);
};

std::unique_ptr<UniformData> make_sweep_gradient_uniform_data(SkPoint point,
                                                              SkColor4f colors[kMaxStops],
                                                              float offsets[kMaxStops]) {
    static constexpr int kNumUniforms = 3;
    static constexpr Uniform kUniforms[kNumUniforms] {
        {"point",   SLType::kFloat2 },
        {"colors",  SLType::kFloat4 , kMaxStops },
        {"offsets", SLType::kFloat, kMaxStops },
    };

    void* srcs[kNumUniforms] = {
        &point,
        colors,
        offsets,
    };

    return make_gradient_uniform_data_common(kNumUniforms, kUniforms, srcs);
};

std::unique_ptr<UniformData> make_conical_gradient_uniform_data(SkPoint point0,
                                                                SkPoint point1,
                                                                float radius0,
                                                                float radius1,
                                                                SkColor4f colors[kMaxStops],
                                                                float offsets[kMaxStops]) {
    static constexpr int kNumUniforms = 6;
    static constexpr Uniform kUniforms[kNumUniforms] {
            {"point0",   SLType::kFloat2 },
            {"point1",   SLType::kFloat2 },
            {"radius0",  SLType::kFloat },
            {"radius1",  SLType::kFloat },
            {"colors",  SLType::kFloat4 , kMaxStops },
            {"offsets", SLType::kFloat, kMaxStops },
    };

    void* srcs[kNumUniforms] = {
            &point0,
            &point1,
            &radius0,
            &radius1,
            colors,
            offsets,
    };

    return make_gradient_uniform_data_common(kNumUniforms, kUniforms, srcs);
};

void to_color4fs(int numColors, SkColor colors[kMaxStops], SkColor4f color4fs[kMaxStops]) {
    SkASSERT(numColors >= 2 && numColors <= kMaxStops);

    int i;
    for (i = 0; i < numColors; ++i) {
        color4fs[i] = SkColor4f::FromColor(colors[i]);
    }
    for ( ; i < kMaxStops; ++i) {
        color4fs[i] = color4fs[numColors-1];
    }
}

void expand_stops(int numStops, float offsets[kMaxStops]) {
    SkASSERT(numStops >= 2 && numStops <= kMaxStops);

    for (int i = numStops ; i < kMaxStops; ++i) {
        offsets[i] = offsets[numStops-1];
    }
}

void extract_combo(const SkPaint& p) {
    Combo result;
    std::unique_ptr<UniformData> uniforms;

    if (auto s = p.getShader()) {
        SkColor colors[kMaxStops];
        SkColor4f color4fs[kMaxStops];
        float   offsets[kMaxStops];
        SkShader::GradientInfo gradInfo;

        gradInfo.fColorCount = kMaxStops;
        gradInfo.fColors = colors;
        gradInfo.fColorOffsets = offsets;

        SkShader::GradientType type = s->asAGradient(&gradInfo);
        if (gradInfo.fColorCount > kMaxStops) {
            type = SkShader::GradientType::kNone_GradientType;
        }

        switch (type) {
            case SkShader::kLinear_GradientType: {
                to_color4fs(gradInfo.fColorCount, colors, color4fs);
                expand_stops(gradInfo.fColorCount, offsets);

                result.fShaderType = ShaderCombo::ShaderType::kLinearGradient;
                result.fTileMode = gradInfo.fTileMode;

                uniforms = make_linear_gradient_uniform_data(gradInfo.fPoint[0],
                                                             gradInfo.fPoint[1],
                                                             color4fs,
                                                             offsets);
            } break;
            case SkShader::kRadial_GradientType: {
                to_color4fs(gradInfo.fColorCount, colors, color4fs);
                expand_stops(gradInfo.fColorCount, offsets);

                result.fShaderType = ShaderCombo::ShaderType::kRadialGradient;
                result.fTileMode = gradInfo.fTileMode;

                uniforms =  make_radial_gradient_uniform_data(gradInfo.fPoint[0],
                                                              gradInfo.fRadius[0],
                                                              color4fs,
                                                              offsets);
            } break;
            case SkShader::kSweep_GradientType:
                to_color4fs(gradInfo.fColorCount, colors, color4fs);
                expand_stops(gradInfo.fColorCount, offsets);

                result.fShaderType = ShaderCombo::ShaderType::kSweepGradient;
                result.fTileMode = gradInfo.fTileMode;

                uniforms = make_sweep_gradient_uniform_data(gradInfo.fPoint[0],
                                                            color4fs,
                                                            offsets);
                break;
            case SkShader::GradientType::kConical_GradientType:
                to_color4fs(gradInfo.fColorCount, colors, color4fs);
                expand_stops(gradInfo.fColorCount, offsets);

                result.fShaderType = ShaderCombo::ShaderType::kConicalGradient;
                result.fTileMode = gradInfo.fTileMode;

                uniforms = make_conical_gradient_uniform_data(gradInfo.fPoint[0],
                                                              gradInfo.fPoint[1],
                                                              gradInfo.fRadius[0],
                                                              gradInfo.fRadius[1],
                                                              color4fs,
                                                              offsets);
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

// From MtlToy
struct FragmentUniforms {
    vector_float2 startPos;
    vector_float2 endPos;

    vector_float4 color0;
    vector_float4 color1;
    vector_float4 color2;
    vector_float4 color3;

    vector_float4 stops;
    simd_uint2 rtSize;
};

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
