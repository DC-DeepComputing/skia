/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/EndCapRenderStep.h"

#include "include/core/SkPath.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkDebug.h"
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
                     Flags::kAppendInstances | Flags::kEmitsCoverage | Flags::kPerformsShading |
                     Flags::kHasTextures,
                     /*uniforms=*/{{"atlasSize", SkSLType::kFloat2}},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthLessPass,
                     /*vertexAttrs=*/ {},
                     /*instanceAttrs=*/
                     {{"tileBounds",  VertexAttribType::kFloat4, SkSLType::kFloat4},
                      {"depth",       VertexAttribType::kFloat,  SkSLType::kFloat},
                      {"ssboIndex",   VertexAttribType::kUInt,   SkSLType::kUInt},
                      {"mask",        VertexAttribType::kUInt,   SkSLType::kUInt}},
                     /*varyings=*/
                     {{{"rectCoord", SkSLType::kFloat2},
                       {"maskIndex", SkSLType::kUInt}}}) {}

EndCapRenderStep::~EndCapRenderStep() {}

std::string EndCapRenderStep::vertexSkSL() const {
    return "float2 position = float2(sk_VertexID < 2 ? tileBounds.x : tileBounds.z,\n"
           "                         (sk_VertexID % 2) == 0 ? tileBounds.y : tileBounds.w);\n"
           "float2 rectCoordAttr = float2(sk_VertexID < 2 ? 0.0 : (tileBounds.z - tileBounds.x),\n"
           "                              (sk_VertexID % 2) == 0 ? 0.0 : (tileBounds.w - tileBounds.y));\n"
           "float4 devPosition = endcap_coverage_fn(position, rectCoordAttr, depth, mask, maskIndex, rectCoord);\n";
}

std::string EndCapRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    std::string result;
    for (int i = 0; i < 4; ++i) {
        result += EmitSamplerLayout(bindingReqs, nextBindingIndex);
        SkSL::String::appendf(&result, " sampler2D alpha_text_%d;", i);
    }
    return result;
}

const char* EndCapRenderStep::fragmentCoverageSkSL() const {
    return "outputCoverage = endcap_coverage_fn(rectCoord,    "
                                               "atlasSize,    "
                                               "alpha_text_0, "
                                               "alpha_text_1, "
                                               "alpha_text_2, "
                                               "alpha_text_3, "
                                               "maskIndex);   ";
}

void EndCapRenderStep::writeVertices(DrawWriter* writer,
                                     const DrawParams& params,
                                     uint32_t ssboIndex) const {
    SkASSERT(params.geometry().isEndCaps());
    const auto& caps = params.geometry().endCaps().caps();
    if (caps.empty()) {
        return;
    }

    DrawWriter::Instances instances{*writer, {}, {}, 4};
    instances.reserve(caps.size());

    float depth = params.order().depthAsFloat();
    float height = static_cast<float>(SparseStripConfig::kTileHeight);

    for (const auto& cap : caps) {
        float l = static_cast<float>(cap.fX);
        float t = static_cast<float>(cap.fY);
        float width = static_cast<float>(cap.fWidth);

        float r = l + width;
        float b = t + height;

        instances.append(1)
            << l << t << r << b
            << depth
            << ssboIndex
            << cap.fAlphaIndex;
    }
}

void EndCapRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(gatherer->checkRewind());
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    SkASSERT(params.geometry().isEndCaps());
    const auto& proxies = params.geometry().endCaps().proxies();

    if (proxies.empty()) {
        return;
    }

    SkV2 atlasDim = {static_cast<float>(proxies[0]->dimensions().width()),
                     static_cast<float>(proxies[0]->dimensions().height())};
    gatherer->write(atlasDim);

    for (int i = 0; i < EndCaps<SparseStripConfig::kTileHeight>::kMaxTextures; ++i) {
        int proxyIdx = std::min(i, proxies.size() - 1);
        gatherer->add(proxies[proxyIdx], {SkFilterMode::kNearest, SkTileMode::kClamp});
    }
}

}  // namespace skgpu::graphite
