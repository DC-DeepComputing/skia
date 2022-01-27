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
            , fDeferredN4{-1.f}
            , fChunker(target, vertexChunkArray, patchStride, initialAllocCount) {}

#if SK_GPU_V1
    // Create PatchWriters that write directly to the GrVertexChunkArrays stored on the provided
    // tessellators.
    PatchWriter(GrMeshDrawTarget*, PathTessellator*,
                int maxTessellationSegments, int initialPatchAllocCount);
    PatchWriter(GrMeshDrawTarget*, StrokeTessellator*,
                int maxTessellationSegments, int initialPatchAllocCount);
#endif

    PatchAttribs attribs() const { return fAttribs; }

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
        if (fDeferredN4 >= 0.f) {
            SkASSERT(fJoinControlPointAttrib.isFinite());
            // Overwrite join control point with updated value, which is the first attribute
            // after the 4 control points.
            memcpy(SkTAddOffset<void>(fDeferredPatchStorage.get(), 4*sizeof(SkPoint)),
                   &fJoinControlPointAttrib, sizeof(SkPoint));
            if (VertexWriter vw = this->appendPatch(fDeferredN4, /*defer=*/false)) {
                vw << VertexWriter::Array<char>(fDeferredPatchStorage.get(), fChunker.stride());
            }
            fDeferredN4 = -1.f;
            fJoinControlPointAttrib = {SK_ScalarNaN, SK_ScalarNaN};
        }
    }

    // Updates the fan point that will be written out with each patch (i.e., the point that wedges
    // fan around).
    //
    // PatchAttribs::kFanPoint must be enabled.
    void updateFanPointAttrib(SkPoint fanPoint) {
        SkASSERT(fAttribs & PatchAttribs::kFanPoint);
        fFanPointAttrib = fanPoint;
    }

    // Updates the stroke params that are written out with each patch.
    //
    // PatchAttribs::kStrokeParams must be enabled.
    void updateStrokeParamsAttrib(StrokeParams strokeParams) {
        SkASSERT(fAttribs & PatchAttribs::kStrokeParams);
        fStrokeParamsAttrib = strokeParams;
    }

    // Updates the color that will be written out with each patch.
    //
    // PathAttribs::kColor must be enabled.
    void updateColorAttrib(const SkPMColor4f& color) {
        SkASSERT(fAttribs & PatchAttribs::kColor);
        fColorAttrib.set(color, fAttribs & PatchAttribs::kWideColorIfEnabled);
    }

    // RAII. Appends a patch during construction and writes the attribs during destruction.
    //
    //    Patch(patchWriter, explicitCurveType) << p0 << p1 << ...;
    //
    // n4 is appropriate Wang's formula result to the fourth power, e.g. wangs_formula::cubic_pow4.
    // struct Patch {
    //     Patch(PatchWriter& w, float explicitCurveType, float n4)
    //             : fPatchWriter(w)
    //             , fVertexWriter(w.appendPatch(n4))
    //             , fExplicitCurveType(explicitCurveType) {}
    //     ~Patch() {
    //         fPatchWriter.emitPatchAttribs(std::move(fVertexWriter), fExplicitCurveType);
    //     }
    //     operator VertexWriter&() { return fVertexWriter; }
    //     PatchWriter& fPatchWriter;
    //     VertexWriter fVertexWriter;
    //     const float fExplicitCurveType;
    // };
    // struct Patch {
    //     Patch(float2 p0, float2 p1, float2 p2, float2 p3, float explicitCurveType, float n4)
    //             : fP0(p0)
    //             , fP1(p1)
    //             , fP2(p2)
    //             , fP3(p3)
    // }

    struct Line {
        Line(float2 p0, float2 p1) : fP0(p0), fP1(p1) {}

        float2 fP0, fP1;
    };
    struct Triangle {
        Triangle(float2 p0, float2 p1, float2 p2) : fP0(p0), fP1(p1), fP2(p2) {}

        float2 fP0, fP1, fP2;
    };

    struct Circle {
        Circle(float2 p) : fP(p) {}

        float2 fP;
    };
    struct Quadratic {
        Quadratic(float2 p0, float2 p1, float2 p2, float n4 = 0.f)
                : fP0(p0), fP1(p1), fP2(p2), fN4(n4) {}
        // Quadratic(float2 p0, float2 p1, float2 p2, float precision,
        //           const wangs_formula::VectorXform& shaderXform)
        //         : Quad(p0, p1, p2, wangs_formula::quadratic_pow4(precision, p0, p1, p2, shaderXform)) {}
        // Quadratic(float2 p0, float2 p1, float2 p2)
        //         : Quadratic(p0, p1, p2, 0.f) {}

        float2 fP0, fP1, fP2;
        float  fN4; // Computed or known result of Wang's formula to the 4th power
    };

    struct Cubic {
        Cubic(float2 p0, float2 p1, float2 p2, float2 p3, float n4 = 0.f)
                : fP0(p0), fP1(p1), fP2(p2), fP3(p3), fN4(n4) {}
        // Cubic(float2 p0, float2 p1, float2 p2, float2 p3, float precision,
        //       const wangs_formula::VectorXform& shaderXform)
        //         : Cubic(p0, p1, p2, p3, wangs_formula::cubic_pow4(precision, p0, p1, p2, p3, shaderXform)) {}
        // Cubic(float2 p0, float2 p1, float2 p2, float2 p3)
        //         : Cubic(p0, p1, p2, p3, 0.f) {}

        static Cubic From(const Quadratic& quad) {
            float4 pMid = mix(float4(quad.fP0, quad.fP2), quad.fP1.xyxy(), 2/3.f);
            return Cubic(quad.fP0, pMid.lo, pMid.hi, quad.fP2, quad.fN4);
        }

        static Cubic From(const Line& line) {
            float4 p0p1{line.fP0, line.fP1};
            float4 pMid = (p0p1.zwxy() - p0p1) * (1/3.f) + p0p1;
            return Cubic(line.fP0, pMid.lo, pMid.hi, line.fP1);
        }

        float2 fP0, fP1, fP2, fP3;
        float  fN4; // Computed or known result of Wang's formula to the 4th power
    };

    struct Conic {
        Conic(float2 p0, float2 p1, float2 p2, float w, float n2)
                : fP0(p0), fP1(p1), fP2(p2), fW(w), fN2(n2) {}
        // Conic(float2 p0, float2 p1, float2 p2, float w, float precision,
        //       const wangs_formula::VectorXform& shaderXform)
        //         : Conic(p0, p1, p2, w, wangs_formula::conic_pow2(precision, p0, p1, p2, w, shaderXform)) {}
        Conic(float2 p0, float2 p1, float2 p2, float w)
                : Conic(p0, p1, p2, w, 0.f) {}

        static Conic From(const Triangle& tri) {
            return Conic(tri.fP0, tri.fP1, tri.fP2, SK_FloatInfinity);
        }

        float2 fP0, fP1, fP2;
        float  fW;
        float  fN2; // Computed or known result of Wang's formula squared
    };

    void writePatch(float2 p0, float2 p1, float2 p2, float2 p3, float explicitCurveType, float n4) {
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

            this->emitPatchAttribs(std::move(vw), explicitCurveType);
        }
    }

    SK_ALWAYS_INLINE void operator<<(const Cubic& cubic) {
        if (cubic.fN4 <= fMaxSegments_pow4) {
            this->writePatch(cubic.fP0, cubic.fP1, cubic.fP2, cubic.fP3,
                             kCubicCurveType, cubic.fN4);
        } else {
            int numPatches = SkScalarCeilToInt(wangs_formula::root4(
                std::min(cubic.fN4, pow4(kMaxTessellationSegmentsPerCurve)) / fMaxSegments_pow4));
            this->chopAndWriteCubics(cubic.fP0, cubic.fP1, cubic.fP2, cubic.fP3, numPatches);
        }
    }

    SK_ALWAYS_INLINE void operator<<(const Quadratic& quad) {
        if (quad.fN4 <= fMaxSegments_pow4) {
            *this << Cubic::From(quad);
        } else {
            int numPatches = SkScalarCeilToInt(wangs_formula::root4(
                std::min(quad.fN4, pow4(kMaxTessellationSegmentsPerCurve)) / fMaxSegments_pow4));
            this->chopAndWriteQuads(quad.fP0, quad.fP1, quad.fP2, numPatches);
        }
    }

    SK_ALWAYS_INLINE void operator<<(const Conic& conic) {
        SkASSERT(VertexWriter::kIEEE_32_infinity == SkFloat2Bits(SK_FloatInfinity));
        if (conic.fN2 < fMaxSegments_pow2) {
            this->writePatch(conic.fP0, conic.fP1, conic.fP2, {conic.fW, SK_FloatInfinity},
                             kConicCurveType, conic.fN2 * conic.fN2);
        } else {
            int numPatches = SkScalarCeilToInt(sqrtf(
                std::min(conic.fN2, pow2(kMaxTessellationSegmentsPerCurve)) / fMaxSegments_pow2));
            this->chopAndWriteConics(conic.fP0, conic.fP1, conic.fP2, conic.fW, numPatches);
        }
    }

    SK_ALWAYS_INLINE void operator<<(const Circle& circle) {
        // The shader interprets an empty cubic + empty join as a special case that denotes a
        // circle, or 180-degree point stroke. This does not use writePatch() because it does not
        // modify the join control point for the rest of the stroke.
        SkASSERT(fAttribs & PatchAttribs::kJoinControlPoint);
        if (VertexWriter vw = this->appendPatch(fMaxSegments_pow4, /*defer=*/false)) {
            vw << VertexWriter::Repeat<5>(circle.fP); // p0,p1,p2,p3,join = p -> 5 copies
            this->emitPatchAttribs(std::move(vw), kCubicCurveType);
        }
    }

    SK_ALWAYS_INLINE void operator<<(const Line& line) {
        *this << Cubic::From(line);
    }

    SK_ALWAYS_INLINE void operator<<(const Triangle& tri) {
        *this << Conic::From(tri);
    }

    SK_ALWAYS_INLINE void operator<<(MiddleOutPolygonTriangulator::PoppedTriangleStack&& stack) {
        for (auto [p0, p1, p2] : stack) {
            *this << Triangle(float2::Load(&p0), float2::Load(&p1), float2::Load(&p2));
        }
    }

    // RAII. Appends a patch during construction and writes the remaining data for a cubic during
    // destruction. The caller outputs p0,p1,p2,p3 (8 floats):
    //
    //    CubicPatch(patchWriter) << p0 << p1 << p2 << p3;
    //
    // n4 should be result of wangs_formula::cubic_pow4(kTessellationPrecision, p0,p1,p2,p3), or
    // wangs_formula::quadratic_pow4(p0,p1,p2) if using QuadToCubic(p0, p1, p2).
    // struct CubicPatch : public Patch {
    //     CubicPatch(PatchWriter& w, float n4) : Patch(w, kCubicCurveType, n4) {}
    // };

    // RAII. Appends a patch during construction and writes the remaining data for a conic during
    // destruction. The caller outputs p0,p1,p2,w (7 floats):
    //
    //     ConicPatch(patchWriter) << p0 << p1 << p2 << w;
    //
    // n4 should be result of wangs_formula::conic_pow4(kTessellationPrecision, p0,p1,p2,w);
    // struct ConicPatch : public Patch {
    //     ConicPatch(PatchWriter& w, float n4) : Patch(w, kConicCurveType, n4) {}
    //     ~ConicPatch() {
    //         fVertexWriter << VertexWriter::kIEEE_32_infinity;  // p3.y=Inf indicates a conic.
    //     }
    // };

    // RAII. Appends a patch during construction and writes the remaining data for a triangle during
    // destruction. The caller outputs p0,p1,p2 (6 floats):
    //
    //     TrianglePatch(patchWriter) << p0 << p1 << p2;
    //
    // struct TrianglePatch : public Patch {
    //     TrianglePatch(PatchWriter& w) : Patch(w, kTriangularConicCurveType, 1.f) {}
    //     ~TrianglePatch() {
    //         // Mark this patch as a triangle by setting it to a conic with w=Inf.
    //         fVertexWriter << VertexWriter::Repeat<2>(VertexWriter::kIEEE_32_infinity);
    //     }
    // };

    // Helper functions that automatically evaluate Wang's formula on the control points, assuming
    // that a subsequent shader applies 'shaderXform' to reach device space. Will chop as needed
    // based on configured max segments that are supported. Returns the result of the Wang's formula
    // raised to the 4th power, e.g. wangs_formula::quadratic_pow4
    //
    // Fills space between chops with triangles if PathPatchAttrib::kFanPoint is not enabled.

