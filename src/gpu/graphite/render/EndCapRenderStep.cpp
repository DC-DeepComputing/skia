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
#include "src/gpu/graphite/geom/EndCap.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/sksl/SkSLString.h"

namespace skgpu::graphite {

EndCapRenderStep::EndCapRenderStep(Layout layout)
        : RenderStep(layout,
                     RenderStepID::kEndCap,
                     Flags::kAppendVertices | Flags::kEmitsCoverage | Flags::kPerformsShading |
                     Flags::kHasTextures,
                     /*uniforms=*/{{"atlasSize", SkSLType::kFloat2},
                                   {"tileWidthF", SkSLType::kFloat}},
                     PrimitiveType::kTriangles,
                     kDirectDepthLEqualPass,
                     /*staticAttrs=*/ {},
                     /*appendAttrs=*/
                     {{"position",      VertexAttribType::kFloat2, SkSLType::kFloat2},
                      {"rectCoordAttr", VertexAttribType::kFloat2, SkSLType::kFloat2},
                      {"depth",         VertexAttribType::kFloat,  SkSLType::kFloat},
                      {"ssboIndex",     VertexAttribType::kUInt,   SkSLType::kUInt},
                      {"mask",          VertexAttribType::kUInt,   SkSLType::kUInt}},
                     /*varyings=*/
                     {{{"rectCoord", SkSLType::kFloat2},
                       {"maskIndex", SkSLType::kUInt}}}) {}

EndCapRenderStep::~EndCapRenderStep() {}

std::string EndCapRenderStep::vertexSkSL() const {
    return "float4 devPosition = endcap_coverage_fn(position, rectCoordAttr, depth, mask, maskIndex, rectCoord);\n";
}

std::string EndCapRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    std::string result;
    result += EmitSamplerLayout(bindingReqs, nextBindingIndex);
    SkSL::String::appendf(&result, " sampler2D alpha_text_0;");
    return result;
}

const char* EndCapRenderStep::fragmentCoverageSkSL() const {
    return "outputCoverage = endcap_coverage_fn(rectCoord, "
                                               "atlasSize, "
                                               "tileWidthF, "
                                               "alpha_text_0, "
                                               "maskIndex);";
}

void EndCapRenderStep::writeVertices(DrawWriter* writer,
                                     const DrawParams& params,
                                     uint32_t ssboIndex) const {
    SkASSERT(params.geometry().isEndCap());
    const auto& endCap = params.geometry().endCap();

    float l = (float)endCap.x();
    float t = (float)endCap.y();
    float width = (float)endCap.width();
    float height = 4.0f; // Fixed height

    float r = l + width;
    float b = t + height;

    const Transform& transform = params.transform();

    SkV2 localPoints[4] = {{l, t}, {l, b}, {r, t}, {r, b}};
    SkV4 devicePoints[4];
    transform.mapPoints(localPoints, devicePoints, 4);

    DrawWriter::Vertices verts{*writer};
    verts.reserve(6);

    float depth = params.order().depthAsFloat();
    uint32_t maskIdx = endCap.alphaIndex();

    auto getPoint = [&](int i) {
        if (devicePoints[i].w == 1.f) return SkPoint{devicePoints[i].x, devicePoints[i].y};
        float invW = 1.0f / devicePoints[i].w;
        return SkPoint{devicePoints[i].x * invW, devicePoints[i].y * invW};
    };

    SkPoint pTL = getPoint(0);
    SkPoint pBL = getPoint(1);
    SkPoint pTR = getPoint(2);
    SkPoint pBR = getPoint(3);

    // X/Y for device coordinates, then X/Y for local rectCoord (0 to width, 0 to height)
    verts.append(6)
        << pTL.fX << pTL.fY << 0.0f  << 0.0f   << depth << ssboIndex << maskIdx
        << pBL.fX << pBL.fY << 0.0f  << height << depth << ssboIndex << maskIdx
        << pTR.fX << pTR.fY << width << 0.0f   << depth << ssboIndex << maskIdx

        << pTR.fX << pTR.fY << width << 0.0f   << depth << ssboIndex << maskIdx
        << pBL.fX << pBL.fY << 0.0f  << height << depth << ssboIndex << maskIdx
        << pBR.fX << pBR.fY << width << height << depth << ssboIndex << maskIdx;
}

void EndCapRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(gatherer->checkRewind());
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    const EndCap<4,4>& endCap = params.geometry().endCap();
    Recorder* recorder = endCap.recorder();
    const sk_sp<TextureProxy> proxy =
            recorder->priv().atlasProvider()->alphaAtlasManager()->getProxy();

    SkV2 atlasDim = {static_cast<float>(proxy->dimensions().width()),
                    static_cast<float>(proxy->dimensions().height())};
    gatherer->write(atlasDim);
    gatherer->write(static_cast<float>(endCap.width()));
    gatherer->add(proxy, {SkFilterMode::kNearest, SkTileMode::kClamp});
}

}  // namespace skgpu::graphite
