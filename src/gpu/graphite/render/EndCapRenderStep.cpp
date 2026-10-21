/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/EndCapRenderStep.h"

#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/SkDebug.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/sksl/SkSLString.h"

#include <algorithm>

namespace skgpu::graphite {

EndCapRenderStep::EndCapRenderStep(Layout layout)
        : RenderStep(layout,
                     RenderStepID::kEndCap,
                     Flags::kAppendInstances | Flags::kEmitsCoverage
                                             | Flags::kPerformsShading
                                             | Flags::kHasTextures
                                             | Flags::kNoSelfIntersections,
                     // The local coordinates are derived from device positions. The transform is
                     // shared by all of a draw's caps, so it isn't repeated in every instance.
                     /*uniforms=*/{{"deviceToLocal", SkSLType::kFloat3x3}},
                     PrimitiveType::kTriangleStrip,
                     /*stencilSettings=*/{},
                     /*vertexAttrs=*/{},
                     // The caps' coordinates are written as the uint16 values that EndCaps stores.
                     /*instanceAttrs=*/
                     {{"xy", VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"widthAndPage", VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"mask", VertexAttribType::kUInt, SkSLType::kUInt},
                      {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                      {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt}},
                     /*storageUniforms=*/{},
                     /*varyings=*/
                     {{{"rectCoord", SkSLType::kFloat2},
                       {"maskIndex", SkSLType::kUInt},
                       {"texPageVarying", SkSLType::kInt}}}) {}

EndCapRenderStep::~EndCapRenderStep() {}

std::string EndCapRenderStep::vertexSkSL(const RootNodesInfo&) const {
    return SkSL::String::printf(
            "float2 capOrigin = float2(xy);"
            "float2 capSize = float2(float(widthAndPage.x), %d.0);"
            "float2 rectCoordAttr = float2(sk_VertexID < 2 ? 0.0 : capSize.x,"
                                          "(sk_VertexID %% 2) == 0 ? 0.0 : capSize.y);"
            "float2 position = capOrigin + rectCoordAttr;"
            "float4 devPosition = endcap_vertex_fn(position, rectCoordAttr, depth, mask, "
                                                  "uint(widthAndPage.y), maskIndex, "
                                                  "texPageVarying, rectCoord);"
            "float3 localCoords = deviceToLocal * float3(position, 1.0);"
            "stepLocalCoords = localCoords.xy / localCoords.z;",
            SparseStripConfig::kTileHeight);
}

std::string EndCapRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    std::string result;
    for (int i = 0; i < SparseStripConfig::kNumAlphaBindings; ++i) {
        result += EmitSamplerLayout(bindingReqs, nextBindingIndex);
        SkSL::String::appendf(&result, " sampler2D alpha_text_%d;", i);
    }
    return result;
}

const char* EndCapRenderStep::fragmentCoverageSkSL() const {
    return "outputCoverage = endcap_coverage_fn(rectCoord,"
                                               "alpha_text_0,"
                                               "alpha_text_1,"
                                               "alpha_text_2,"
                                               "maskIndex,"
                                               "texPageVarying);";
}

bool EndCapRenderStep::usesUniformsInFragmentSkSL() const { return false; }

void EndCapRenderStep::writeVertices(DrawWriter* writer,
                                     StorageContext* /*storageContext*/,
                                     const DrawParams& params,
                                     uint32_t ssboIndex) const {
    SkASSERT(params.geometry().isEndCaps());
    const auto& endCaps = params.geometry().endCaps();
    // Overflow caps must have been committed to the atlas' overflow page before being recorded.
    SkASSERT(!endCaps.hasPendingOverflow());

    const auto& caps = endCaps.caps();
    if (caps.empty()) {
        return;
    }

    DrawWriter::Instances instances{*writer, {}, {}, 4};
    instances.reserve(caps.size());

    const float depth = params.order().depthAsFloat();
    for (const auto& cap : caps) {
        instances.append(1) << cap.fX << cap.fY << cap.fWidth << cap.fTexPage << cap.fAlphaIndex
                            << depth << ssboIndex;
    }
}

void EndCapRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(gatherer->checkRewind());
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    SkASSERT(params.geometry().isEndCaps());
    const auto& proxies = params.geometry().endCaps().proxies();

    SkM44 deviceToLocal;
    SkAssertResult(params.transform().matrix().invert(&deviceToLocal));
    gatherer->write(deviceToLocal.asM33());

    for (int i = 0; i < SparseStripConfig::kNumAlphaBindings; ++i) {
        SkASSERT(proxies[i] != nullptr);
        gatherer->add(proxies[i], {SkFilterMode::kNearest, SkTileMode::kClamp});
    }
}

}  // namespace skgpu::graphite
