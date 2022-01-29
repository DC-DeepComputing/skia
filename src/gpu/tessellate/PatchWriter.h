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
#include "src/gpu/tessellate/WangsFormula.h"

namespace skgpu {

#if SK_GPU_V1
class PathTessellator;
class StrokeTessellator;
#endif

// Writes out tessellation patches, formatted with their specific attribs, to a GPU buffer.
class PatchWriter {
    using VectorXform = wangs_formula::VectorXform;
public:
    PatchWriter(GrMeshDrawTarget* target,
                GrVertexChunkArray* vertexChunkArray,
                PatchAttribs attribs,
                int maxTessellationSegments,
                size_t patchStride,
                int initialAllocCount)
            : fAttribs(attribs)
            , fMaxSegments_pow2(pow2(maxTessellationSegments))
            , fMaxSegments_pow4(pow2(fMaxSegments_pow2))
            , fJoinControlPointAttrib{SK_ScalarNaN, SK_ScalarNaN}
            , fChunker(target, vertexChunkArray, patchStride, initialAllocCount) {
    }

#if SK_GPU_V1
    // Create PatchWriters that write directly to the GrVertexChunkArrays stored on the provided
    // tessellators.
    PatchWriter(GrMeshDrawTarget*, PathTessellator*,
                int maxTessellationSegments, int initialPatchAllocCount);
    PatchWriter(GrMeshDrawTarget*, StrokeTessellator*,
                int maxTessellationSegments, int initialPatchAllocCount);
#endif

    ~PatchWriter() {
        // finishStrokeContour() should have been called before this was deleted (or never used).
        SkASSERT(fDeferredPatchN4 < 0.f && !fJoinControlPointAttrib.isFinite());
    }

    PatchAttribs attribs() const { return fAttribs; }

    void resetFixedSegments(int minFixedSegments) {
        float segments_pow4 = pow2(pow2(minFixedSegments));
        SkASSERT(segments_pow4 <= fMaxSegments_pow4);
        fCurrMinSegments_pow4 = segments_pow4;
    }
    // Fast log2 of minimum required # of segments per tracked Wang's formula calculations.
    int requiredResolveLevel() const {
        return wangs_formula::nextlog16(fCurrMinSegments_pow4); // log16(n^4) == log2(n)
    }
    // Fast minimum required # of segments from tracked Wang's formula calculations.
    int requiredFixedSegments() const {
        return SkScalarCeilToInt(wangs_formula::root4(fCurrMinSegments_pow4));
    }

    // Updates the stroke's join control point that will be written out with each patch. This is
    // automatically adjusted when appending various geometries (e.g. Conic/Cubic), but sometimes
    // must be set explicitly.
    //
    // PatchAttribs::kJoinControlPoint must be enabled.
    void updateJoinControlPointAttrib(SkPoint lastControlPoint) {
        SkASSERT(fAttribs & PatchAttribs::kJoinControlPoint && lastControlPoint.isFinite());
        fJoinControlPointAttrib = lastControlPoint;
    }
    // Completes a closed contour of a stroke by rewriting a deferred patch with now-available
    // join control point information.
    //
    // PatchAttribs::kJoinControlPoint must be enabled.
    void finishStrokeContour() {
        SkASSERT(fAttribs & PatchAttribs::kJoinControlPoint);
        if (fDeferredPatchN4 >= 0.f) {
            SkASSERT(fJoinControlPointAttrib.isFinite());
            // Overwrite join control point with updated value, which is the first attribute
            // after the 4 control points.
            memcpy(SkTAddOffset<void>(fDeferredPatchStorage.get(), 4 * sizeof(SkPoint)),
                   &fJoinControlPointAttrib, sizeof(SkPoint));
            if (VertexWriter vw = this->appendPatch(fDeferredPatchN4, /*defer=*/false)) {
                vw << VertexWriter::Array<char>(fDeferredPatchStorage.get(), fChunker.stride());
            }
        }

        fDeferredPatchN4 = -1.f;
        fJoinControlPointAttrib = {SK_ScalarNaN, SK_ScalarNaN};
    }
    // TODO: This is only used by StrokeHardwareTessellator and ideally its patch writing logic
    // should be simplified like StrokeFixedCountTessellator's, and then this can go away.
    SkPoint joinControlPoint() const { return fJoinControlPointAttrib; }

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
     * Helper structs that represent a path verb and geometry, that can be used to convert to
     * patches of instance data using operator<< on the PatchWriter, e.g.
     *
     *   patchWriter << Cubic(p0, p1, p2, p3);
     *   patchWriter << Conic(p0, p1, p2, w);
     *   patchWriter << Quadratic(p0, p1, p2);
     *   patchWriter << Line(p0, p1);
     *   patchWriter << Triangle(p0, p1, p2);
     *   patchWriter << Circle(p); // only for strokes
     *
     * Every geometric type is converted to an equivalent cubic or conic, so this will always
     * write at minimum 8 floats for the four control points (cubic) or three control points and
     * {w, inf} (conics). The PatchWriter additionally writes the current values of all attributes
     * enabled in its PatchAttribs flags.
     *
     * Curve data can either store or automatically compute Wang's formula for their control points.
     * This segmentation level is used in operator<< to automatically chop if it is larger than
     * the supported maximum segments of the PatchWriter. The PatchWriter tracks the maximum
     * encountered segmentation level for callers to later select an index count for instance draws.
     */

