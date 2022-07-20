/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/CoverBoundsRenderStep.h"

#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/render/StencilAndCoverDSS.h"

namespace skgpu::graphite {

CoverBoundsRenderStep::CoverBoundsRenderStep(bool inverseFill)
        : RenderStep("CoverBoundsRenderStep",
                     inverseFill ? "inverse" : "regular",
                     Flags::kPerformsShading,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangles,
                     inverseFill ? kInverseCoverPass : kRegularCoverPass,
                     /*vertexAttrs=*/{{"position",
                                       VertexAttribType::kFloat4,
                                       SkSLType::kFloat4},
                                      {"sk_ssbo_binding1_index", VertexAttribType::kInt, SkSLType::kInt},
                                      {"sk_ssbo_binding2_index_fs", VertexAttribType::kInt, SkSLType::kInt}},
                     /*instanceAttrs=*/{},
                     /*varyings=*/     {{"sk_ssbo_binding2_index", SkSLType::kInt}})
        , fInverseFill(inverseFill) {}

CoverBoundsRenderStep::~CoverBoundsRenderStep() {}

const char* CoverBoundsRenderStep::vertexSkSL() const {
    return "     float4 devPosition = position;\n";
}

void CoverBoundsRenderStep::writeVertices(DrawWriter* writer, const DrawParams& params, int geometryStorageBufferIndex, int shadingStorageBufferIndex) const {
    SkV4 devPoints[4]; // ordered TL, TR, BR, BL

    if (fInverseFill) {
        // TODO: When we handle local coords, we'd need to map these corners by the inverse.
        const SkIRect& bounds = params.clip().scissor();
        devPoints[0] = {(float) bounds.fLeft,  (float) bounds.fTop,    0.f, 1.f};
        devPoints[1] = {(float) bounds.fRight, (float) bounds.fTop,    0.f, 1.f};
        devPoints[2] = {(float) bounds.fRight, (float) bounds.fBottom, 0.f, 1.f};
        devPoints[3] = {(float) bounds.fLeft,  (float) bounds.fBottom, 0.f, 1.f};
    } else {
        params.transform().mapPoints(params.geometry().shape().bounds(), devPoints);
    }

    float depth = params.order().depthAsFloat();
    DrawWriter::Vertices verts{*writer};
    // TODO how to decide whether to pass in ssbo index?
    verts.append(6) << devPoints[0].x << devPoints[0].y << depth << devPoints[0].w << geometryStorageBufferIndex << shadingStorageBufferIndex // TL
                    << devPoints[3].x << devPoints[3].y << depth << devPoints[3].w << geometryStorageBufferIndex << shadingStorageBufferIndex // BL
                    << devPoints[1].x << devPoints[1].y << depth << devPoints[1].w << geometryStorageBufferIndex << shadingStorageBufferIndex // TR
                    << devPoints[1].x << devPoints[1].y << depth << devPoints[1].w << geometryStorageBufferIndex << shadingStorageBufferIndex // TR
                    << devPoints[3].x << devPoints[3].y << depth << devPoints[3].w << geometryStorageBufferIndex << shadingStorageBufferIndex // BL
                    << devPoints[2].x << devPoints[2].y << depth << devPoints[2].w << geometryStorageBufferIndex << shadingStorageBufferIndex;// BR
}

void CoverBoundsRenderStep::writeUniforms(const DrawParams&, SkPipelineDataGatherer*) const {
    // Control points are pre-transformed to device space on the CPU, so no uniforms needed.
}

}  // namespace skgpu::graphite