private:
    VertexWriter appendPatch(float n4, bool defer) {
        // TODO: For now providing n4 here is purely validation, but Graphite will use it when
        // appending instance data and calculating the template index count on the fly.
        SkASSERT(n4 <= fMaxSegments_pow4);

        if (defer) {
            // The patch needs to be written to the deferred storage, which holds the entire patch
            // except the stroke control point attrib, which gets filled in when
            // closeStrokedContour() is called.
            SkASSERT(fDeferredN4 < 0.f); // Shouldn't have a deferred patch already
            fDeferredN4 = n4;
            return {fDeferredPatchStorage.reset(fChunker.stride()), fChunker.stride()};
        }

        return fChunker.appendVertex();
    }

    template <typename T>
    static VertexWriter::Conditional<T> If(bool c, const T& v) { return VertexWriter::If(c,v); }

    // NOTE: Does *not* write the kJoinControlPoint attrib since that requires special handling.
    void emitPatchAttribs(VertexWriter vertexWriter, float explicitCurveType) {
        vertexWriter << If((fAttribs & PatchAttribs::kFanPoint), fFanPointAttrib)
                     << If((fAttribs & PatchAttribs::kStrokeParams), fStrokeParamsAttrib)
                     << If((fAttribs & PatchAttribs::kColor), fColorAttrib)
                     << If((fAttribs & PatchAttribs::kExplicitCurveType), explicitCurveType);
    }

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

    // Only used if kJoinControlPointAttrib is set in fAttribs, in which case it holds data for
    // a single patch waiting for the incoming join control point to be computed.
    // Contents are valid (sans join control point) if fDeferredN4 >= 0.
    SkAutoTMalloc<char> fDeferredPatchStorage;
    // The n^4 segment value to be passed to appendPatch() when pushing the deferred patch, or
    // a negative value if there is no deferred patch.
    float fDeferredN4;

    GrVertexChunkBuilder fChunker;
};

}  // namespace skgpu

#endif  // tessellate_PatchWriter_DEFINED