    // Defines a line that will be automatically converted into an equivalent cubic patch.
    struct Line {
        Line(float4 p0p1) : fP0{p0p1.lo}, fP1{p0p1.hi} {}
        Line(float2 p0, float2 p1) : fP0{p0}, fP1{p1} {}
        Line(SkPoint p0, SkPoint p1)
                : fP0{skvx::bit_pun<float2>(p0)}
                , fP1{skvx::bit_pun<float2>(p1)} {}

        float2 fP0, fP1;
    };

    // Defines a triangle that will be automatically converted into a conic patch (w = inf).
    struct Triangle {
        Triangle(float2 p0, float2 p1, float2 p2) : fP0(p0), fP1(p1), fP2(p2) {}
        Triangle(SkPoint p0, SkPoint p1, SkPoint p2)
                : fP0(skvx::bit_pun<float2>(p0))
                , fP1(skvx::bit_pun<float2>(p1))
                , fP2(skvx::bit_pun<float2>(p2)) {}

        float2 fP0, fP1, fP2;
    };

    // Defines a circle used for round caps and joins in stroking, encoded as a cubic with
    // identical control points and an empty join.
    struct Circle {
        Circle(float2 p) : fP(p) {}
        Circle(SkPoint p) : fP(skvx::bit_pun<float2>(p)) {}

        float2 fP;
    };

    // Defines a quadratic curve that is automatically converted into an equivalent cubic.
    struct Quadratic {
        Quadratic(float2 p0, float2 p1, float2 p2, float n4=0.f)
                : fP0(p0), fP1(p1), fP2(p2), fN4(n4) {}
        Quadratic(const SkPoint p[3], float n4=0.f)
                : fP0(float2::Load(p))
                , fP1(float2::Load(p + 1))
                , fP2(float2::Load(p + 2))
                , fN4(n4) {}

        // Helper to compute and set N^4 from Wang's formula with this quadratic's control points,
        // assuming that 'shaderXform' will be applied later to the points, in a GPU shader.
        Quadratic& segmentedFor(const VectorXform& shaderXform,
                                float precision = kTessellationPrecision) {
            fN4 = wangs_formula::quadratic_pow4(precision, fP0, fP1, fP2, shaderXform);
            return *this;
        }

        float2 fP0, fP1, fP2;
        float  fN4;
    };

    // Defines a cubic curve from four control points.
    struct Cubic {
        Cubic(float2 p0, float2 p1, float2 p2, float2 p3, float n4=0.f)
                : fP0(p0), fP1(p1), fP2(p2), fP3(p3), fN4(n4) {}
        Cubic(float4 p0p1, float4 p2p3, float n4=0.f)
                : fP0(p0p1.lo), fP1(p0p1.hi), fP2(p2p3.lo), fP3(p2p3.hi), fN4(n4) {}
        Cubic(float2 p0, float4 p1p2, float2 p3, float n4=0.f)
                : fP0(p0), fP1(p1p2.lo), fP2(p1p2.hi), fP3(p3), fN4(n4) {}
        Cubic(const SkPoint p[4], float n4=0.f)
                : Cubic(float4::Load(p), float4::Load(p + 2), n4) {}

        static Cubic From(const Quadratic& quad) {
            float4 pMid = mix(float4(quad.fP0, quad.fP2), quad.fP1.xyxy(), 2/3.f);
            return Cubic(quad.fP0, pMid, quad.fP2, quad.fN4);
        }

        static Cubic From(const Line& line) {
            float4 p0p1{line.fP0, line.fP1};
            float4 pMid = (p0p1.zwxy() - p0p1) * (1/3.f) + p0p1;
            return Cubic(line.fP0, pMid, line.fP1, 1.f);
        }

        // Helper to compute and set N^4 from Wang's formula with this cubic's control points,
        // assuming that 'shaderXform' will be applied later to the points, in a GPU shader.
        Cubic& segmentedFor(const VectorXform& shaderXform,
                            float precision = kTessellationPrecision) {
            fN4 = wangs_formula::cubic_pow4(precision, fP0, fP1, fP2, fP3, shaderXform);
            return *this;
        }

        float2 fP0, fP1, fP2, fP3;
        float  fN4;
    };

