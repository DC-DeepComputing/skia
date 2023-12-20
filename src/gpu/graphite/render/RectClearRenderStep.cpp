/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/RectClearRenderStep.h"

#include "src/base/SkVx.h"
#include "src/core/SkRRectPriv.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

// This RenderStep is specialized to do rect clear without AA.

namespace skgpu::graphite {

RectClearRenderStep::RectClearRenderStep(StaticBufferManager* bufferManager)
        : RenderStep("RectClearRenderStep",
                     "",
                     Flags::kPerformsShading,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthGreaterPass,
                     /*vertexAttrs=*/{},
                     /*instanceAttrs=*/{
                        {"bounds", VertexAttribType::kFloat4, SkSLType::kFloat4},
                        {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                        {"ssboIndices", VertexAttribType::kUShort2, SkSLType::kUShort2},
                        {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                        {"mat1", VertexAttribType::kFloat3, SkSLType::kFloat3},
                        {"mat2", VertexAttribType::kFloat3, SkSLType::kFloat3}
                     },
                     /*varyings=*/{}) {
}

RectClearRenderStep::~RectClearRenderStep() {}

std::string RectClearRenderStep::vertexSkSL() const {
    // Returns the body of a vertex function, which must define a float4 devPosition variable and
    // must write to an already-defined float2 stepLocalCoords variable.
    return "stepLocalCoords = float2(sk_VertexID / 2, sk_VertexID % 2);\n"
        "stepLocalCoords = mix(bounds.LT, bounds.RB, stepLocalCoords);\n"
        "float4 devPosition;\n"
        "devPosition.xyz = float3x3(mat0, mat1, mat2) * stepLocalCoords.xy1;\n"
        "devPosition = float4(devPosition.xy, depth*devPosition.z, devPosition.z);\n";
}

void RectClearRenderStep::writeVertices(DrawWriter* writer,
                                           const DrawParams& params,
                                           skvx::ushort2 ssboIndices) const {
    // Each instance is 4 vertices, forming 2 triangles from a single triangle strip, so no indices
    // are needed. sk_VertexID is used to place vertex positions, so no vertex buffer is needed.
    DrawWriter::Instances instances{*writer, {}, {}, 4};

    skvx::float4 bounds = params.geometry().bounds().ltrb();
    const SkM44* m = &params.transform().matrix();

    // Since the local coords always have Z=0, we can discard the 3rd row and column of the matrix.
    instances.append(1) << bounds << params.order().depthAsFloat() << ssboIndices
                        << m->rc(0,0) << m->rc(1,0) << m->rc(3,0)
                        << m->rc(0,1) << m->rc(1,1) << m->rc(3,1)
                        << m->rc(0,3) << m->rc(1,3) << m->rc(3,3);
}

void RectClearRenderStep::writeUniformsAndTextures(const DrawParams&,
                                                       PipelineDataGatherer*) const {
    // All data is uploaded as instance attributes, so no uniforms are needed.
}

}  // namespace skgpu::graphite
