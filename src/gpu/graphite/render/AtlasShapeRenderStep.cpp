/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/render/AtlasShapeRenderStep.h"

#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PathAtlas.h"
#include "src/gpu/graphite/geom/AtlasShape.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

namespace skgpu::graphite {

AtlasShapeRenderStep::AtlasShapeRenderStep()
        : RenderStep("AtlasShapeRenderStep",
                     "",
                     Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsCoverage,
                     /*uniforms=*/{{"atlasSizeInv", SkSLType::kFloat2}},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthGEqualPass,
                     /*vertexAttrs=*/{},
                     /*instanceAttrs=*/
                     {{"size", VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"uvPos", VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"xyPos", VertexAttribType::kFloat2, SkSLType::kFloat2},
                      {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                      {"ssboIndex", VertexAttribType::kInt, SkSLType::kInt}},
                     /*varyings=*/
                     {{"textureCoords", SkSLType::kFloat2}}) {}

std::string AtlasShapeRenderStep::vertexSkSL() const {
    // An atlas shape is a axis-aligned rectangle tessellated as a triangle strip.
    return R"(
        float2 quadCoords = float2(float(sk_VertexID >> 1), float(sk_VertexID & 1));
        quadCoords.xy *= float2(size);

        // Adjust the vertex position for the 1 pixel outset of the atlas rect.
        stepLocalCoords = quadCoords + float2(xyPos) - float2(1);
        float4 position = float4(stepLocalCoords, 0, 1);

        float2 unormTexCoords = quadCoords + float2(uvPos);
        textureCoords = unormTexCoords * atlasSizeInv;

        float4 devPosition = float4(position.xy, depth, position.w);
    )";
}

std::string AtlasShapeRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    return EmitSamplerLayout(bindingReqs, nextBindingIndex) + " uniform sampler2D pathAtlas;";
}

const char* AtlasShapeRenderStep::fragmentCoverageSkSL() const {
    // TODO(b/283876923): Support inverse fills.
    return R"(
        half4 texColor = sample(pathAtlas, textureCoords);
        outputCoverage = texColor.rrrr;
    )";
}

void AtlasShapeRenderStep::writeVertices(DrawWriter* dw,
                                         const DrawParams& params,
                                         int ssboIndex) const {
    const AtlasShape& atlasShape = params.geometry().atlasShape();
    const Rect& atlasBounds = atlasShape.atlasBounds();
    const Rect& deviceSpaceMaskBounds = atlasShape.maskBounds();

    // A quad is a 4-vertex instance. The coordinates are derived from the vertex IDs.
    // TODO(b/283876964): For inverse fills and clipping, assign xyPos based on the draw bounds. We
    // will also need to still use the top-left position of `deviceSpaceMaskBounds` to track the
    // position of the mask shape relative to the actual draw bounds for inverse fills apply the
    // mask sample correctly.
    DrawWriter::Instances instances(*dw, {}, {}, 4);
    skvx::float2 size = atlasBounds.size();
    instances.append(1) << uint16_t(size.x()) << uint16_t(size.y())                // size
                        << uint16_t(atlasBounds.x()) << uint16_t(atlasBounds.y())  // uvPos
                        << deviceSpaceMaskBounds.topLeft()                         // xyPos
                        << params.order().depthAsFloat() << ssboIndex;
}

void AtlasShapeRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                    PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

            const AtlasShape& atlasShape = params.geometry().atlasShape();
    const TextureProxy* proxy = atlasShape.atlas()->texture();
    SkASSERT(proxy);

    // write uniforms
    SkV2 atlasSizeInv = {1.f / proxy->dimensions().width(), 1.f / proxy->dimensions().height()};
    gatherer->write(atlasSizeInv);

    // write textures and samplers
    const SkSamplingOptions kSamplingOptions(SkFilterMode::kNearest);
    constexpr SkTileMode kTileModes[2] = {SkTileMode::kClamp, SkTileMode::kClamp};
    gatherer->add(kSamplingOptions, kTileModes, sk_ref_sp(proxy));
}

}  // namespace skgpu::graphite