    // Defines a conic curve from three control points and 'w'.
    struct Conic {
        Conic(float2 p0, float2 p1, float2 p2, float w, float n2=0.f)
                : fP0(p0), fP1(p1), fP2(p2), fW(w), fN2(n2) {}
        Conic(const SkPoint p[3], float w, float n2=0.f)
                : fP0(float2::Load(p))
                , fP1(float2::Load(p + 1))
                , fP2(float2::Load(p + 2))
                , fW(w)
                , fN2(n2) {}

        // Helper to compute and set N^4 from Wang's formula with this conic's control points,
        // assuming that 'shaderXform' will be applied later to the points, in a GPU shader.
        Conic& segmentedFor(const VectorXform& shaderXform,
                            float precision = kTessellationPrecision) {
            fN2 = wangs_formula::conic_pow2(precision, fP0, fP1, fP2, fW, shaderXform);
            return *this;
        }

        float2 fP0, fP1, fP2;
        float  fW;
        float  fN2;
    };

    // operator<< definitions for the patch geometry types.

    SK_ALWAYS_INLINE void operator<<(const Cubic& cubic) {
        if (!(fAttribs & PatchAttribs::kFanPoint) &&
            !(fAttribs & PatchAttribs::kJoinControlPoint)) {
            if (cubic.fN4 <= 1.f) {
                // This cubic only needs one segment (e.g. a line) but we're not filling space with
                // fans or stroking, so nothing actually needs to be drawn.
                return;
            }
        }
        if (cubic.fN4 <= fMaxSegments_pow4) {
            this->writePatch(cubic.fP0, cubic.fP1, cubic.fP2, cubic.fP3,
                             cubic.fN4, kCubicCurveType);
        } else {
            int numPatches = SkScalarCeilToInt(wangs_formula::root4(
                std::min(cubic.fN4, pow4(kMaxTessellationSegmentsPerCurve)) / fMaxSegments_pow4));
            this->chopAndWriteCubics(cubic.fP0, cubic.fP1, cubic.fP2, cubic.fP3, numPatches);
        }
    }

    SK_ALWAYS_INLINE void operator<<(const Conic& conic) {
        if (!(fAttribs & PatchAttribs::kFanPoint) &&
            !(fAttribs & PatchAttribs::kJoinControlPoint)) {
            if (conic.fN2 <= 1.f) {
                // This conic only needs one segment (e.g. a line) but we're not filling space with
                // fans or stroking, so nothing actually needs to be drawn.
                return;
            }
        }
        if (conic.fN2 <= fMaxSegments_pow2) {
            this->writePatch(conic.fP0, conic.fP1, conic.fP2, {conic.fW, SK_FloatInfinity},
                             conic.fN2 * conic.fN2, kConicCurveType);
        } else {
            int numPatches = SkScalarCeilToInt(sqrtf(
                std::min(conic.fN2, pow2(kMaxTessellationSegmentsPerCurve)) / fMaxSegments_pow2));
            this->chopAndWriteConics(conic.fP0, conic.fP1, conic.fP2, conic.fW, numPatches);
        }
    }

    SK_ALWAYS_INLINE void operator<<(const Quadratic& quad) {
        if (!(fAttribs & PatchAttribs::kFanPoint) &&
            !(fAttribs & PatchAttribs::kJoinControlPoint)) {
            if (quad.fN4 <= 1.f) {
                // This quad only needs one segment (e.g. a line) but we're not filling space with
                // fans or stroking, so nothing actually needs to be drawn.
                return;
            }
        }
        if (quad.fN4 <= fMaxSegments_pow4) {
            // This won't chop, since we know n^4 <= maxSegments^4
            *this << Cubic::From(quad);
        } else {
            int numPatches = SkScalarCeilToInt(wangs_formula::root4(
                std::min(quad.fN4, pow4(kMaxTessellationSegmentsPerCurve)) / fMaxSegments_pow4));
            this->chopAndWriteQuads(quad.fP0, quad.fP1, quad.fP2, numPatches);
        }
    }

    SK_ALWAYS_INLINE void operator<<(const Line& line) { *this << Cubic::From(line); }

    SK_ALWAYS_INLINE void operator<<(const Triangle& tri) {
        static const float kTriFixedSegments_pow4 = 2.f * 2.f * 2.f * 2.f;
        // Mark this patch as a triangle by setting it to a conic with w=Inf, and use a distinct
        // explicit curve type for when inf isn't supported in shaders.
        this->writePatch(tri.fP0, tri.fP1, tri.fP2, {SK_FloatInfinity, SK_FloatInfinity},
                         kTriFixedSegments_pow4, kTriangularConicCurveType);
    }

