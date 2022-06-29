/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/TextDirectRenderStep.h"

#include "src/core/SkPipelineData.h"

#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"

using AtlasSubRun = sktext::gpu::AtlasSubRun;

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

// TODO: These attributes encompass only the most basic shaders.
// Do we need wide color? Do we need float3 positions? Do we need to handle float texcoords?
static const std::initializer_list<Attribute> kMaskAttrs =
        {{"position", VertexAttribType::kFloat2, SkSLType::kFloat2},
         {"color", VertexAttribType::kUByte4_norm, SkSLType::kFloat2},
         {"texCoords", VertexAttribType::kUShort2, SkSLType::kUShort2}};
static const std::initializer_list<Attribute> kBitmapAttrs =
        {{"position", VertexAttribType::kFloat2, SkSLType::kFloat2},
         {"texCoords", VertexAttribType::kUShort2, SkSLType::kUShort2}};

TextDirectRenderStep::TextDirectRenderStep(bool isMask)
        : RenderStep("TextDirectRenderStep",
                     isMask ? "mask" : "bitmap",
                     Flags::kPerformsShading,
                     /*uniforms=*/{{"atlasSizeInv", SkSLType::kFloat2}},
                     PrimitiveType::kTriangles,
                     kDirectShadingPass,
                     /*vertexAttrs=*/ isMask ? kMaskAttrs : kBitmapAttrs,
                     /*instanceAttrs=*/{})
        , fIsMask(isMask) {}

TextDirectRenderStep::~TextDirectRenderStep() {}

const char* TextDirectRenderStep::vertexSkSL() const {
    return R"(
        int2 coords = int2(texCoords.x, texCoords.y);
        int texIdx = coords.x >> 13;
        float2 unormTexCoords = float2(coords.x & 0x1FFF, coords.y);

        // TODO: these should be varyings
        float2 textureCoords = unormTexCoords * atlasSizeInv;
        float texIndex = float(texIdx);

        float4 devPosition = float4(position, 0, 1);
        )";
}

void TextDirectRenderStep::writeVertices(DrawWriter* dw, const DrawParams& params) const {
    const AtlasSubRun* subRun = params.geometry().subRunData().subRun();
    const Transform& toDevice = params.transform();

    DrawWriter::Vertices verts{*dw};
    if (fIsMask) {
        verts.append(1) << 10.f << 10.f << 0xffffffff << 0x1f << 0x1f;
    } else {
        verts.append(1) << 10.f << 10.f << (uint16_t)0x1f << (uint16_t)0x1f;
    }
}

void TextDirectRenderStep::writeUniforms(const DrawParams& params,
                                         SkPipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    // TODO: get this from the actual texture size
    skvx::float2 atlasDimensionsInverse = {1.f/1024, 1.f/1024};
    gatherer->write(atlasDimensionsInverse);
}

}  // namespace skgpu::graphite
