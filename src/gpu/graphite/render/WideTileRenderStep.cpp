/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/WideTileRenderStep.h"

#include "include/core/SkPath.h"
#include "include/private/base/SkDebug.h"
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
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"

#include <algorithm>

namespace skgpu::graphite {

WideTileRenderStep::WideTileRenderStep(Layout layout)
        : RenderStep(layout,
                     RenderStepID::kWideTile,
                     Flags::kAppendVertices | Flags::kPerformsShading,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangles,
                     kDirectDepthLessPass,
                     /*staticAttrs=*/ {},
                     /*appendAttrs=*/
                     {{"position", VertexAttribType::kFloat2, SkSLType::kFloat2},
                      {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                      {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt}}) {}

WideTileRenderStep::~WideTileRenderStep() {}

std::string WideTileRenderStep::vertexSkSL() const {
    return "float4 devPosition = float4(position, depth, 1.0);\n"
           "stepLocalCoords = position;\n";
}

void WideTileRenderStep::writeVertices(DrawWriter* writer,
                                       const DrawParams& params,
                                       uint32_t ssboIndex) const {
    // 1. Extract Bounds
    Rect localBounds = params.geometry().wideTile().bounds();

    float l = localBounds.left();
    float t = localBounds.top();
    float r = localBounds.right();
    float b = localBounds.bot();

    DrawWriter::Vertices verts{*writer};
    verts.reserve(6);

    SkPoint p0 = {l, t};
    SkPoint p1 = {l, b};
    SkPoint p2 = {r, t};
    SkPoint p3 = {r, b};

    float depth = params.order().depthAsFloat();

    // Triangle 1: TL, BL, TR (CCW)
    verts.append(3)
        << p0.fX << p0.fY << depth << ssboIndex
        << p1.fX << p1.fY << depth << ssboIndex
        << p2.fX << p2.fY << depth << ssboIndex;

    // Triangle 2: BL, BR, TR (CCW)
    verts.append(3)
        << p1.fX << p1.fY << depth << ssboIndex
        << p3.fX << p3.fY << depth << ssboIndex
        << p2.fX << p2.fY << depth << ssboIndex;
}

void WideTileRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                  PipelineDataGatherer* gatherer) const {
}

}  // namespace skgpu::graphite
