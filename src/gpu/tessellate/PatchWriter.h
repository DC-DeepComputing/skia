/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_PatchWriter_DEFINED
#define tessellate_PatchWriter_DEFINED

#include "include/private/SkColorData.h"
#include "src/gpu/GrVertexChunkArray.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/Tessellation.h"

#define AI SK_ALWAYS_INLINE

namespace skgpu {

#if SK_GPU_V1
class PathTessellator;
class StrokeTessellator;
#endif

// Writes out tessellation patches, formatted with their specific attribs, to a GPU buffer.
class PatchWriter {
public:
    PatchWriter(GrMeshDrawTarget* target,
                GrVertexChunkArray* vertexChunkArray,
                PatchAttribs attribs,
                size_t patchStride,
                int initialAllocCount)
            : fAttribs(attribs)
            , fChunker(target, vertexChunkArray, patchStride, initialAllocCount) {}

#if SK_GPU_V1
    // Create PatchWriters that write directly to the GrVertexChunkArrays stored on the provided
    // tessellators.
    PatchWriter(GrMeshDrawTarget*, PathTessellator*, int initialPatchAllocCount);
    PatchWriter(GrMeshDrawTarget*, StrokeTessellator*, int initialPatchAllocCount);
#endif

    ~PatchWriter() {
        // finishStrokeContour() should have been called before this was deleted (or never used).
        SkASSERT(fDeferredCurveType < 0.f && !fHasJoinControlPoint);
    }

    PatchAttribs attribs() const { return fAttribs; }

    // Updates the stroke's join control point that will be written out with each patch. This is
    // automatically adjusted when appending various geometries (e.g. Conic/Cubic), but sometimes
    // must be set explicitly.
    //
    // PatchAttribs::kJoinControlPoint must be enabled.
    void updateJoinControlPointAttrib(SkPoint lastControlPoint) {
        SkASSERT(fAttribs & PatchAttribs::kJoinControlPoint && lastControlPoint.isFinite());
        fJoinControlPointAttrib = lastControlPoint;
        fHasJoinControlPoint = true;
    }
    // Completes a closed contour of a stroke by rewriting a deferred patch with now-available
    // join control point information. Automatically resets the join control point attribute.
    //
    // PatchAttribs::kJoinControlPoint must be enabled.
    void writeDeferredStrokePatch() {
        SkASSERT(fAttribs & PatchAttribs::kJoinControlPoint);
        if (fDeferredCurveType >= 0.f) {
            SkASSERT(fHasJoinControlPoint);
            if (VertexWriter vw = fChunker.appendVertex()) {
                vw << VertexWriter::Array<SkPoint>(fDeferredControlPoints, 4);
                this->emitPatchAttribs(std::move(vw), fJoinControlPointAttrib, fDeferredCurveType);
            }
        }

        fDeferredCurveType = -1.f;
        fHasJoinControlPoint = false;
    }
    // TODO: These are only used by StrokeHardwareTessellator and ideally its patch writing logic
    // should be simplified like StrokeFixedCountTessellator's, and then this can go away.
    void resetJoinControlPointAttrib() {
        SkASSERT(fAttribs & PatchAttribs::kJoinControlPoint);
        // Should have already been written or caller should manually defer
        SkASSERT(fDeferredCurveType < 0.f);
        fHasJoinControlPoint = false;
    }
    SkPoint joinControlPoint() const { return fJoinControlPointAttrib; }
    bool hasJoinControlPoint() const { return fHasJoinControlPoint; }

    // Updates the fan point that will be written out with each patch (i.e., the point that wedges
    // fan around).
    // PatchAttribs::kFanPoint must be enabled.
    void updateFanPointAttrib(SkPoint fanPoint) {
        SkASSERT(fAttribs & PatchAttribs::kFanPoint);
        fFanPointAttrib = fanPoint;
    }

    // Updates the stroke params that are written out with each patch.
    // PatchAttribs::kStrokeParams must be enabled.
    void updateStrokeParamsAttrib(StrokeParams strokeParams) {
        SkASSERT(fAttribs & PatchAttribs::kStrokeParams);
        fStrokeParamsAttrib = strokeParams;
    }

    // Updates the color that will be written out with each patch.
    // PatchAttribs::kColor must be enabled.
    void updateColorAttrib(const SkPMColor4f& color) {
        SkASSERT(fAttribs & PatchAttribs::kColor);
        fColorAttrib.set(color, fAttribs & PatchAttribs::kWideColorIfEnabled);
    }

