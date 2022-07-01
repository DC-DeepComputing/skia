/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/VerticesRenderStep.h"

#include "src/core/SkVertState.h"
#include "src/core/SkVerticesPriv.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/render/StencilAndCoverDSS.h"

namespace skgpu::graphite {

namespace {
static constexpr DepthStencilSettings kDirectShadingPass = {
        /*frontStencil=*/{},
        /*backStencil=*/ {},
        /*refValue=*/    0,
        /*stencilTest=*/ false,
        /*depthCompare=*/CompareOp::kGreater,
        /*depthTest=*/   true,
        /*depthWrite=*/  true
};
}  // namespace

VerticesRenderStep::VerticesRenderStep(PrimitiveType type, std::string_view variantName) :
              RenderStep("VerticesRenderStep",
                         variantName,
                         Flags::kRequiresMSAA | Flags::kPerformsShading,
                         /*uniforms=*/{},
                         type,
                         kDirectShadingPass, //  kRegularCoverPass, ?
                         /*vertexAttrs=*/{{"position",
                                         VertexAttribType::kFloat2, SkSLType::kFloat2}},
                         /*instanceAttrs=*/{}){
}

VerticesRenderStep::~VerticesRenderStep() {}

const char* VerticesRenderStep::vertexSkSL() const {
    // Each vertex will have  a vertex attribute of position and eventually color
    // Dynamically fetch the X and Y values of a vertex depending upon the vertex index

    return "float4 devPosition = float4(position, 0, 1);\n";
    // return "float4 devPosition = float4(position.x, position.y, 0, 1);\n";
}

void VerticesRenderStep::writeVertices(DrawWriter* writer, const DrawParams& drawParams) const {
    // writers automatically handle writing to some buffers and draw call to make
    // Differentiate between different vertices primary types

    SkVerticesPriv info(drawParams.geometry().vertices()->priv());

    const int vertexCount = info.vertexCount();
    const int indexCount = info.indexCount();
    const SkPoint* positions = info.positions();
    const uint16_t* indices = info.indices();
    // const SkColor* colors = info.colors();
    // const SkPoint* texCoords = info.texCoords();

    DrawWriter::Vertices verts{*writer};
    // verts.reserve(vertexCount);
    // verts.reserve(3);
    // verts.append(3) << 0 << 0 << 1.0 << 1.0
    //                 << 100 << 100 << 1.0 << 1.0
    //                 << 250 << 250 << 1.0 << 1.0;


    // for (int i = 0; i < vertexCount; ++i) {
    //     SkPoint pos = fVertices->priv().positions()[i];
    VertState state(vertexCount, indices, indexCount);
    VertState::Proc vertProc = state.chooseProc(info.mode());
    while (vertProc(&state)) {
        // State.f0,f1,f2 correspond to indices of a point
        verts.append(3) << positions[state.f0].x() << positions[state.f0].y()
                        << positions[state.f1].x() << positions[state.f1].y()
                        << positions[state.f2].x() << positions[state.f2].y();
    }

    // Collects new vertex data for a call to CommandBuffer::draw(). Automatically accumulates
    // vertex data into a buffer, issuing draw and bind calls as needed when a new buffer is
    // required, so that it is seamless to the caller. The draws do not use instances or indices.
    //
    // Usage (assuming writer has already had 'newPipelineState()' called with correct strides):
    //    DrawWriter::Vertices verts{writer};
    //    verts.append(n) << x << y << ...;

    // From MeshOp::Mesh::WriteVertices:
    // int vertexCount = fVertices->priv().vertexCount();
    // for (int i = 0; i < vertexCount; ++i) {
    //     SkPoint pos = fVertices->priv().positions()[i];
    // }
}

void VerticesRenderStep::writeUniforms(const DrawParams&, SkPipelineDataGatherer*) const {
    // Control points are pre-transformed to device space on the CPU, so no uniforms needed.
}

}  // namespace skgpu::graphite
