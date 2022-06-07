/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/VerticesRenderStep.h"

#include "src/core/SkVerticesPriv.h"
#include "src/gpu/graphite/DrawGeometry.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/render/StencilAndCoverDSS.h"

namespace skgpu::graphite {

// NP TODO - Change primitive type based upon SkVertices::VertexMode

    // RenderStep(std::string_view className,
    //            std::string_view variantName,
    //            SkEnumBitMask<Flags> flags,
    //            std::initializer_list<SkUniform> uniforms,
    //            PrimitiveType primitiveType,
    //            DepthStencilSettings depthStencilSettings,
    //            std::initializer_list<Attribute> vertexAttrs,
    //            std::initializer_list<Attribute> instanceAttrs)

VerticesRenderStep::VerticesRenderStep(const SkVertices& vertices) :
              RenderStep("VerticesRenderStep",
                         "regular", // NP - Support inverse filling of vertices?
                         Flags::kNone, // NP - maybe kPerformsShading
                         /*uniforms=*/{},
                         PrimitiveType::kTriangles,
                         kRegularCoverPass, // NP - Support inverse filling of vertices?
                         /*vertexAttrs=*/{{"position",
                                           VertexAttribType::kFloat4,
                                           SkSLType::kFloat4}},
                     /*instanceAttrs=*/{}){
  // NP TODO - Call RenderStep constructor with primitive type derived from vertices obj.
  // For now, defaulting to PrimitiveType::kTriangles.

  // SkVerticesPriv info(vertices->priv());

  // PrimitiveType primitiveType;
  // switch (info.mode()) {
  // case SkVertices::VertexMode::kTriangles_VertexMode:
  //   primitiveType = PrimitiveType::kTriangles;
  //   break;
  // case SkVertices::VertexMode::kTriangleStrip_VertexMode:
  //   primitiveType = PrimitiveType::kTriangleStrip;
  //   break;
  // case SkVertices::VertexMode::kTriangleFan_VertexMode:
  //   primitiveType = PrimitiveType::kTriangleFan;
  //   break;
  // }
}

VerticesRenderStep::~VerticesRenderStep() {}

const char* VerticesRenderStep::vertexSkSL() const {
    // return "     float4 devPosition = position;\n";
}

void VerticesRenderStep::writeVertices(DrawWriter* writer, const DrawGeometry& geom) const {

}

void CoverBoundsRenderStep::writeUniforms(const DrawGeometry&, SkPipelineDataGatherer*) const {
    // Control points are pre-transformed to device space on the CPU, so no uniforms needed.
}

}  // namespace skgpu::graphite