    /**
     * writeX functions for supported patch geometry types. Every geometric type is converted to an
     * equivalent cubic or conic, so this will always write at minimum 8 floats for the four control
     * points (cubic) or three control points and {w, inf} (conics). The PatchWriter additionally
     * writes the current values of all attributes enabled in its PatchAttribs flags.
     */

    // Write a cubic curve with its four control points.
    AI void writeCubic(float2 p0, float2 p1, float2 p2, float2 p3) {
        // TODO: Have cubic store or automatically compute wang's formula so this can automatically
        // call into chopAndWriteCubics.
        this->writePatch(p0, p1, p2, p3, kCubicCurveType);
    }
    AI void writeCubic(float4 p0p1, float4 p2p3) {
        this->writeCubic(p0p1.lo, p0p1.hi, p2p3.lo, p2p3.hi);
    }
    AI void writeCubic(float2 p0, float4 p1p2, float2 p3) {
        this->writeCubic(p0, p1p2.lo, p1p2.hi, p3);
    }
    AI void writeCubic(const SkPoint pts[4]) {
        this->writeCubic(float4::Load(pts), float4::Load(pts + 2));
    }

    // Write a conic curve with three control points and 'w', with the last coord of the last
    // control point signaling a conic by being set to infinity.
    AI void writeConic(float2 p0, float2 p1, float2 p2, float w) {
        // TODO: Have Conic store or automatically compute Wang's formula so this can automatically
        // call into chopAndWriteConics.
        this->writePatch(p0, p1, p2, {w, SK_FloatInfinity}, kConicCurveType);
    }
    AI void writeConic(const SkPoint pts[3], float w) {
        this->writeConic(skvx::bit_pun<float2>(pts[0]),
                         skvx::bit_pun<float2>(pts[1]),
                         skvx::bit_pun<float2>(pts[2]),
                         w);
    }

    // Write a quadratic curve that automatically converts its three control points into an
    // equivalent cubic.
    AI void writeQuadratic(float2 p0, float2 p1, float2 p2) {
        // TODO: Have Quadratic store or automatically compute Wang's formula so this can
        // automatically  call into chopAndWriteQuadratics *before* it is converted to an equivalent
        // cubic if needed.
        this->writeCubic(p0, mix(float4(p0, p2), p1.xyxy(), 2/3.f), p2);
    }
    AI void writeQuadratic(const SkPoint pts[3]) {
        this->writeQuadratic(skvx::bit_pun<float2>(pts[0]),
                             skvx::bit_pun<float2>(pts[1]),
                             skvx::bit_pun<float2>(pts[2]));
    }

    // TODO: Currently stroked lines are encoded as [p0,p0,p1,p1] and detected in the shaders;
    // usage of this should be converted over to writeLineAsCubic() and that should be how all
    // lines are handled w/o any need for shader detection.
    AI void writeLine(float2 p0, float2 p1) {
        this->writePatch(p0, p0, p1, p1, kCubicCurveType);
    }
    AI void writeLine(SkPoint p0, SkPoint p1) {
        this->writeLine(skvx::bit_pun<float2>(p0), skvx::bit_pun<float2>(p1));
    }
    // Write a line that is automatically converted into an equivalent cubic.
    AI void writeLineAsCubic(float4 p0p1) {
        this->writeCubic(p0p1.lo, (p0p1.zwxy() - p0p1) * (1/3.f) + p0p1, p0p1.hi);
    }
    AI void writeLineAsCubic(float2 p0, float2 p1) { this->writeLineAsCubic({p0, p1}); }

    // Write a triangle by setting it to a conic with w=Inf, and using a distinct
    // explicit curve type for when inf isn't supported in shaders.
    AI void writeTriangle(float2 p0, float2 p1, float2 p2) {
        this->writePatch(p0, p1, p2, {SK_FloatInfinity, SK_FloatInfinity},
                         kTriangularConicCurveType);
    }
    AI void writeTriangle(SkPoint p0, SkPoint p1, SkPoint p2) {
        this->writeTriangle(skvx::bit_pun<float2>(p0),
                            skvx::bit_pun<float2>(p1),
                            skvx::bit_pun<float2>(p2));
    }

