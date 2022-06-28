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

VerticesRenderStep::VerticesRenderStep(const SkVertices* vertices, PrimitiveType type) :
              RenderStep("VerticesRenderStep",
                         "",
                         Flags::kPerformsShading,
                         /*uniforms=*/{},
                         type,
                         kRegularCoverPass,
                         /*vertexAttrs=*/{{"position",
                                         VertexAttribType::kFloat2, SkSLType::kFloat2}},
                         /*instanceAttrs=*/{}){
    fVertices = vertices;
}

VerticesRenderStep::~VerticesRenderStep() {}

const char* VerticesRenderStep::vertexSkSL() const {
    // Each vertex will have  a vertex attribute of position and eventually color
    // Dynamically fetch the X and Y values of a vertex depending upon the vertex index
    // TODO: Assumes vertex ID support for now
    return "float2 devPosition = position;\n";
}

void VerticesRenderStep::writeVertices(DrawWriter* writer, const DrawParams& geom) const {
    // writers automatically handle writing to some buffers and draw call to make
    // Differentiate between different vertices primary types

    SkVerticesPriv info(fVertices->priv());

    const int vertexCount = info.vertexCount();
    const int indexCount = info.indexCount();
    const SkPoint* positions = info.positions();
    const SkPoint* texCoords = info.texCoords();
    const uint16_t* indices = info.indices();
    const SkColor* colors = info.colors();

    DrawWriter::Vertices verts{*writer};
    verts.reserve(vertexCount);

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

    // int vertexCount = fVertices->priv().vertexCount();
    // for (int i = 0; i < vertexCount; ++i) {
    //     SkPoint pos = fVertices->priv().positions()[i];
    // }
}

void VerticesRenderStep::writeUniforms(const DrawParams&, SkPipelineDataGatherer*) const {
    // Control points are pre-transformed to device space on the CPU, so no uniforms needed.
}

}  // namespace skgpu::graphite




















    // // TODO: Have Shape provide a path-like iterator so we don't actually have to convert non
    // // paths to SkPath just to iterate their pts/verbs
    // // SkPath path = params.geometry().shape().asPath();
    // SkVerticesPriv info(fVertices->priv());
    // const int vertexCount = info.vertexCount();
    // const int indexCount = info.indexCount();

    // // Abort early if there is nothing to draw
    // if (vertexCount < 3 || (indexCount > 0 && indexCount < 3)) {
    //     return;
    // }


    // // const int maxTrianglesInFans = std::max(path.countVerbs() - 2, 0);

    // // NP - How to define depth for the vertices?
    // // float depth = params.order().depthAsFloat();

    // DrawWriter::Vertices verts{*writer};


    // /*
    // // Collects new vertex data for a call to CommandBuffer::draw(). Automatically accumulates
    // // vertex data into a buffer, issuing draw and bind calls as needed when a new buffer is
    // // required, so that it is seamless to the caller. The draws do not use instances or indices.
    // //
    // // Usage (assuming writer has already had 'newPipelineState()' called with correct strides):
    // //    DrawWriter::Vertices verts{writer};
    // //    verts.append(n) << x << y << ...;
    // //
    // // This should not be used when the vertex stride is 0.
    // class Vertices;
    // */
    // verts.reserve(vertexCount);
    // // NP note - I think this will be handled differently depending upon the type of triangles -
    // // the indexing will be handled differently.
    // // Assume regular triangles, so 3 points for one triangle, 3 more for the next for now.
    // // Use SkPoint or SkV2? Use SkVertices traversal helper - VertState.




    // // for (tess::PathMiddleOutFanIter it(path); !it.done();) {
    // //     for (auto [p0, p1, p2] : it.nextStack()) {
    // //         // TODO: PathMiddleOutFanIter should use SkV2 instead of SkPoint?
    // //         SkV2 p[3] = {{p0.fX, p0.fY}, {p1.fX, p1.fY}, {p2.fX, p2.fY}};
    // //         SkV4 devPoints[3];
    // //         params.transform().mapPoints(p, devPoints, 3);

    // //         verts.append(3) << devPoints[0].x << devPoints[0].y << depth << devPoints[0].w  // p0
    // //                         << devPoints[1].x << devPoints[1].y << depth << devPoints[1].w  // p1
    // //                         << devPoints[2].x << devPoints[2].y << depth << devPoints[2].w; // p2
    // //     }
    // // }
