/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/EndCapRenderStep.h"

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

EndCapRenderStep::EndCapRenderStep(Layout layout)
        : RenderStep(layout,
                     RenderStepID::kEndCap,
                     Flags::kAppendVertices | Flags::kEmitsPrimitiveColor | Flags::kPerformsShading,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangles,
                     kDirectDepthLEqualPass,
                     /*staticAttrs=*/ {},
                     /*appendAttrs=*/
                     {{"position", VertexAttribType::kFloat2, SkSLType::kFloat2},
                      {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                      {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt}}) {}

EndCapRenderStep::~EndCapRenderStep() {}

std::string EndCapRenderStep::vertexSkSL() const {
    return "float4 devPosition = float4(position, 1.0, 1.0);\n"
           "stepLocalCoords = position;\n";
}

const char* EndCapRenderStep::fragmentColorSkSL() const {
    return "primitiveColor = half4(0, 0, 1, 1); sk_FragColor = primitiveColor; return;";
}

void EndCapRenderStep::writeVertices(DrawWriter* writer,
                                    const DrawParams& params,
                                    uint32_t ssboIndex) const {
    DrawWriter::Vertices verts{*writer};
    verts.reserve(6);

    float left = 500.0f;
    float right = 1000.0f;
    float top = 0.0f;
    float bottom = 500.0f;
    float depth = 1.0f;

    verts.append(3)
        << left  << top    << depth << ssboIndex
        << left  << bottom << depth << ssboIndex
        << right << top    << depth << ssboIndex;

    verts.append(3)
        << left  << bottom << depth << ssboIndex
        << right << bottom << depth << ssboIndex
        << right << top    << depth << ssboIndex;
}

void EndCapRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                PipelineDataGatherer* gatherer) const {
}

}  // namespace skgpu::graphite
