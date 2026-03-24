/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/SDFTextLCDRenderStep.h"

#include "include/core/SkColor.h"
#include "include/core/SkM44.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTileMode.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "src/base/SkEnumBitMask.h"
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
#include "src/text/gpu/DistanceFieldAdjustTable.h"
#include "src/text/gpu/SubRunContainer.h"
#include "src/text/gpu/VertexFiller.h"

namespace skgpu::graphite {

SDFTextLCDRenderStep::SDFTextLCDRenderStep(Layout layout)
        : RenderStep(layout,
                     RenderStepID::kSDFTextLCD,
                     Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsCoverage |
                     Flags::kLCDCoverage | Flags::kAppendInstances,
                     /*uniforms=*/{{"subRunDeviceMatrix", SkSLType::kFloat4x4},
                                   {"deviceToLocal", SkSLType::kFloat4x4},
                                   {"atlasSizeInv", SkSLType::kFloat2},
                                   {"pixelGeometryDelta", SkSLType::kHalf2},
                                   {"gammaParams", SkSLType::kHalf4}},
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
                     {{{"unormTexCoords", SkSLType::kFloat2},
                      {"textureCoords", SkSLType::kFloat2}}}) {}

SDFTextLCDRenderStep::~SDFTextLCDRenderStep() {}

std::string SDFTextLCDRenderStep::vertexSkSL() const {
    // Returns the body of a vertex function, which must define a float4 devPosition variable and
    // must write to an already-defined float2 stepLocalCoords variable.
    return "float4 devPosition = text_vertex_fn(float2(sk_VertexID >> 1, sk_VertexID & 1), "
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

std::string SDFTextLCDRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    std::string result;

    result += EmitSamplerLayout(bindingReqs, nextBindingIndex);
    SkSL::String::appendf(&result, " sampler2D sdf_atlas;\n");

    return result;
}

const char* SDFTextLCDRenderStep::fragmentCoverageSkSL() const {
    // The returned SkSL must write its coverage into a 'half4 outputCoverage' variable (defined in
    // the calling code) with the actual coverage splatted out into all four channels.

    // TODO: To minimize the number of shaders generated this is the full affine shader.
    // For best performance it may be worth creating the uniform scale shader as well,
    // as that's the most common case.
    // TODO: Need to add 565 support.
    // TODO: Need aliased and possibly sRGB support.
    return "outputCoverage = sdf_text_lcd_coverage_fn(textureCoords, "
                                                     "pixelGeometryDelta, "
                                                     "gammaParams, "
                                                     "unormTexCoords, "
                                                     "sdf_atlas);";
}

void SDFTextLCDRenderStep::writeVertices(DrawWriter* dw,
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

void SDFTextLCDRenderStep::writeUniformsAndTextures(const DrawParams& params,
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

    // compute and write pixelGeometry vector
    SkV2 pixelGeometryDelta = {0, 0};
    if (SkPixelGeometryIsH(subRunData.pixelGeometry())) {
        pixelGeometryDelta = {1.f/(3*proxy->dimensions().width()), 0};
    } else if (SkPixelGeometryIsV(subRunData.pixelGeometry())) {
        pixelGeometryDelta = {0, 1.f/(3*proxy->dimensions().height())};
    }
    if (SkPixelGeometryIsBGR(subRunData.pixelGeometry())) {
        pixelGeometryDelta = -pixelGeometryDelta;
    }
    gatherer->writeHalf(pixelGeometryDelta);

    // compute and write gamma adjustment
    auto dfAdjustTable = sktext::gpu::DistanceFieldAdjustTable::Get();
    float redCorrection = dfAdjustTable->getAdjustment(SkColorGetR(subRunData.luminanceColor()),
                                                       subRunData.useGammaCorrectDistanceTable());
    float greenCorrection = dfAdjustTable->getAdjustment(SkColorGetG(subRunData.luminanceColor()),
                                                         subRunData.useGammaCorrectDistanceTable());
    float blueCorrection = dfAdjustTable->getAdjustment(SkColorGetB(subRunData.luminanceColor()),
                                                        subRunData.useGammaCorrectDistanceTable());
    SkV4 gammaParams = {redCorrection, greenCorrection, blueCorrection,
                        subRunData.useGammaCorrectDistanceTable() ? 1.f : 0.f};
    gatherer->writeHalf(gammaParams);

    // write texture and sampler
    gatherer->add(proxy, {SkFilterMode::kLinear, SkTileMode::kClamp});
}

}  // namespace skgpu::graphite
