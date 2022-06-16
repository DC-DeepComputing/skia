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

VerticesRenderStep::VerticesRenderStep(const SkVertices* vertices, PrimitiveType type) :
              RenderStep("VerticesRenderStep",
                         "regular", // NP - Support inverse filling of vertices?
                         Flags::kNone, // NP - maybe kPerformsShading
                         /*uniforms=*/{},
                         type,
                         kRegularCoverPass, // NP - Support inverse filling of vertices?
                         /*vertexAttrs=*/{{"position",
                                           VertexAttribType::kFloat4,
                                           SkSLType::kFloat4}},
                     /*instanceAttrs=*/{}){
}

VerticesRenderStep::~VerticesRenderStep() {}

const char* VerticesRenderStep::vertexSkSL() const {
  return "";
}

void VerticesRenderStep::writeVertices(DrawWriter* writer, const DrawParams& geom) const {

}

void VerticesRenderStep::writeUniforms(const DrawParams&, SkPipelineDataGatherer*) const {
    // Control points are pre-transformed to device space on the CPU, so no uniforms needed.
}

}  // namespace skgpu::graphite
