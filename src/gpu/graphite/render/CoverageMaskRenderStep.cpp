/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/render/CoverageMaskRenderStep.h"

#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PathAtlas.h"
#include "src/gpu/graphite/geom/CoverageMaskShape.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

namespace skgpu::graphite {

CoverageMaskRenderStep::CoverageMaskRenderStep()
        : RenderStep("CoverageMaskRenderStep",
                     "",
                     Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsCoverage,
                     /*uniforms=*/{{"maskToDeviceRemainder", SkSLType::kFloat3x3},
                                   {"atlasSizeInv", SkSLType::kFloat2},
                                   {"isInverted", SkSLType::kInt}},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthGEqualPass,
                     /*vertexAttrs=*/{},
                     /*instanceAttrs=*/
                     {{"drawBounds"  , VertexAttribType::kFloat4 , SkSLType::kFloat4},  // ltrb
                      {"deviceOrigin", VertexAttribType::kFloat2, SkSLType::kFloat2},
                      {"uvOrigin"    , VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"maskSize"    , VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"depth"     , VertexAttribType::kFloat, SkSLType::kFloat},
                      {"ssboIndex" , VertexAttribType::kInt  , SkSLType::kInt},
                      // deviceToLocal matrix for producing local coords for shader evaluation
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

std::string CoverageMaskRenderStep::vertexSkSL() const {
    // Returns the body of a vertex function, which must define a float4 devPosition variable and
    // must write to an already-defined float2 stepLocalCoords variable.
    return "float4 devPosition = coverage_mask_vertex_fn("
                    "float2(sk_VertexID >> 1, sk_VertexID & 1), atlasSizeInv, "
                    "drawBounds, float2(deviceOrigin), maskToDeviceRemainder, float2(uvOrigin), "
                    "float2(maskSize), depth, float3x3(mat0, mat1, mat2), "
                    "maskBounds, textureCoords, stepLocalCoords);\n";
}

std::string CoverageMaskRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    return EmitSamplerLayout(bindingReqs, nextBindingIndex) + " sampler2D pathAtlas;";
}

const char* CoverageMaskRenderStep::fragmentCoverageSkSL() const {
    return R"(
        half c = sample(pathAtlas, clamp(textureCoords, maskBounds.LT, maskBounds.RB)).r;
        outputCoverage = half4(isInverted == 1 ? (1 - c) : c);
    )";
}

float CoverageMaskRenderStep::boundsOutset(const Transform& localToDevice, const Rect&) const {
    // Always incorporate a 1-pixel wide border to the (device space) mask for AA. CoverageMasks are
    // expected to be in device space but only after the clip stack has been applied to the
    // CoverageMask's originating geometry. Hence `localToDevice` is not guaranteed to be identity
    // `boundsOutset` needs to return a local coordinate outset for the shape which will be applied
    // to its local-coordinate bounds before it gets transformed to device space.
    //
    // TODO(b/238770428): This won't produce an accurate result if the transform has perspective as
    // the scale is not uniform across the shape. Reconsider what to do here when this RenderStep
    // supports perspective.
    return 1.0 / localToDevice.maxScaleFactor();
}

void CoverageMaskRenderStep::writeVertices(DrawWriter* dw,
                                           const DrawParams& params,
                                           int ssboIndex) const {
    const CoverageMaskShape& coverageMask = params.geometry().coverageMaskShape();

    // A quad is a 4-vertex instance. The coordinates are derived from the vertex IDs.
    DrawWriter::Instances instances(*dw, {}, {}, 4);

    // The device origin is the integer translation extracted from the mask-to-device matrix so
    // that the remaining matrix uniform has less variance between draws.
    const auto& maskToDevice = params.transform().matrix();
    skvx::float2 deviceOrigin = {sk_float_floor(maskToDevice.rc(0, 3)),
                                 sk_float_floor(maskToDevice.rc(1, 3))};

    // relative to mask space (device origin and mask-to-device remainder must be applied in shader)
    skvx::float4 drawBounds;
    skvx::half2 maskSize, uvOrigin;
    if (params.clip().transformedShapeBounds().isEmptyNegativeOrNaN()) {
        // If the mask shape is clipped out then this must be an inverse fill. There is no mask to
        // sample but we still need to paint the fill region that excludes the mask shape. Signal
        // this by setting the mask size to 0.
        SkASSERT(coverageMask.inverted());
        maskSize = uvOrigin = 0;
    } else {
        // `maskSize` correspond to the mask subset to access for the draw. It's assumed that the
        // texture origin has already been adjusted to take into account any padding between atlas
        // entries.
        maskSize = coverageMask.maskSize();
        uvOrigin = coverageMask.textureOrigin();
    }

    if (coverageMask.inverted()) {
        // Only mask filters trigger complex transforms, and they are never inverse filled. Since
        // we know this is an inverted mask, then we can exactly map the draw's clip bounds to mask
        // space so that the clip is fully covered.
        SkASSERT(maskToDevice == SkM44::Translate(deviceOrigin.x(), deviceOrigin.y()));
        drawBounds = params.clip().drawBounds().makeOffset(-deviceOrigin).ltrb();
    } else {
        // If we aren't inverted, then the mask bounds define the draw bounds, and they should not
        // be empty (or the draw should have been discarded earlier).
        SkASSERT(maskSize.x() > 0 && maskSize.y() > 0);
        drawBounds = coverageMask.bounds().ltrb();
    }

    const SkM44& m = coverageMask.deviceToLocal();
    instances.append(1) << drawBounds << deviceOrigin << uvOrigin << maskSize
                        << params.order().depthAsFloat() << ssboIndex
                        << m.rc(0,0) << m.rc(1,0) << m.rc(3,0)   // mat0
                        << m.rc(0,1) << m.rc(1,1) << m.rc(3,1)   // mat1
                        << m.rc(0,3) << m.rc(1,3) << m.rc(3,3);  // mat2
}

void CoverageMaskRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                      PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    const CoverageMaskShape& coverageMask = params.geometry().coverageMaskShape();
    const TextureProxy* proxy = coverageMask.textureProxy();
    SkASSERT(proxy);

    // write uniforms

    // Most coverage masks are aligned with the device pixels, so the params' transform is an
    // integer translation matrix. This translation is extracted as an instance attribute so that
    // the remaining transform has a much lower frequency of changing (only complex-transformed
    // mask filters).
    SkMatrix maskToDevice = params.transform().matrix().asM33();
    maskToDevice.postTranslate(-sk_float_floor(maskToDevice.getTranslateX()),
                               -sk_float_floor(maskToDevice.getTranslateY()));
    gatherer->write(maskToDevice);

    SkV2 atlasSizeInv = {1.f / proxy->dimensions().width(), 1.f / proxy->dimensions().height()};
    gatherer->write(atlasSizeInv);
    gatherer->write(int(coverageMask.inverted()));

    // write textures and samplers
    const SkSamplingOptions kSamplingOptions(SkFilterMode::kNearest);
    constexpr SkTileMode kTileModes[2] = {SkTileMode::kClamp, SkTileMode::kClamp};
    gatherer->add(kSamplingOptions, kTileModes, sk_ref_sp(proxy));
}

}  // namespace skgpu::graphite