    // Writes a circle used for round caps and joins in stroking, encoded as a cubic with
    // identical control points and an empty join.
    AI void writeCircle(float2 p) {
        // This does not use writePatch() because it uses its own location as the join attribute
        // value instead of fJoinControlPointAttrib.
        SkASSERT(fAttribs & PatchAttribs::kJoinControlPoint);
        if (VertexWriter vw = fChunker.appendVertex()) {
            vw << VertexWriter::Repeat<4>(p); // p0,p1,p2,p3 = p -> 4 copies
            this->emitPatchAttribs(std::move(vw), skvx::bit_pun<SkPoint>(p), kCubicCurveType);
        }
    }
    AI void writeCircle(SkPoint p) { this->writeCircle(skvx::bit_pun<float2>(p)); }

    SK_ALWAYS_INLINE void operator<<(MiddleOutPolygonTriangulator::PoppedTriangleStack&& stack) {
        for (auto [p0, p1, p2] : stack) {
            this->writeTriangle(p0, p1, p2);
        }
    }

    // Chops the given quadratic into 'numPatches' equal segments (in the parametric sense) and
    // writes them to the GPU buffer.
    //
    // Fills space between chops with triangles if PathPatchAttrib::kFanPoint is not enabled.
    void chopAndWriteQuads(float2 p0, float2 p1, float2 p2, int numPatches);

    // Chops the given conic into 'numPatches' equal segments (in the parametric sense) and
    // writes them to the GPU buffer.
    //
    // Fills space between chops with triangles if PathPatchAttrib::kFanPoint is not enabled.
    void chopAndWriteConics(float2 p0, float2 p1, float2 p2, float w, int numPatches);

    // Chops the given cubic into 'numPatches' equal segments (in the parametric sense) and
    // writes them to the GPU buffer.
    //
    // Fills space between chops with triangles if PathPatchAttrib::kFanPoint is not enabled.
    void chopAndWriteCubics(float2 p0, float2 p1, float2 p2, float2 p3, int numPatches);

private:
    template <typename T>
    static VertexWriter::Conditional<T> If(bool c, const T& v) { return VertexWriter::If(c,v); }

    AI void emitPatchAttribs(VertexWriter vertexWriter, SkPoint join, float explicitCurveType) {
        SkASSERT(!(fAttribs & PatchAttribs::kJoinControlPoint) || fHasJoinControlPoint);
        vertexWriter << If((fAttribs & PatchAttribs::kJoinControlPoint), join)
                     << If((fAttribs & PatchAttribs::kFanPoint), fFanPointAttrib)
                     << If((fAttribs & PatchAttribs::kStrokeParams), fStrokeParamsAttrib)
                     << If((fAttribs & PatchAttribs::kColor), fColorAttrib)
                     << If((fAttribs & PatchAttribs::kExplicitCurveType), explicitCurveType);
    }

    AI void writePatch(float2 p0, float2 p1, float2 p2, float2 p3, float explicitCurveType) {
        const bool defer = (fAttribs & PatchAttribs::kJoinControlPoint) &&
                           !fHasJoinControlPoint;
        if (defer) {
            SkASSERT(fDeferredCurveType < 0.f && explicitCurveType >= 0.f);
            fDeferredCurveType = explicitCurveType;
            p0.store(fDeferredControlPoints);
            p1.store(fDeferredControlPoints + 1);
            p2.store(fDeferredControlPoints + 2);
            p3.store(fDeferredControlPoints + 3);
        } else if (VertexWriter vw = fChunker.appendVertex()) {
            vw << p0 << p1 << p2 << p3;
            this->emitPatchAttribs(std::move(vw), fJoinControlPointAttrib, explicitCurveType);
        }

        if (fAttribs & PatchAttribs::kJoinControlPoint) {
            SkPoint nextControlPoint;
            if (explicitCurveType == kCubicCurveType && any(p3 != p2)) {
                // p2 is control point defining the tangent vector into the next patch.
                p2.store(&nextControlPoint);
            } else if (any(p2 != p1)) {
                // p1 is the control point defining the tangent vector.
                p1.store(&nextControlPoint);
            } else {
                // p0 is the control point defining the tangent vector.
                p0.store(&nextControlPoint);
            }
            this->updateJoinControlPointAttrib(nextControlPoint);
        }
    }

    const PatchAttribs fAttribs;
    GrVertexChunkBuilder fChunker;

    SkPoint fJoinControlPointAttrib;
    SkPoint fFanPointAttrib;
    StrokeParams fStrokeParamsAttrib;
    VertexColor fColorAttrib;

    SkPoint fDeferredControlPoints[4];

    // If >= 0.f, then fDeferredControlPoints are valid;
    float fDeferredCurveType = -1.f;
    bool fHasJoinControlPoint = false;


};

}  // namespace skgpu

#undef AI

#endif  // tessellate_PatchWriter_DEFINED
