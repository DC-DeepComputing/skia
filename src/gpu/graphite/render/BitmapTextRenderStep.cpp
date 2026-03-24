/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/BitmapTextRenderStep.h"

#include "include/core/SkM44.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkTileMode.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/SubRunData.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
#include "src/gpu/graphite/text/GlyphData.h"
#include "src/gpu/graphite/text/TextAtlasManager.h"
#include "src/sksl/SkSLString.h"
#include "src/text/gpu/SubRunContainer.h"
#include "src/text/gpu/VertexFiller.h"

using AtlasSubRun = sktext::gpu::AtlasSubRun;

namespace skgpu::graphite {

namespace {

RenderStep::RenderStepID variant_id(skgpu::MaskFormat variant) {
    switch (variant) {
        case skgpu::MaskFormat::kA8:   return RenderStep::RenderStepID::kBitmapText_Mask;
        case skgpu::MaskFormat::kA565: return RenderStep::RenderStepID::kBitmapText_LCD;
        case skgpu::MaskFormat::kARGB: return RenderStep::RenderStepID::kBitmapText_Color;
    }

    SkUNREACHABLE;
}

}  // namespace

BitmapTextRenderStep::BitmapTextRenderStep(Layout layout, skgpu::MaskFormat variant)
        : RenderStep(layout,
                     variant_id(variant),
                     Flags(variant) | Flags::kAppendInstances,
                     /*uniforms=*/{{"subRunDeviceMatrix", SkSLType::kFloat4x4},
                                   {"deviceToLocal"     , SkSLType::kFloat4x4},
                                   {"atlasSizeInv"      , SkSLType::kFloat2}},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthLEqualPass,
                     /*staticAttrs=*/ {},
                     /*appendAttrs=*/
                     {{{"size", VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"uvPos", VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"xyPos", VertexAttribType::kFloat2, SkSLType::kFloat2},
                      {"flags", VertexAttribType::kUInt, SkSLType::kUInt},
                      {"strikeToSourceScale", VertexAttribType::kFloat, SkSLType::kFloat},
                      {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                      {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt}}},
                     /*varyings=*/
                     {{{"textureCoords", SkSLType::kFloat2},
                      {"maskFormat", SkSLType::kHalf}}}) {}

BitmapTextRenderStep::~BitmapTextRenderStep() {}

SkEnumBitMask<RenderStep::Flags> BitmapTextRenderStep::Flags(skgpu::MaskFormat variant) {
    switch (variant) {
        case skgpu::MaskFormat::kA8:
            return Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsCoverage;
        case skgpu::MaskFormat::kA565:
            return Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsCoverage |
                   Flags::kLCDCoverage;
        case skgpu::MaskFormat::kARGB:
            return Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsPrimitiveColor;
        default:
            SkUNREACHABLE;
    }
}

std::string BitmapTextRenderStep::vertexSkSL() const {
    // Returns the body of a vertex function, which must define a float4 devPosition variable and
    // must write to an already-defined float2 stepLocalCoords variable.
    return "maskFormat = half(flags);"
           "float2 unormTexCoords;"
           "float4 devPosition = text_vertex_fn(float2(sk_VertexID >> 1, sk_VertexID & 1), "
                                               "subRunDeviceMatrix, "
                                               "deviceToLocal, "
                                               "atlasSizeInv, "
                                               "float2(size), "
                                               "float2(uvPos), "
                                               "xyPos, "
                                               "strikeToSourceScale, "
                                               "depth, "
                                               "textureCoords, "
                                               "unormTexCoords, "
                                               "stepLocalCoords);";
}

std::string BitmapTextRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    std::string result;

    result += EmitSamplerLayout(bindingReqs, nextBindingIndex);
    SkSL::String::appendf(&result, " sampler2D text_atlas;\n");

    return result;
}


const char* BitmapTextRenderStep::fragmentColorSkSL() const {
    return "primitiveColor = sample(text_atlas, textureCoords);";
}

const char* BitmapTextRenderStep::fragmentCoverageSkSL() const {
    return "outputCoverage = bitmap_text_coverage_fn(sample(text_atlas, textureCoords), "
                                                    "int(maskFormat));";
}

bool BitmapTextRenderStep::usesUniformsInFragmentSkSL() const { return false; }

void BitmapTextRenderStep::writeVertices(DrawWriter* dw,
                                         const DrawParams& params,
                                         uint32_t ssboIndex) const {
    const SubRunData& subRunData = params.geometry().subRunData();
    auto& glyphData = subRunData.subRun()->glyphVector().accessBackendData<GlyphData>();
    glyphData.fillInstanceData(subRunData.subRun()->vertexFiller(),
                               subRunData.subRun()->glyphVector().accessBackendGlyphs<Glyph>(),
                               dw,
                               subRunData.startGlyphIndex(),
                               subRunData.glyphCount(),
                               subRunData.subRun()->instanceFlags(),
                               ssboIndex,
                               params.order().depthAsFloat());
}

void BitmapTextRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                    PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(gatherer->checkRewind());
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    const SubRunData& subRunData = params.geometry().subRunData();
    Recorder* recorder = subRunData.recorder();
    const sk_sp<TextureProxy>& proxy =
            recorder->priv().atlasProvider()->textAtlasManager()->getProxy(
                    subRunData.subRun()->maskFormat());
    SkASSERT(proxy);

    // write uniforms
    gatherer->write(params.transform().matrix());  // subRunDeviceMatrix
    gatherer->write(subRunData.deviceToLocal());
    SkV2 atlasDimensionsInverse = {1.f/proxy->dimensions().width(),
                                   1.f/proxy->dimensions().height()};
    gatherer->write(atlasDimensionsInverse);

    // write texture and sampler
    gatherer->add(proxy, {SkFilterMode::kNearest, SkTileMode::kClamp});
}

}  // namespace skgpu::graphite
