/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ContextUtils.h"

#include <string>
#include "experimental/graphite/src/ContextPriv.h"
#include "experimental/graphite/src/DrawTypes.h"
#include "experimental/graphite/src/PaintParams.h"
#include "experimental/graphite/src/UniformManager.h"
#include "include/core/SkPaint.h"
#include "include/private/SkShaderCodeDictionary.h"
#include "include/private/SkUniquePaintParamsID.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkUniform.h"
#include "src/core/SkUniformData.h"

namespace skgpu {

namespace {

// TODO: For the sprint we unify all the gradient uniforms into a standard set of 6:
//   kMaxStops colors
//   kMaxStops offsets
//   2 points
//   2 radii
static constexpr int kNumGradientUniforms = 6;
static constexpr SkUniform kGradientUniforms[kNumGradientUniforms] {
        {"colors",   SkSLType::kHalf4 , GradientShaderBlocks::GradientData::kMaxStops },
        {"offsets",  SkSLType::kFloat, GradientShaderBlocks::GradientData::kMaxStops },
        {"point0",   SkSLType::kFloat2 },
        {"point1",   SkSLType::kFloat2 },
        {"radius0",  SkSLType::kFloat },
        {"radius1",  SkSLType::kFloat },
};

static constexpr int kNumSolidUniforms = 1;
static constexpr SkUniform kSolidUniforms[kNumSolidUniforms] {
        {"color",  SkSLType::kFloat4 }
};

static const char* kGradientSkSL =
        // TODO: This should use local coords
        "float2 pos = sk_FragCoord.xy;\n"
        "float2 delta = point1 - point0;\n"
        "float2 pt = pos - point0;\n"
        "float t = dot(pt, delta) / dot(delta, delta);\n"
        "float4 result = colors[0];\n"
        "result = mix(result, colors[1],\n"
        "             clamp((t-offsets[0])/(offsets[1]-offsets[0]),\n"
        "                   0, 1));\n"
        "result = mix(result, colors[2],\n"
        "             clamp((t-offsets[1])/(offsets[2]-offsets[1]),\n"
        "                   0, 1));\n"
        "result = mix(result, colors[3],\n"
        "             clamp((t-offsets[2])/(offsets[3]-offsets[2]),\n"
        "             0, 1));\n"
        "outColor = half4(result);\n";

static const char* kSolidColorSkSL = "    outColor = half4(color);\n";

// TODO: kNone is for depth-only draws, so should actually have a fragment output type
// that only defines a [[depth]] attribute but no color calculation.
static const char* kNoneSkSL = "outColor = half4(0.0, 0.0, 1.0, 1.0);\n";

} // anonymous namespace

std::tuple<SkUniquePaintParamsID, std::unique_ptr<SkUniformBlock>> ExtractPaintData(
        Context* context, const PaintParams& p) {
    SkPaintParamsKey key;

    auto dict = context->priv().shaderCodeDictionary();

    std::unique_ptr<SkUniformBlock> block = std::make_unique<SkUniformBlock>();

    // TODO: add UniformData generation to PaintParams::toKey and use it here
    if (auto s = p.shader()) {
        SkColor colors[GradientShaderBlocks::GradientData::kMaxStops];
        float offsets[GradientShaderBlocks::GradientData::kMaxStops];
        SkShader::GradientInfo gradInfo;

        gradInfo.fColorCount = GradientShaderBlocks::GradientData::kMaxStops;
        gradInfo.fColors = colors;
        gradInfo.fColorOffsets = offsets;

        SkShader::GradientType type = s->asAGradient(&gradInfo);
        if (gradInfo.fColorCount > GradientShaderBlocks::GradientData::kMaxStops) {
            type = SkShader::GradientType::kNone_GradientType;
        }

        GradientShaderBlocks::GradientData data(type, gradInfo.fPoint, gradInfo.fRadius,
                                                gradInfo.fTileMode, gradInfo.fColorCount,
                                                colors, offsets);

        switch (type) {
            case SkShader::kLinear_GradientType: {
                GradientShaderBlocks::AddToKey(SkBackend::kGraphite,
                                               &key,
                                               block.get(),
                                               data);
            } break;
            case SkShader::kRadial_GradientType: {
                GradientShaderBlocks::AddToKey(SkBackend::kGraphite,
                                               &key,
                                               block.get(),
                                               data);
            } break;
            case SkShader::kSweep_GradientType:
                GradientShaderBlocks::AddToKey(SkBackend::kGraphite,
                                               &key,
                                               block.get(),
                                               data);
                break;
            case SkShader::GradientType::kConical_GradientType:
                GradientShaderBlocks::AddToKey(SkBackend::kGraphite,
                                               &key,
                                               block.get(),
                                               data);
                break;
            case SkShader::GradientType::kColor_GradientType: [[fallthrough]];
                // TODO: The solid color gradient type should use its color, not
                // the paint color
            case SkShader::GradientType::kNone_GradientType:  [[fallthrough]];
            default:
                SolidColorShaderBlock::AddToKey(SkBackend::kGraphite, &key, block.get(), p.color());
                break;
        }
    } else {
        // Solid colored paint
        SolidColorShaderBlock::AddToKey(SkBackend::kGraphite, &key, block.get(), p.color());
    }

    if (p.blender()) {
        as_BB(p.blender())->addToKey(dict, SkBackend::kGraphite, &key, block.get());
    } else {
        BlendModeBlock::AddToKey(SkBackend::kGraphite, &key, block.get(), SkBlendMode::kSrcOver);
    }

    auto entry = context->priv().shaderCodeDictionary()->findOrCreate(key);

    return { entry->uniqueID(), std::move(block) };
}

SkSpan<const SkUniform> GetUniforms(CodeSnippetID snippetID) {
    switch (snippetID) {
        case CodeSnippetID::kDepthStencilOnlyDraw:
            return {nullptr, 0};
        case CodeSnippetID::kLinearGradientShader:
            return SkMakeSpan(kGradientUniforms, kNumGradientUniforms);
        case CodeSnippetID::kSolidColorShader:
        case CodeSnippetID::kRadialGradientShader:
        case CodeSnippetID::kSweepGradientShader:
        case CodeSnippetID::kConicalGradientShader:
        default:
            return SkMakeSpan(kSolidUniforms, kNumSolidUniforms);
    }
}

const char* GetShaderSkSL(CodeSnippetID snippetID) {
    switch (snippetID) {
        case CodeSnippetID::kDepthStencilOnlyDraw:
            return kNoneSkSL;
        case CodeSnippetID::kLinearGradientShader:
            return kGradientSkSL;
        case CodeSnippetID::kSolidColorShader:
        case CodeSnippetID::kRadialGradientShader:
        case CodeSnippetID::kSweepGradientShader:
        case CodeSnippetID::kConicalGradientShader:
        default:
            return kSolidColorSkSL;
    }
}

} // namespace skgpu
