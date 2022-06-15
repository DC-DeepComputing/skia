/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/VerticesRenderStep.h"

#include "src/core/SkVerticesPriv.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/render/StencilAndCoverDSS.h"

namespace skgpu::graphite {

/**
 * Get per-vertex color (arbitrarily set to, say, pink?)
 */

VerticesRenderStep::VerticesRenderStep(const SkVertices* vertices, PrimitiveType type) :
              RenderStep("VerticesRenderStep",
                         "",
                         Flags::kPerformsShading, // NP - correct flag?
                         /*uniforms=*/{},
                         type,
                         kRegularCoverPass,
                         /*vertexAttrs=*/{{"position",
                                           VertexAttribType::kFloat4,
                                           SkSLType::kFloat4}},
                         /*instanceAttrs=*/{}){
}

VerticesRenderStep::~VerticesRenderStep() {}

const char* VerticesRenderStep::vertexSkSL() const {
    // Returns the body of a vertex function, which must define a float4 devPosition variable.
    // It has access to the variables declared by vertexAttributes(), instanceAttributes(),
    // and uniforms(). The 'devPosition' variable's z must store the PaintDepth normalized to a
    // float from [0, 1], for each processed draw although the RenderStep can choose to upload it
    // as attributes or uniforms.
    return "     float4 devPosition = position;\n";
}

void VerticesRenderStep::writeVertices(DrawWriter* writer, const DrawParams& geom) const {
    // BindBufferInfo
    // Write
}

void VerticesRenderStep::writeUniforms(const DrawParams&, SkPipelineDataGatherer*) const {
    // Color, coordinates
}

}  // namespace skgpu::graphite
