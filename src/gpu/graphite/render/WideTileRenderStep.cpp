/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/WideTileRenderStep.h"

#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/private/SkDebug.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/sksl/SkSLString.h"

#include <algorithm>
#include <cstdint>

namespace skgpu::graphite {

WideTileRenderStep::WideTileRenderStep(Layout layout)
        : RenderStep(layout,
                     RenderStepID::kWideTile,
                     Flags::kAppendInstances | Flags::kPerformsShading
                                             | Flags::kNoSelfIntersections,
                     // The local coordinates are derived from device positions. The transform is
                     // shared by all of a draw's tiles, so it isn't repeated in every instance.
                     /*uniforms=*/{{"deviceToLocal", SkSLType::kFloat3x3}},
                     PrimitiveType::kTriangleStrip,
                     /*stencilSettings=*/{},
                     /*vertexAttrs=*/{},
                     // The tiles' coordinates are written as the uint16 values that WideTiles
                     // stores; the second component of `width` is unused.
                     /*instanceAttrs=*/
                     {{"xy", VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"width", VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                      {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt}},
                     /*storageUniforms=*/{}) {}

WideTileRenderStep::~WideTileRenderStep() {}

std::string WideTileRenderStep::vertexSkSL(const RootNodesInfo&) const {
    return SkSL::String::printf(
            "float2 position = float2(xy) +"
                              "float2(sk_VertexID < 2 ? 0.0 : float(width.x),"
                                     "(sk_VertexID %% 2) == 0 ? 0.0 : %d.0);"
            "float4 devPosition = float4(position, depth, 1.0);"
            "float3 localCoords = deviceToLocal * float3(position, 1.0);"
            "stepLocalCoords = localCoords.xy / localCoords.z;",
            SparseStripConfig::kTileHeight);
}

void WideTileRenderStep::writeVertices(DrawWriter* writer,
                                       StorageContext* /*storageContext*/,
                                       const DrawParams& params,
                                       uint32_t ssboIndex) const {
    const auto& tiles = params.geometry().wideTiles().tiles();
    if (tiles.empty()) {
        return;
    }

    DrawWriter::Instances instances{*writer, {}, {}, 4};
    instances.reserve(tiles.size());

    const float depth = params.order().depthAsFloat();
    for (const auto& tile : tiles) {
        instances.append(1) << tile.fX << tile.fY << tile.fWidth << uint16_t(0) << depth
                            << ssboIndex;
    }
}

void WideTileRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                  PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(gatherer->checkRewind());
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    SkM44 deviceToLocal;
    SkAssertResult(params.transform().matrix().invert(&deviceToLocal));
    gatherer->write(deviceToLocal.asM33());
}

}  // namespace skgpu::graphite
