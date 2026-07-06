/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/TessellateStrokesRenderStep.h"

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

// Always use dynamic stroke params and join control points, track the join control point in
// PatchWriter and replicate line end points (match Ganesh's shader behavior).
//
// No explicit curve type on platforms that support infinity.
// No color or wide color attribs, since it might always be part of the PaintParams
// or we'll add a color-only fast path to RenderStep later.
// The order of the attribute declarations must match the order used by
// PatchWriter::emitPatchAttribs, i.e.:
//     join << fanPoint << stroke << color << depth << curveType << ssboIndex << vertexStorageIndex
static constexpr Attribute kSSBOPatchAttributes[] = {
        {"p01", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"p23", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"prevPoint", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"vertexStorageIndex", VertexAttribType::kUInt, SkSLType::kUInt}};

static constexpr Attribute kSSBOPatchAttributesWithCurveType[] = {
        {"p01", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"p23", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"prevPoint", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"curveType", VertexAttribType::kFloat, SkSLType::kFloat},
        {"vertexStorageIndex", VertexAttribType::kUInt, SkSLType::kUInt}};

static constexpr Attribute kInstancedAttributes[] = {
        {"p01", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"p23", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"prevPoint", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"stroke", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
        {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt}};

static constexpr Attribute kInstancedAttributesWithCurveType[] = {
        {"p01", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"p23", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"prevPoint", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"stroke", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
        {"curveType", VertexAttribType::kFloat, SkSLType::kFloat},
        {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt}};

inline SkSpan<const Attribute> get_attributes(bool infinitySupport, bool vertexStorageSupport) {
    if (vertexStorageSupport) {
        return infinitySupport ? SkSpan(kSSBOPatchAttributes)
                               : SkSpan(kSSBOPatchAttributesWithCurveType);
    } else {
        return infinitySupport ? SkSpan(kInstancedAttributes)
                               : SkSpan(kInstancedAttributesWithCurveType);
    }
}

template <typename WriterType>
SK_ALWAYS_INLINE void write_path_vertices(WriterType& writer,
                                          const SkPath& path,
                                          const StrokeStyle& strokeStyle) {
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        switch (verb) {
            case SkPathVerb::kMove:
                // This automatically joins the last contour with the first contour (deferred) if
                // the contour is closed. If the contour is not closed, it automatically adds
                // additional patches for the end cap of the last patch and the beginning cap of the
                // deferred patch. This does nothing if this is the beginning of the first contour.
                writer.writeDeferredStrokePatch(pts[0], strokeStyle.cap());
                break;

            case SkPathVerb::kClose:
                // Draws a line back to the starting point of the contour and writes any deferred
                // patch with a join (instead of caps). Or if the contour was empty, draws a cap.
                // Since any deferred patch is consumed, the next moveTo's writeDeferredStrokePatch
                // will do nothing but record the beginning of the new contour.
                writer.closeDeferredStrokePatch(strokeStyle.cap());
                break;

            case SkPathVerb::kLine:
                writer.writeLine(pts[0], pts[1]);
                break;

            case SkPathVerb::kQuad:
                if (ConicHasCusp(pts)) {
                    // The cusp is always at the midtangent.
                    SkPoint cusp = SkEvalQuadAt(pts, SkFindQuadMidTangent(pts));
                    writer.writeCircle(cusp);
                    // A quad can only have a cusp if it's flat with a 180-degree turnaround.
                    writer.writeLine(pts[0], cusp);
                    writer.writeLine(cusp, pts[2]);
                } else {
                    writer.writeQuadratic(pts);
                }
                break;

            case SkPathVerb::kConic:
                if (ConicHasCusp(pts)) {
                    // The cusp is always at the midtangent.
                    SkConic conic(pts, *w);
                    SkPoint cusp = conic.evalAt(conic.findMidTangent());
                    writer.writeCircle(cusp);
                    // A conic can only have a cusp if it's flat with a 180-degree turnaround.
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
                        // In a perfect world, these 3 points would be be equal after chopping
                        // on a cusp.
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
                        // Two cusps are only possible if it's a flat line with two 180-degree
                        // turnarounds.
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
    // Finish the last contour (next moveTo point doesn't matter)
    writer.writeDeferredStrokePatch({0.f, 0.f}, strokeStyle.cap());
}

template <typename WriterType, typename InitFunc>
SK_ALWAYS_INLINE void run_writer(DrawWriter* dw,
                                 const BindBufferInfo& nullBinding,
                                 int patchReserveCount,
                                 PatchAttribs attribs,
                                 const DrawParams& params,
                                 const SkPath& path,
                                 InitFunc&& initFunc) {
    WriterType writer{attribs, *dw, nullBinding, nullBinding, patchReserveCount};
    writer.setShaderTransform(wangs_formula::VectorXform{params.transform().matrix()},
                              params.transform().maxScaleFactor());
    writer.updateUniformStrokeParams(
            {params.strokeStyle().halfWidth(), params.strokeStyle().joinLimit()});
    initFunc(writer);
    SkASSERT(params.isStroke());
    write_path_vertices(writer, path, params.strokeStyle());
}

}  // namespace

TessellateStrokesRenderStep::TessellateStrokesRenderStep(Layout layout,
                                                         bool infinitySupport,
                                                         bool vertexStorageSupport)
        : RenderStep(
                  layout,
                  RenderStepID::kTessellateStrokes,
                  Flags::kRequiresMSAA | Flags::kPerformsShading | Flags::kAppendDynamicInstances,
                  /*uniforms=*/
                  {{"affineMatrix", SkSLType::kFloat4},
                   {"translate", SkSLType::kFloat2},
                   {"maxScale", SkSLType::kFloat}},
                  PrimitiveType::kTriangleStrip,
                  kDirectDepthLessPass,
                  /*staticAttrs=*/{},
                  /*appendAttrs=*/get_attributes(infinitySupport, vertexStorageSupport))
        , fInfinitySupport(infinitySupport)
        , fVertexStorageSupport(vertexStorageSupport) {}

TessellateStrokesRenderStep::~TessellateStrokesRenderStep() {}

std::string TessellateStrokesRenderStep::vertexSkSL() const {
    // TODO: Assumes vertex ID support for now, max edges must equal
    // skgpu::tess::FixedCountStrokes::kMaxEdges -> (2^14 - 1) -> 16383
    std::string sksl = "float edgeID = float(sk_VertexID >> 1);\n"
                       "if ((sk_VertexID & 1) != 0) {"
                           "edgeID = -edgeID;"
                       "}\n";
    if (fVertexStorageSupport) {
        sksl += "float2 stroke = strokeData[vertexStorageIndex].stroke;\n"
                "float depth = strokeData[vertexStorageIndex].depth;\n";
    }
    sksl += SkSL::String::printf(
            "float2x2 affine = float2x2(affineMatrix.xy, affineMatrix.zw);\n"
            "float4 devAndLocalCoords = tessellate_stroked_curve("
                    "edgeID, 16383, affine, translate, maxScale, p01, p23, prevPoint,"
                    "stroke, %s);\n"
            "float4 devPosition = float4(devAndLocalCoords.xy, depth, 1.0);\n"
            "stepLocalCoords = devAndLocalCoords.zw;\n",
            fInfinitySupport ? "curve_type_using_inf_support(p23)" : "curveType");
    return sksl;
}

std::string TessellateStrokesRenderStep::vertexStorageSkSL(const ResourceBindingRequirements& reqs,
                                                           int binding) const {
    return SkSL::String::printf(
            "struct StrokeAndDepth {\n"
            "    float2 stroke;\n"
            "    float  depth;\n"
            "    uint   ssboIndex;\n"
            "};\n"
            "layout(set=%d, binding=%d) readonly buffer StrokeStorageBuffer {\n"
            "    StrokeAndDepth strokeData[];\n"
            "};\n",
            reqs.fUniformsSetIdx,
            binding);
}

void TessellateStrokesRenderStep::writeVertices(DrawWriter* dw,
                                                const DrawParams& params,
                                                uint32_t ssboIndex) const {
    SkPath path = params.geometry().shape().asPath();  // TODO: Iterate the Shape directly

    int patchReserveCount = FixedCountStrokes::PreallocCount(path.countVerbs());
    static const BindBufferInfo kNullBinding = {};

    // Patch traits for Vertex Storage (SSBO) path:
    static constexpr PatchAttribs kSSBOAttribs =
            PatchAttribs::kJoinControlPoint | PatchAttribs::kVertexStorageIndex;
    static constexpr PatchAttribs kSSBOAttribsWithCurveType =
            kSSBOAttribs | PatchAttribs::kExplicitCurveType;
    using StorageWriter = PatchWriter<DynamicInstancesPatchAllocator<FixedCountStrokes>,
                                      Required<PatchAttribs::kJoinControlPoint>,
                                      Required<PatchAttribs::kVertexStorageIndex>,
                                      Optional<PatchAttribs::kExplicitCurveType>,
                                      ReplicateLineEndPoints,
                                      TrackJoinControlPoints>;

    static constexpr PatchAttribs kInstancedAttribs =
            PatchAttribs::kJoinControlPoint | PatchAttribs::kStrokeParams |
            PatchAttribs::kPaintDepth | PatchAttribs::kSsboIndex;
    static constexpr PatchAttribs kInstancedAttribsWithCurveType =
            kInstancedAttribs | PatchAttribs::kExplicitCurveType;
    using InstancedWriter = PatchWriter<DynamicInstancesPatchAllocator<FixedCountStrokes>,
                                        Required<PatchAttribs::kJoinControlPoint>,
                                        Required<PatchAttribs::kStrokeParams>,
                                        Required<PatchAttribs::kPaintDepth>,
                                        Required<PatchAttribs::kSsboIndex>,
                                        Optional<PatchAttribs::kExplicitCurveType>,
                                        ReplicateLineEndPoints,
                                        TrackJoinControlPoints>;

    if (fVertexStorageSupport) {
        uint32_t storageIndex = dw->pendingStorageCount();
        auto storageWriter = dw->appendVertexStorage(1);
        storageWriter << SkPoint{params.strokeStyle().halfWidth(), params.strokeStyle().joinLimit()}
                      << params.order().depthAsFloat() << ssboIndex;

        run_writer<StorageWriter>(dw,
                                  kNullBinding,
                                  patchReserveCount,
                                  fInfinitySupport ? kSSBOAttribs : kSSBOAttribsWithCurveType,
                                  params,
                                  path,
                                  [storageIndex](StorageWriter& writer) {
                                      writer.updateVertexStorageIndexAttrib(storageIndex);
                                  });
    } else {
        run_writer<InstancedWriter>(
                dw,
                kNullBinding,
                patchReserveCount,
                fInfinitySupport ? kInstancedAttribs : kInstancedAttribsWithCurveType,
                params,
                path,
                [depth = params.order().depthAsFloat(), ssboIndex, &params](
                        InstancedWriter& writer) {
                    writer.updatePaintDepthAttrib(depth);
                    writer.updateSsboIndexAttrib(ssboIndex);
                    writer.updateStrokeParamsAttrib(
                            {params.strokeStyle().halfWidth(), params.strokeStyle().joinLimit()});
                });
    }
}

void TessellateStrokesRenderStep::writeUniformsAndTextures(const DrawParams& params,
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

size_t TessellateStrokesRenderStep::vertexStorageStride() const {
    struct StrokeAndDepth {
        SkPoint fStroke;
        float fDepth;
        uint32_t fSsboIndex;
    };
    return sizeof(StrokeAndDepth);
}

}  // namespace skgpu::graphite
