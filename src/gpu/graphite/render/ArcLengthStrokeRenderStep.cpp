/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/ArcLengthStrokeRenderStep.h"

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
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/StorageContext.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
#include "src/gpu/graphite/render/DynamicInstancesPatchAllocator.h"
#include "src/gpu/tessellate/ArcLengthCalculator.h"
#include "src/gpu/tessellate/FixedCountBufferUtils.h"
#include "src/gpu/tessellate/PatchWriter.h"
#include "src/gpu/tessellate/StrokeIterator.h"
#include "src/gpu/tessellate/Tessellation.h"
#include "src/gpu/tessellate/WangsFormula.h"
#include "src/sksl/SkSLString.h"

namespace skgpu::graphite {

namespace {

using namespace skgpu::tess;

// Always use dynamic stroke params and join control points, track the join control point in
// PatchWriter and replicate line end points (match Ganesh's shader behavior).
//
// No explicit curve type on platforms that support infinity.
// No color or wide color attribs, since it might always be part of the PaintParams
// or we'll add a color-only fast path to RenderStep later.
// The order of the attribute declarations must match the order used by
// PatchWriter::emitPatchAttribs, i.e.:
//     join << fanPoint << stroke << color << depth << curveType << vertexStorageIndex << ssboIndex << arcLength
static constexpr Uniform kSSBOStorageUniforms[] = {
        {"totalLength", SkSLType::kFloat},
        {"padding0", SkSLType::kFloat},
        {"padding1", SkSLType::kFloat},
        {"padding2", SkSLType::kFloat}};

static constexpr Attribute kSSBOPatchAttributes[] = {
        {"p01", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"p23", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"prevPoint", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"stroke", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
        {"vertexStorageIndex", VertexAttribType::kUInt, SkSLType::kUInt},
        {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt},
        {"arcLength", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"arcCorners", VertexAttribType::kFloat4, SkSLType::kFloat4}};

static constexpr Attribute kSSBOPatchAttributesWithCurveType[] = {
        {"p01", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"p23", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"prevPoint", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"stroke", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
        {"curveType", VertexAttribType::kFloat, SkSLType::kFloat},
        {"vertexStorageIndex", VertexAttribType::kUInt, SkSLType::kUInt},
        {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt},
        {"arcLength", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"arcCorners", VertexAttribType::kFloat4, SkSLType::kFloat4}};

static constexpr Attribute kInstancedAttributes[] = {
        {"p01", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"p23", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"prevPoint", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"stroke", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
        {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt},
        {"arcLength", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"arcCorners", VertexAttribType::kFloat4, SkSLType::kFloat4}};

static constexpr Attribute kInstancedAttributesWithCurveType[] = {
        {"p01", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"p23", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"prevPoint", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"stroke", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
        {"curveType", VertexAttribType::kFloat, SkSLType::kFloat},
        {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt},
        {"arcLength", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"arcCorners", VertexAttribType::kFloat4, SkSLType::kFloat4}};

inline SkSpan<const Attribute> get_attributes(bool infinitySupport, bool vertexStorageSupport) {
    if (vertexStorageSupport) {
        return infinitySupport ? SkSpan(kSSBOPatchAttributes)
                               : SkSpan(kSSBOPatchAttributesWithCurveType);
    } else {
        return infinitySupport ? SkSpan(kInstancedAttributes)
                               : SkSpan(kInstancedAttributesWithCurveType);
    }
}

template <typename WriterType, typename InitFunc>
SK_ALWAYS_INLINE float run_writer(DrawWriter* dw,
                                  const BindBufferInfo& nullBinding,
                                  int patchReserveCount,
                                  PatchAttribs attribs,
                                  const DrawParams& params,
                                  const SkPath& path,
                                  InitFunc&& initFunc) {
    WriterType writer{attribs, *dw, nullBinding, nullBinding, patchReserveCount};
    writer.setShaderTransform(wangs_formula::VectorXform{params.transform().matrix()},
                              params.transform().maxScaleFactor());
    const StrokeParams strokeParams{params.strokeStyle().halfWidth(),
                                    params.strokeStyle().joinLimit()};
    writer.updateUniformStrokeParams(strokeParams);
    initFunc(writer);
    SkASSERT(params.isStroke());
    WriteStrokedPathVerbs(writer, path, params.strokeStyle().cap(),
                          ArcLengthAccumulator::JoinRadius(strokeParams));
    return writer.accumulatedLength();
}

static constexpr Varying kVaryings[] = {
        // The arc length interpolated from the vertices, which is only exact away from joins.
        {"vertexArcLength", SkSLType::kFloat},
        {"lateralOffset", SkSLType::kFloat},
        // Used to compute the arc length in and around joins per fragment (see
        // fragmentColorSkSL). The corners are described in tessellate_arclength_stroked_curve(),
        // and are all zeros for joins without a corner.
        {"localPos", SkSLType::kFloat2},
        {"startCornerA", SkSLType::kFloat4, Interpolation::kFlat},
        {"startCornerB", SkSLType::kFloat4, Interpolation::kFlat},
        {"endCornerA", SkSLType::kFloat4, Interpolation::kFlat},
        {"endCornerB", SkSLType::kFloat4, Interpolation::kFlat},
        {"invTotalArcLength", SkSLType::kFloat, Interpolation::kFlat},
};

// Set to true to overlay stripes at regular arc length intervals. They make errors in the
// parameterization (e.g. inside joins, or uneven spacing along curves) much easier to see than
// the smooth ramp alone.
static constexpr bool kDebugArcLengthStripes = false;

}  // namespace

ArcLengthStrokeRenderStep::ArcLengthStrokeRenderStep(Layout layout,
                                                     bool computeSupport,
                                                     bool infinitySupport,
                                                     bool vertexStorageSupport)
        : RenderStep(
                  layout,
                  RenderStepID::kArcLengthStroke,
                  Flags::kRequiresMSAA | Flags::kPerformsShading | Flags::kAppendDynamicInstances |
                  Flags::kEmitsPrimitiveColor |
                  (vertexStorageSupport ? Flags::kVsUsesStorage : Flags::kNone),
                  /*uniforms=*/
                  {{"affineMatrix", SkSLType::kFloat4},
                   {"translate", SkSLType::kFloat2},
                   {"maxScale", SkSLType::kFloat}},
                  PrimitiveType::kTriangleStrip,
                  // Required so that the starting cap doesnt go over the body of the stroke and cause a color cliff
                  kDirectDepthLessPass,
                  /*staticAttrs=*/{},
                  /*appendAttrs=*/get_attributes(infinitySupport, vertexStorageSupport),
                  /*storageUniforms=*/vertexStorageSupport ? SkSpan<const Uniform>(kSSBOStorageUniforms)
                                                           : SkSpan<const Uniform>(),
                  /*varyings=*/kVaryings)
        , fComputeSupport(computeSupport)
        , fInfinitySupport(infinitySupport)
        , fVertexStorageSupport(vertexStorageSupport) {}

ArcLengthStrokeRenderStep::~ArcLengthStrokeRenderStep() {}

std::string ArcLengthStrokeRenderStep::vertexSkSL(const RootNodesInfo&) const {
    // TODO: Assumes vertex ID support for now, max edges must equal
    // skgpu::tess::FixedCountStrokes::kMaxEdges -> (2^14 - 1) -> 16383
    std::string sksl = "float edgeID = float(sk_VertexID >> 1);\n"
                       "if ((sk_VertexID & 1) != 0) {"
                           "edgeID = -edgeID;"
                       "}\n";
    if (fVertexStorageSupport) {
        sksl += "float totalArcLength = readStepStorageData(vertexStorageIndex).totalLength;\n";
    } else {
        sksl += "float totalArcLength = arcLength.w;\n";
    }
    sksl += SkSL::String::printf(
            "float2x2 affine = float2x2(affineMatrix.xy, affineMatrix.zw);\n"
            "float outArcLength;\n"
            "float outLateralOffset;\n"
            "float4 outStartCornerA, outStartCornerB, outEndCornerA, outEndCornerB;\n"
            "float4 devAndLocalCoords = tessellate_arclength_stroked_curve("
                    "edgeID, 16383, affine, translate, maxScale, p01, p23, prevPoint,"
                    "stroke, %s, arcLength, arcCorners, outArcLength, outLateralOffset, "
                    "outStartCornerA, outStartCornerB, outEndCornerA, outEndCornerB);\n"
            "vertexArcLength = outArcLength;\n"
            "lateralOffset = outLateralOffset;\n"
            "localPos = devAndLocalCoords.zw;\n"
            "startCornerA = outStartCornerA;\n"
            "startCornerB = outStartCornerB;\n"
            "endCornerA = outEndCornerA;\n"
            "endCornerB = outEndCornerB;\n"
            "float invTotal = (totalArcLength > 0.0) ? 1.0 / totalArcLength : 0.0;\n"
            "invTotalArcLength = invTotal;\n"
            "float strokeDepth = depth - saturate(outArcLength * invTotal) * (1.0 / 131072.0);\n"
            "float4 devPosition = float4(devAndLocalCoords.xy, strokeDepth, 1.0);\n"
            "stepLocalCoords = devAndLocalCoords.zw;\n",
            fInfinitySupport ? "curve_type_using_inf_support(p23)" : "curveType");
    return sksl;
}

std::string ArcLengthStrokeRenderStep::fragmentColorSkSL(const RootNodesInfo&) const {
    // In and around a join, the arc length depends on the position: it sweeps around the join's
    // inner corner (see skgpu::tess::ArcLengthAccumulator). Every piece of the stroke that covers a
    // fragment there (the curves on either side and the join between them) computes the same value
    // from the fragment's position, so it doesn't matter which one ends up on top.
    //
    // A curve uses its start join's arc length until it meets the join, and its end join's once it
    // meets that one. The per-vertex arc length decides which: unlike a test against the joins'
    // cross-sections, it can't pick up parts of a curve that turn back towards a join. Join
    // vertices have the arc length where the join begins, so the join's own triangles always pick
    // their start join.
    //
    // MSAA evaluates varyings at the pixel center, which can lie slightly outside the triangle, so
    // the arc length is clamped to the path.
    std::string sksl =
            "float pathArcLength = vertexArcLength;\n"
            "if (startCornerB.w > 0.0 && vertexArcLength < startCornerB.z) {\n"
                "pathArcLength = arclength_join_fn(localPos, startCornerA, startCornerB, true);\n"
            "} else if (endCornerB.w > 0.0 && vertexArcLength > endCornerB.z) {\n"
                "pathArcLength = arclength_join_fn(localPos, endCornerA, endCornerB, false);\n"
            "}\n"
            "float t = saturate(pathArcLength * invTotalArcLength);\n"
            "half4 ramp = mix(half4(1.0, 0.0, 0.0, 1.0), half4(0.0, 0.0, 1.0, 1.0), half(t));\n";
    if constexpr (kDebugArcLengthStripes) {
        // Offset by a quarter stripe so the ends of the path (t = 0 and t = 1) aren't stripe edges.
        sksl += "half stripe = half(step(0.5, fract(t * 24.0 + 0.25)));\n"
                "primitiveColor = mix(ramp, half4(1.0), 0.4 * stripe);\n";
    } else {
        sksl += "primitiveColor = ramp;\n";
    }
    return sksl;
}

void ArcLengthStrokeRenderStep::writeVertices(DrawWriter* dw,
                                              StorageContext* storageContext,
                                              const DrawParams& params,
                                              uint32_t ssboIndex) const {
    SkPath path = params.geometry().shape().asPath();  // TODO: Iterate the Shape directly

    int patchReserveCount = FixedCountStrokes::PreallocCount(path.countVerbs());
    static const BindBufferInfo kNullBinding = {};

    // Patch traits for Vertex Storage (SSBO) path:
    static constexpr PatchAttribs kSSBOAttribs =
            PatchAttribs::kJoinControlPoint | PatchAttribs::kStrokeParams |
            PatchAttribs::kPaintDepth | PatchAttribs::kSsboIndex |
            PatchAttribs::kVertexStorageIndex | PatchAttribs::kArcLength;
    static constexpr PatchAttribs kSSBOAttribsWithCurveType =
            kSSBOAttribs | PatchAttribs::kExplicitCurveType;
    using StorageWriter = PatchWriter<DynamicInstancesPatchAllocator<FixedCountStrokes>,
                                      Required<PatchAttribs::kJoinControlPoint>,
                                      Required<PatchAttribs::kStrokeParams>,
                                      Required<PatchAttribs::kPaintDepth>,
                                      Required<PatchAttribs::kSsboIndex>,
                                      Required<PatchAttribs::kVertexStorageIndex>,
                                      Required<PatchAttribs::kArcLength>,
                                      Optional<PatchAttribs::kExplicitCurveType>,
                                      ReplicateLineEndPoints,
                                      TrackJoinControlPoints>;

    static constexpr PatchAttribs kInstancedAttribs =
            PatchAttribs::kJoinControlPoint | PatchAttribs::kStrokeParams |
            PatchAttribs::kPaintDepth | PatchAttribs::kSsboIndex | PatchAttribs::kArcLength;
    static constexpr PatchAttribs kInstancedAttribsWithCurveType =
            kInstancedAttribs | PatchAttribs::kExplicitCurveType;
    using InstancedWriter = PatchWriter<DynamicInstancesPatchAllocator<FixedCountStrokes>,
                                        Required<PatchAttribs::kJoinControlPoint>,
                                        Required<PatchAttribs::kStrokeParams>,
                                        Required<PatchAttribs::kPaintDepth>,
                                        Required<PatchAttribs::kSsboIndex>,
                                        Required<PatchAttribs::kArcLength>,
                                        Optional<PatchAttribs::kExplicitCurveType>,
                                        ReplicateLineEndPoints,
                                        TrackJoinControlPoints>;

    if (fVertexStorageSupport) {
        struct StepStorageData {
            float totalLength;
            float padding0;
            float padding1;
            float padding2;
        };
        auto [dst, totalOffset] = storageContext->allocateVertices(
                1, storageUniformStride(), storageUniformAlignment());
        uint32_t storageIndex = totalOffset / storageUniformStride();

        float totalLength = run_writer<StorageWriter>(
                dw,
                kNullBinding,
                patchReserveCount,
                fInfinitySupport ? kSSBOAttribs : kSSBOAttribsWithCurveType,
                params,
                path,
                [storageIndex, depth = params.order().depthAsFloat(), ssboIndex, &params](
                        StorageWriter& writer) {
                    writer.updateVertexStorageIndexAttrib(storageIndex);
                    writer.updatePaintDepthAttrib(depth);
                    writer.updateSsboIndexAttrib(ssboIndex);
                    writer.updateStrokeParamsAttrib(
                            {params.strokeStyle().halfWidth(), params.strokeStyle().joinLimit()});
                });

        if (dst) {
            *static_cast<StepStorageData*>(dst) = {totalLength, 0.f, 0.f, 0.f};
        }
    } else {
        // As an absolute fallback, a device which has neither compute nor vertex storage support
        // makes two passes through the path, once to find the total arcLength and a second to
        // output the instance data. Alternatively, a pointer to each patch's total length could be
        // stored, but this seems unsafe.
        const StrokeParams strokeParams{params.strokeStyle().halfWidth(),
                                        params.strokeStyle().joinLimit()};
        ArcLengthPrecalculator precalculator(strokeParams);
        WriteStrokedPathVerbs(precalculator, path, params.strokeStyle().cap(),
                              ArcLengthAccumulator::JoinRadius(strokeParams));
        float totalLength = precalculator.totalLength();

        run_writer<InstancedWriter>(
                dw,
                kNullBinding,
                patchReserveCount,
                fInfinitySupport ? kInstancedAttribs : kInstancedAttribsWithCurveType,
                params,
                path,
                [depth = params.order().depthAsFloat(), ssboIndex, totalLength, &params](
                        InstancedWriter& writer) {
                    writer.updatePaintDepthAttrib(depth);
                    writer.updateSsboIndexAttrib(ssboIndex);
                    writer.updateStrokeParamsAttrib(
                            {params.strokeStyle().halfWidth(), params.strokeStyle().joinLimit()});
                    writer.setTotalPathLength(totalLength);
                });
    }
}

void ArcLengthStrokeRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                         PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(gatherer->checkRewind());
    // TODO: Implement perspective
    SkASSERT(params.transform().type() < Transform::Type::kPerspective);

    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

            // affineMatrix = float4 (2x2 of transform), translate = float2, maxScale = float
            // Column-major 2x2 of the transform.
            SkV4 upper = {params.transform().matrix().rc(0, 0),
                          params.transform().matrix().rc(1, 0),
                          params.transform().matrix().rc(0, 1),
                          params.transform().matrix().rc(1, 1)};
    gatherer->write(upper);

    gatherer->write(
            SkPoint{params.transform().matrix().rc(0, 3), params.transform().matrix().rc(1, 3)});

    gatherer->write(params.transform().maxScaleFactor());
}

}  // namespace skgpu::graphite
