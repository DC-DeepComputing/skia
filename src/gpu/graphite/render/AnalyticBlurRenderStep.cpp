/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/AnalyticBlurRenderStep.h"

#include "include/core/SkM44.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkTileMode.h"
#include "include/private/base/SkDebug.h"
#include "src/base/SkEnumBitMask.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/geom/AnalyticBlurMask.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

namespace skgpu::graphite {

AnalyticBlurRenderStep::AnalyticBlurRenderStep()
        : RenderStep(RenderStepID::kAnalyticBlur,
                     Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsCoverage |
                     Flags::kAppendVertices,
                     /*uniforms=*/
                     {{"localToDevice", SkSLType::kFloat4x4},
                      {"deviceToScaledShape", SkSLType::kFloat3x3},
                      {"shapeData", SkSLType::kFloat4},
                      {"blurData", SkSLType::kHalf2},
                      {"shapeType", SkSLType::kInt},
                      {"depth", SkSLType::kFloat}},
                     PrimitiveType::kTriangles,
                     kDirectDepthGreaterPass,
                     /*staticAttrs=*/ {},
                     /*appendAttrs=*/
                     {{"position", VertexAttribType::kFloat2, SkSLType::kFloat2},
                      {"ssboIndices", VertexAttribType::kUInt2, SkSLType::kUInt2}},
                     /*varyings=*/
                     // scaledShapeCoords are the fragment coordinates in local shape space, where
                     // the shape has been scaled to device space but not translated or rotated.
                     {{"scaledShapeCoords", SkSLType::kFloat2}}) {}

std::string AnalyticBlurRenderStep::vertexSkSL() const {
    return
        "float4 devPosition = localToDevice * float4(position, depth, 1.0);\n"
        "stepLocalCoords = position;\n"
        "scaledShapeCoords = (deviceToScaledShape * devPosition.xy1).xy;\n";
}

std::string AnalyticBlurRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    return EmitSamplerLayout(bindingReqs, nextBindingIndex) + " sampler2D s;";
}

const char* AnalyticBlurRenderStep::fragmentCoverageSkSL() const {
    return "outputCoverage = blur_coverage_fn(scaledShapeCoords, "
                                             "shapeData, "
                                             "blurData, "
                                             "shapeType, "
                                             "s);";
}

void AnalyticBlurRenderStep::writeVertices(DrawWriter* writer,
                                           const DrawParams& params,
                                           skvx::uint2 ssboIndices) const {
    const Rect& r = params.geometry().analyticBlurMask().drawBounds();
    DrawWriter::Vertices verts{*writer};
    verts.append(6) << skvx::float2(r.left(), r.top()) << ssboIndices
                    << skvx::float2(r.right(), r.top()) << ssboIndices
                    << skvx::float2(r.left(), r.bot()) << ssboIndices
                    << skvx::float2(r.right(), r.top()) << ssboIndices
                    << skvx::float2(r.right(), r.bot()) << ssboIndices
                    << skvx::float2(r.left(), r.bot()) << ssboIndices;
}

void AnalyticBlurRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                      PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    gatherer->write(params.transform().matrix());

    const AnalyticBlurMask& blur = params.geometry().analyticBlurMask();
    gatherer->write(blur.deviceToScaledShape().asM33());
    gatherer->write(blur.shapeData().asSkRect());
    gatherer->writeHalf(blur.blurData());
    gatherer->write(static_cast<int>(blur.shapeType()));
    gatherer->write(params.order().depthAsFloat());

    gatherer->add(blur.refProxy(), {SkFilterMode::kLinear, SkTileMode::kClamp});
}

///////

AnalyticBlur2RenderStep::AnalyticBlur2RenderStep()
        : RenderStep(RenderStepID::kAnalyticBlur2,
                     Flags::kPerformsShading /*| Flags::kHasTextures*/ | Flags::kEmitsCoverage |
                     Flags::kAppendInstances,
                     /*uniforms=*/ {},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthGreaterPass,
                     /*staticAttrs=*/ {},
                     /*appendAttrs=*/
                     {{"localShape", VertexAttribType::kFloat4, SkSLType::kFloat4},
                      {"cornerRadii", VertexAttribType::kFloat4, SkSLType::kFloat4},
                      {"localSigma", VertexAttribType::kFloat2, SkSLType::kFloat2},
                      {"ssboIndices", VertexAttribType::kUInt2, SkSLType::kUInt2},
                      {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                      {"mat1", VertexAttribType::kFloat3, SkSLType::kFloat3},
                      {"mat2", VertexAttribType::kFloat3, SkSLType::kFloat3},
                      {"depth", VertexAttribType::kFloat, SkSLType::kFloat}},
                     /*varyings=*/
                     {{"shapeCoord", SkSLType::kFloat2}, // FIXME use localCoords, or optimize per patch area
                      {"sigma", SkSLType::kFloat2},
                      {"baseShape", SkSLType::kFloat4}, // does not include blurring
                      {"corners", SkSLType::kFloat4}}) {}

std::string AnalyticBlur2RenderStep::vertexSkSL() const {
    return
        "float4 outerBounds = localShape + localSigma.xyxy * float4(-3,-3,3,3);\n"
        "float4 devPosition = cover_bounds_vertex_fn("
                "float2(sk_VertexID / 2, sk_VertexID % 2), "
                "outerBounds, depth, float3x3(mat0, mat1, mat2), "
                "stepLocalCoords);\n"
        "shapeCoord = stepLocalCoords;\n"
        // Pass through to fragment shader
        "sigma = localSigma;\n"
        "baseShape = localShape;\n"
        "corners = cornerRadii;\n";
}

std::string AnalyticBlur2RenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    SkASSERT(false); // shouldn't be called yet
    return EmitSamplerLayout(bindingReqs, nextBindingIndex) + " sampler2D s;";
}

const char* AnalyticBlur2RenderStep::fragmentCoverageSkSL() const {
    return "outputCoverage = blur_coverage2_fn(shapeCoord, "
                                              "baseShape, "
                                              "corners, "
                                              "sigma);";
}

void AnalyticBlur2RenderStep::writeVertices(DrawWriter* writer,
                                           const DrawParams& params,
                                           skvx::uint2 ssboIndices) const {
    const AnalyticBlurMask2& m = params.geometry().analyticBlurMask2();
    const SkM44& t = params.transform().matrix();
    DrawWriter::Instances instances{*writer, {}, {}, 4};
    instances.append(1) << m.fLocalShape.ltrb()
                        << m.fLocalCornerRadii
                        << m.fLocalSigma
                        << ssboIndices
                        << t.rc(0,0) << t.rc(1,0) << t.rc(3,0)
                        << t.rc(0,1) << t.rc(1,1) << t.rc(3,1)
                        << t.rc(0,3) << t.rc(1,3) << t.rc(3,3)
                        << params.order().depthAsFloat();
}

void AnalyticBlur2RenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                      PipelineDataGatherer* gatherer) const {
    // Nothing yet
    // FIXME add texture lookup table
}

}  // namespace skgpu::graphite
