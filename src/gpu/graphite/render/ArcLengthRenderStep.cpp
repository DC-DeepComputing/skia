/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/ArcLengthRenderStep.h"

#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStrokeRec.h"
#include "include/private/SkAssert.h"
#include "include/private/SkDebug.h"
#include "include/private/SkEnumBitMask.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkSLTypeShared.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
#include "src/gpu/graphite/render/DynamicInstancesPatchAllocator.h"
#include "src/gpu/tessellate/FixedCountBufferUtils.h"
#include "src/gpu/tessellate/PatchWriter.h"
#include "src/gpu/tessellate/StrokeIterator.h"
#include "src/gpu/tessellate/Tessellation.h"
#include "src/gpu/tessellate/WangsFormula.h"
#include "src/sksl/SkSLString.h"

namespace skgpu::graphite {

namespace {

using namespace skgpu::tess;

static constexpr PatchAttribs kAttribs = PatchAttribs::kJoinControlPoint |
                                         PatchAttribs::kStrokeParams |
                                         PatchAttribs::kPaintDepth |
                                         PatchAttribs::kArcLength |
                                         PatchAttribs::kSsboIndex;
static constexpr PatchAttribs kAttribsWithCurveType = kAttribs | PatchAttribs::kExplicitCurveType;
using Writer = PatchWriter<DynamicInstancesPatchAllocator<FixedCountStrokes>,
                           Required<PatchAttribs::kJoinControlPoint>,
                           Required<PatchAttribs::kStrokeParams>,
                           Required<PatchAttribs::kPaintDepth>,
                           Required<PatchAttribs::kArcLength>,
                           Required<PatchAttribs::kSsboIndex>,
                           Optional<PatchAttribs::kExplicitCurveType>,
                           ReplicateLineEndPoints,
                           TrackJoinControlPoints>;

static constexpr Attribute kBaseAttributes[] = {
        {"p01", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"p23", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"prevPoint", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"stroke", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
        {"lengthT", VertexAttribType::kFloat, SkSLType::kFloat},
        {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt}};

static constexpr Attribute kAttributesWithCurveType[] = {
        {"p01", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"p23", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"prevPoint", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"stroke", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
        {"curveType", VertexAttribType::kFloat, SkSLType::kFloat},
        {"lengthT", VertexAttribType::kFloat, SkSLType::kFloat},
        {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt}};

static constexpr SkSpan<const Attribute> kAttributes[2] = {kAttributesWithCurveType,
                                                           kBaseAttributes};

static constexpr Varying kVaryings[] = {
        {"normal", SkSLType::kFloat2}
};

}  // namespace

ArcLengthRenderStep::ArcLengthRenderStep(Layout layout, bool infinitySupport)
        : RenderStep(layout,
                     RenderStepID::kArcLength,
                     Flags::kRequiresMSAA | Flags::kPerformsShading |
                     Flags::kAppendDynamicInstances | Flags::kAppendsVertexStorage,
                     /*uniforms=*/{{"affineMatrix", SkSLType::kFloat4},
                                   {"translate", SkSLType::kFloat2},
                                   {"maxScale", SkSLType::kFloat}},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthLessPass,
                     /*staticAttrs=*/ {},
                     /*appendAttrs=*/kAttributes[infinitySupport],
                     /*varyings=*/kVaryings)
        , fInfinitySupport(infinitySupport) {}

ArcLengthRenderStep::~ArcLengthRenderStep() {}

std::string ArcLengthRenderStep::vertexSkSL() const {
    // TODO: Assumes vertex ID support for now, max edges must equal
    // skgpu::tess::FixedCountStrokes::kMaxEdges -> (2^14 - 1) -> 16383
    return SkSL::String::printf(
            "float edgeID = float(sk_VertexID >> 1);\n"
            "if ((sk_VertexID & 1) != 0) {"
                "edgeID = -edgeID;"
            "}\n"
            "float2x2 affine = float2x2(affineMatrix.xy, affineMatrix.zw);\n"
            "float2 normalVal;\n"
            "float4 devAndLocalCoords = tessellate_arclength_stroked_curve("
                    "edgeID, 16383, affine, translate, maxScale, p01, p23, prevPoint,"
                    "stroke, %s, lengthT, normalVal);\n"
            "float4 devPosition = float4(devAndLocalCoords.xy, depth, 1.0);\n"
            "stepLocalCoords = devAndLocalCoords.zw;\n"
            "normal = normalVal;\n",
            fInfinitySupport ? "curve_type_using_inf_support(p23)" : "curveType");
}

void ArcLengthRenderStep::writeVertices(DrawWriter* dw,
                                        const DrawParams& params,
                                        uint32_t ssboIndex) const {
    SkPath path = params.geometry().shape().asPath(); // TODO: Iterate the Shape directly

    int patchReserveCount = FixedCountStrokes::PreallocCount(path.countVerbs());
    static const BindBufferInfo kNullBinding = {};
    Writer writer{fInfinitySupport ? kAttribs : kAttribsWithCurveType,
                  *dw,
                  kNullBinding,
                  kNullBinding,
                  patchReserveCount};
    writer.updatePaintDepthAttrib(params.order().depthAsFloat());
    writer.updateArcLengthAttrib(0.f); // Upload empty t value (0.f) for now
    writer.updateSsboIndexAttrib(ssboIndex);

    writer.setShaderTransform(wangs_formula::VectorXform{params.transform().matrix()},
                              params.transform().maxScaleFactor());

    SkASSERT(params.isStroke());
    writer.updateStrokeParamsAttrib({params.strokeStyle().halfWidth(),
                                     params.strokeStyle().joinLimit()});

    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        switch (verb) {
            case SkPathVerb::kMove:
                writer.writeDeferredStrokePatch(pts[0], params.strokeStyle().cap());
                break;

            case SkPathVerb::kClose:
                writer.closeDeferredStrokePatch(params.strokeStyle().cap());
                break;

            case SkPathVerb::kLine:
                writer.writeLine(pts[0], pts[1]);
                break;

            case SkPathVerb::kQuad:
                if (ConicHasCusp(pts)) {
                    SkPoint cusp = SkEvalQuadAt(pts, SkFindQuadMidTangent(pts));
                    writer.writeCircle(cusp);
                    writer.writeLine(pts[0], cusp);
                    writer.writeLine(cusp, pts[2]);
                } else {
                    writer.writeQuadratic(pts);
                }
                break;

            case SkPathVerb::kConic:
                if (ConicHasCusp(pts)) {
                    SkConic conic(pts, *w);
                    SkPoint cusp = conic.evalAt(conic.findMidTangent());
                    writer.writeCircle(cusp);
                    writer.writeLine(pts[0], cusp);
                    writer.writeLine(cusp, pts[2]);
                } else {
                    writer.writeConic(pts, *w);
                }
                break;

            case SkPathVerb::kCubic: {
                SkPoint chops[10];
                float T[2];
                bool areCusps;
                int numChops = FindCubicConvex180Chops(pts, T, &areCusps);
                if (numChops == 0) {
                    writer.writeCubic(pts);
                } else if (numChops == 1) {
                    SkChopCubicAt(pts, chops, T[0]);
                    if (areCusps) {
                        writer.writeCircle(chops[3]);
                        chops[2] = chops[4] = chops[3];
                    }
                    writer.writeCubic(chops);
                    writer.writeCubic(chops + 3);
                } else {
                    SkASSERT(numChops == 2);
                    SkChopCubicAt(pts, chops, T[0], T[1]);
                    if (areCusps) {
                        writer.writeCircle(chops[3]);
                        writer.writeCircle(chops[6]);
                        writer.writeLine(chops[0], chops[3]);
                        writer.writeLine(chops[3], chops[6]);
                        writer.writeLine(chops[6], chops[9]);
                    } else {
                        writer.writeCubic(chops);
                        writer.writeCubic(chops + 3);
                        writer.writeCubic(chops + 6);
                    }
                }
                break;
            }
        }
    }

    writer.writeDeferredStrokePatch({0.f, 0.f}, params.strokeStyle().cap());
}

void ArcLengthRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                   PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(gatherer->checkRewind());
    SkASSERT(params.transform().type() < Transform::Type::kPerspective);

    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    SkV4 upper = {params.transform().matrix().rc(0, 0), params.transform().matrix().rc(1, 0),
                  params.transform().matrix().rc(0, 1), params.transform().matrix().rc(1, 1)};
    gatherer->write(upper);

    gatherer->write(SkPoint{params.transform().matrix().rc(0, 3),
                            params.transform().matrix().rc(1, 3)});

    gatherer->write(params.transform().maxScaleFactor());
}

}  // namespace skgpu::graphite