    SK_ALWAYS_INLINE void operator<<(const Circle& circle) {
        // This does not use writePatch() because it uses its own location as the join attribute
        // value instead of fJoinControlPointAttrib.
        SkASSERT(fAttribs & PatchAttribs::kJoinControlPoint);
        if (VertexWriter vw = this->appendPatch(0.f, /*defer=*/false)) {
            vw << VertexWriter::Repeat<5>(circle.fP); // p0,p1,p2,p3,join = p -> 5 copies
            this->emitSimplePatchAttribs(std::move(vw), kCubicCurveType);
        }
    }

    SK_ALWAYS_INLINE void operator<<(MiddleOutPolygonTriangulator::PoppedTriangleStack&& stack) {
        for (auto [p0, p1, p2] : stack) {
            *this << Triangle(p0, p1, p2);
        }
    }

private:
    VertexWriter appendPatch(float n4, bool defer) {
        SkASSERT(n4 >= 0.f && n4 <= fMaxSegments_pow4);

        if (defer) {
            // The patch needs to be written to the deferred storage, which holds the entire patch
            // except the stroke control point attrib, which gets filled in when
            // finishStrokeContour() is called.
            SkASSERT(fDeferredPatchN4 < 0.f); // Shouldn't have a deferred patch yet.
            fDeferredPatchN4 = n4;
            return {fDeferredPatchStorage.reset(fChunker.stride()), fChunker.stride()};
        } else {
            // Append directly to GPU storage
            fCurrMinSegments_pow4 = std::max(n4, fCurrMinSegments_pow4);
            return fChunker.appendVertex();
        }
    }

    template <typename T>
    static VertexWriter::Conditional<T> If(bool c, const T& v) { return VertexWriter::If(c,v); }

    // NOTE: Does *not* write the kJoinControlPoint attrib since that requires special handling.
    void emitSimplePatchAttribs(VertexWriter vertexWriter, float explicitCurveType) {
        vertexWriter << If((fAttribs & PatchAttribs::kFanPoint), fFanPointAttrib)
                     << If((fAttribs & PatchAttribs::kStrokeParams), fStrokeParamsAttrib)
                     << If((fAttribs & PatchAttribs::kColor), fColorAttrib)
                     << If((fAttribs & PatchAttribs::kExplicitCurveType), explicitCurveType);
    }

    void writePatch(float2 p0, float2 p1, float2 p2, float2 p3, float n4, float explicitCurveType) {
        const bool defer = (fAttribs & PatchAttribs::kJoinControlPoint) &&
                           !fJoinControlPointAttrib.isFinite();
        if (VertexWriter vw = this->appendPatch(n4, defer)) {
            vw << p0 << p1 << p2 << p3;
            if (fAttribs & PatchAttribs::kJoinControlPoint) {
                // NOTE: fStrokeControlPointAttrib will contain NaN if we're writing to a deferred
                // patch. If that's the case, correct data will overwrite it when the contour is
                // closed (this is fine since a deferred patch writes to CPU memory instead of
                // directly to the GPU buffer).
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

                vw << fJoinControlPointAttrib;
                fJoinControlPointAttrib = nextControlPoint;
            }
            this->emitSimplePatchAttribs(std::move(vw), explicitCurveType);
        }
    }

    // Helpers that chop the curve type into 'numPatches' parametrically uniform curves. It is
    // assumed that 'numPatches' is calculated such that the resulting curves require the maximum
    // number of segments to draw appropriately (since the original presumably needed even more).
    void chopAndWriteQuads(float2 p0, float2 p1, float2 p2, int numPatches);
    void chopAndWriteConics(float2 p0, float2 p1, float2 p2, float w, int numPatches);
    void chopAndWriteCubics(float2 p0, float2 p1, float2 p2, float2 p3, int numPatches);

    const PatchAttribs fAttribs;
    const float fMaxSegments_pow2;
    const float fMaxSegments_pow4;

    SkPoint fFanPointAttrib;
    StrokeParams fStrokeParamsAttrib;
    VertexColor fColorAttrib;

    // Stores NaN until it's been updated by the last written patch or set explicitly by controlling
    // code. It must be reset to NaN when a stroked contour is closed.
    SkPoint fJoinControlPointAttrib;

    // The current minimum segments required for written patches, always <= fMaxSegments_pow4
    float fCurrMinSegments_pow4 = 0.f;

    GrVertexChunkBuilder fChunker;

    // Only used if kJoinControlPointAttrib is set in fAttribs, in which case it holds data for
    // a single patch waiting for the incoming join control point to be computed.
    // Contents are valid (sans join control point) if fDeferredPatchN4 >= 0.f.
    SkAutoTMalloc<char> fDeferredPatchStorage;
    // The n^4 segment value to be passed to appendPatch() when pushing the deferred patch, or
    // a negative value if there is no deferred patch.
    float fDeferredPatchN4 = -1.f;
};

}  // namespace skgpu

#endif  // tessellate_PatchWriter_DEFINED
