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
                     /*uniforms=*/{{"atlasSizeInv", SkSLType::kFloat2},
                                   {"isInverted", SkSLType::kInt}},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthGEqualPass,
                     /*vertexAttrs=*/{},
                     /*instanceAttrs=*/
                     {{"drawBounds"  , VertexAttribType::kFloat4 , SkSLType::kFloat4},  // ltrb
                      {"deviceOrigin", VertexAttribType::kFloat2 , SkSLType::kFloat2},
                      {"uvOrigin"    , VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"maskSize"    , VertexAttribType::kUShort2, SkSLType::kUShort2}, // width/height
                      {"depth"     , VertexAttribType::kFloat, SkSLType::kFloat},
                      {"ssboIndex" , VertexAttribType::kInt  , SkSLType::kInt},
                      {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                      {"mat1", VertexAttribType::kFloat3, SkSLType::kFloat3},
                      {"mat2", VertexAttribType::kFloat3, SkSLType::kFloat3}},
                     /*varyings=*/
                     {// `maskBounds` are the atlas-relative bounds of the coverage mask.
                      // `textureCoords` are the atlas-relative UV coordinates of the draw, which
                      // can spill beyond `maskBounds` for inverse fills.
                      // TODO: maskBounds is constant for all fragments for a given instance,
                      // could we store them in the draw's SSBO?
                      {"maskBounds"   , SkSLType::kFloat4},
                      {"textureCoords", SkSLType::kFloat2}}) {}

std::string AtlasShapeRenderStep::vertexSkSL() const {
    // An atlas shape is an axis-aligned rectangle tessellated as a triangle strip.
    //
    // The bounds coordinates that we use here have already been transformed to device space and
    // match the desired vertex coordinates of the draw (taking clipping into account), so a
    // localToDevice transform is always the identity matrix.
    //
    // AtlasShape is always defined based on a regular Shape geometry and we can derive the local
    // coordinates from the bounds by simply applying the inverse of the shape's localToDevice
    // transform.
    return R"(
        float3x3 deviceToLocal = float3x3(mat0, mat1, mat2);
        float2 quadCoords = float2(float(sk_VertexID >> 1), float(sk_VertexID & 1));

        // Vertex coordinates.
        float2 maskDims = float2(maskSize);
        float2 drawCoords =
                drawBounds.xy + quadCoords * max(drawBounds.zw - drawBounds.xy, maskDims);

        // Local coordinates used for shading.
        float3 localCoords = deviceToLocal * drawCoords.xy1;
        stepLocalCoords = localCoords.xy / localCoords.z;

        float2 uvPos  = float2(uvOrigin);
        maskBounds    = float4(uvPos, uvPos + maskDims) * atlasSizeInv.xyxy;
        textureCoords = (uvPos + drawCoords - deviceOrigin) * atlasSizeInv;

        float4 devPosition = float4(drawCoords.xy, depth, 1);
    )";
}

std::string AtlasShapeRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    return EmitSamplerLayout(bindingReqs, nextBindingIndex) + " uniform sampler2D pathAtlas;";
}

const char* AtlasShapeRenderStep::fragmentCoverageSkSL() const {
    return R"(
        half c = sample(pathAtlas, clamp(textureCoords, maskBounds.xy, maskBounds.zw)).r;
        if (textureCoords.x < maskBounds.x || textureCoords.y < maskBounds.y ||
            textureCoords.x > maskBounds.z || textureCoords.y > maskBounds.w) {
            c = 0;
        }
        outputCoverage = half4(isInverted == 1 ? (1 - c) : c);
    )";
}

void AtlasShapeRenderStep::writeVertices(DrawWriter* dw,
                                         const DrawParams& params,
                                         int ssboIndex) const {
    const AtlasShape& atlasShape = params.geometry().atlasShape();

    // A quad is a 4-vertex instance. The coordinates are derived from the vertex IDs.
    DrawWriter::Instances instances(*dw, {}, {}, 4);
    skvx::float2 maskSize     = atlasShape.maskSize() + 1;
    skvx::float2 uvOrigin     = atlasShape.atlasOrigin() + 1;
    skvx::float2 deviceOrigin = atlasShape.deviceOrigin();
    const SkM44& m = atlasShape.deviceToLocal();
    instances.append(1) << params.clip().drawBounds().ltrb()                         // drawBounds
                        << deviceOrigin                                              // deviceOrigin
                        << uint16_t(uvOrigin.x())     << uint16_t(uvOrigin.y())      // uvOrigin
                        << uint16_t(maskSize.x())     << uint16_t(maskSize.y())      // maskSize
                        << params.order().depthAsFloat() << ssboIndex
                        << m.rc(0,0) << m.rc(1,0) << m.rc(3,0)   // mat0
                        << m.rc(0,1) << m.rc(1,1) << m.rc(3,1)   // mat1
                        << m.rc(0,3) << m.rc(1,3) << m.rc(3,3);  // mat2
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
    gatherer->write(int(atlasShape.inverted()));

    // write textures and samplers
    const SkSamplingOptions kSamplingOptions(SkFilterMode::kNearest);
    constexpr SkTileMode kTileModes[2] = {SkTileMode::kClamp, SkTileMode::kClamp};
    gatherer->add(kSamplingOptions, kTileModes, sk_ref_sp(proxy));
}

}  // namespace skgpu::graphite
