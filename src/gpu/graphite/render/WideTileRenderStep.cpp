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
                     Flags::kAppendVertices,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangles,
                     kDirectDepthLEqualPass,
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

    // 2. Transform to Device Space
    const Transform& transform = params.transform();

    // Fix: Use SkV2 for input and SkV4 for output (homogeneous coords)
    SkV2 localPoints[4] = {{l, t}, {l, b}, {r, t}, {r, b}}; // Order: TL, BL, TR, BR
    SkV4 devicePoints[4];
    transform.mapPoints(localPoints, devicePoints, 4);

    // 3. Write Vertices
    DrawWriter::Vertices verts{*writer};
    verts.reserve(6);

    float depth = params.order().depthAsFloat();

    // Helper to project 4D to 2D (handles perspective if w != 1)
    auto getPoint = [&](int i) {
        return devicePoints[i].w == 1.f ? SkPoint{devicePoints[i].x, devicePoints[i].y}
                                        : SkPoint{devicePoints[i].x / devicePoints[i].w,
                                                  devicePoints[i].y / devicePoints[i].w};
    };

    SkPoint p0 = getPoint(0); // TL
    SkPoint p1 = getPoint(1); // BL
    SkPoint p2 = getPoint(2); // TR
    SkPoint p3 = getPoint(3); // BR

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
