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

#include <type_traits>
#include <variant>

#define AI SK_ALWAYS_INLINE
#define ENABLE_IF(cond, rt) template <typename RT=rt> std::enable_if_t<cond, RT>
#define ENABLE_FOR_ATTRIB(attrib) ENABLE_IF(has_attrib<PatchAttribs::attrib>::value, void)

namespace skgpu {

// TODO: Document trait system for attribs and behaviors in PatchWriter
template <PatchAttribs A> struct Required {};
template <PatchAttribs A> struct Optional {};

// TODO: combine defer and update join into one feature
struct AutoDeferPatches {};
struct AutoUpdateJoin {};
struct AddTrianglesWhenChopping {};
struct DiscardFlatCurves {};
struct WriteRawPatches {};

template <PatchAttribs Enabled>
struct DeferredPatchImpl {
    static constexpr size_t kMaxStride = 4 * sizeof(SkPoint) + PatchAttribsStride(Enabled);

    // Holds an entire patch, except with an undefined join control point.
    char fData[kMaxStride];
    bool fHasPending = false; // True means the deferred patch in fData needs to be written out
    bool fMustDefer  = false; // True means next patch must be deferred
};

// An no-op object that has the same constructor signature as MiddleOutPolygonTriangulator
struct NullTriangulator {
    NullTriangulator(int, SkPoint) {}
};

// Writes out tessellation patches, formatted with their specific attribs, to a GPU buffer.
// template <PatchAttribs Enabled, PatchAttribs Dynamic>
template <typename... Traits>
class PatchWriter {
    // Helpers to extract specifics from the template traits pack.
    template <typename F>     struct has_feature : std::disjunction<std::is_same<F, Traits>...> {};
    template <PatchAttribs A> using  req_attrib  = has_feature<Required<A>>;
    template <PatchAttribs A> using  opt_attrib  = has_feature<Optional<A>>;
    template <PatchAttribs A> using  has_attrib  = std::disjunction<req_attrib<A>, opt_attrib<A>>;

    template <template<PatchAttribs> typename W, typename T>
    struct filter : std::integral_constant<PatchAttribs, PatchAttribs::kNone> {};
    template <template<PatchAttribs> typename W, PatchAttribs A>
    struct filter<W, W<A>> : std::integral_constant<PatchAttribs, A> {};

    // NOTE: using an empty struct (std::monostate) for disabled attributes still uses up 1 byte in
    // PatchWriter since C++ required that each field is uniquely addressable.
    template <typename T, PatchAttribs A>
    using attrib_t = std::conditional_t<req_attrib<A>::value, T,
                     std::conditional_t<opt_attrib<A>::value, std::optional<T>,
                                                   /* else */ std::monostate>>;

    static constexpr bool kDeferPatches             = has_feature<AutoDeferPatches>::value;
    static constexpr bool kUpdateJoin               = has_feature<AutoUpdateJoin>::value;
    static constexpr bool kAddTrianglesWhenChopping = has_feature<AddTrianglesWhenChopping>::value;
    static constexpr bool kDiscardFlatCurves        = has_feature<DiscardFlatCurves>::value;

    static constexpr PatchAttribs kRequired = (filter<Required, Traits>::value | ... | PatchAttribs::kNone);
    static constexpr PatchAttribs kEnabled =  (filter<Optional, Traits>::value | ... | kRequired);

    using JoinAttrib     = attrib_t<SkPoint,      PatchAttribs::kJoinControlPoint>;
    using FanPointAttrib = attrib_t<SkPoint,      PatchAttribs::kFanPoint>;
    using StrokeAttrib   = attrib_t<StrokeParams, PatchAttribs::kStrokeParams>;

    // kWideColorIfEnabled does not define an attribute, but changes the type of the kColor attrib.
    using Color          =
            std::conditional_t<req_attrib<PatchAttribs::kWideColorIfEnabled>::value, SkPMColor4f,
            std::conditional_t<opt_attrib<PatchAttribs::kWideColorIfEnabled>::value, VertexColor,
                                                                          /* else */ uint32_t>>;
    using ColorAttrib    = attrib_t<Color, PatchAttribs::kColor>;
    // NOTE: We don't bother defining a type for kExplicitCurveType attrib since emitPatchAttribs()
    // just ignores the float parameter, it's never stored on the PatchWriter itself.

    // Types that vary depending on the activated features, but do not define the patch data.
    // TODO: worth doing it this way? Or just always having somethign with max stride that might
    // just be ignored?
    using DeferredPatch = std::conditional_t<kDeferPatches,
            DeferredPatchImpl<kEnabled>, std::monostate>;

    // TODO: See if performance changes with just always using middle out triangulator;
    // maybe now that chopEtc are in header, the compiler can see they aren't ever used and delete
    //  it entirely -> seems unlikely since ctor could have side effects it can't know about?
    //    -> or maybe that will lead to errors because in the constexpr disabled cases we now never
    //       actually reference it anymore?
    using InnerTriangulator = std::conditional_t<kAddTrianglesWhenChopping,
            MiddleOutPolygonTriangulator, NullTriangulator>;
    using VectorXform = wangs_formula::VectorXform;

    static_assert((!kDeferPatches && !kUpdateJoin) || (kRequired & PatchAttribs::kJoinControlPoint),
                  "Deferred patches and auto-updating joins requires kJoinControlPoint attrib");
public:
    PatchWriter(GrMeshDrawTarget* target,
                GrVertexChunkArray* vertexChunkArray,
                PatchAttribs attribs,
                int maxTessellationSegments,
                int initialAllocCount)
            : fAttribs(attribs)
            , fMaxSegments_pow2(pow2(maxTessellationSegments))
            , fMaxSegments_pow4(pow2(fMaxSegments_pow2))
            , fCurrMinSegments_pow4(1.f)
            , fChunker(target, vertexChunkArray, PatchStride(attribs), initialAllocCount) {
        // TODO: This gets cleaner if PatchAttribs is updated to use EnumBitMask.h
        SkASSERT(((int) attribs & (int) kRequired) == (int) kRequired); // all required must be set
        SkASSERT(((int) attribs & ~(int) kEnabled) == 0); // no disabled attribs are requested

        // TODO: Need to initialize all the attrib values, which can't just be {} for those that
        // map to std::optional<T> because we want to check attribs&A. If it's runtime enabled then
        // the optional should have T{} by default; otherwise it should not have any value.
    }

    ~PatchWriter() {
        if constexpr (kDeferPatches) {
            // finishStrokeContour() should have been called before this was deleted (or never used)
            SkASSERT(!fDeferredPatch.fHasPending);
        }
    }

    PatchAttribs attribs() const { return fAttribs; }

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
    ENABLE_FOR_ATTRIB(kJoinControlPoint) updateJoinControlPointAttrib(SkPoint lastControlPoint) {
        SkASSERT(fAttribs & PatchAttribs::kJoinControlPoint);
        fJoin = lastControlPoint;
        if constexpr (kDeferPatches) {
            fDeferredPatch.fMustDefer = false;
        }
    }
    // Completes a closed contour of a stroke by rewriting a deferred patch with now-available
    // join control point information. Automatically resets the join control point attribute.
    ENABLE_IF(kDeferPatches, void) writeDeferredStrokePatch() {
        if (fDeferredPatch.fHasPending) {
            SkASSERT(!fDeferredPatch.fMustDefer);
            // Overwrite join control point with updated value, which is the first attribute
            // after the 4 control points.
            memcpy(SkTAddOffset<void>(fDeferredPatch.fData, 4 * sizeof(SkPoint)),
                   &fJoin, sizeof(SkPoint));
            if (VertexWriter vw = fChunker.appendVertex()) {
                vw << VertexWriter::Array<char>(fDeferredPatch.fData, fChunker.stride());
            }
        }

        fDeferredPatch.fHasPending = false;
        fDeferredPatch.fMustDefer = true;
    }

    // Updates the fan point that will be written out with each patch (i.e., the point that wedges
    // fan around).
    ENABLE_FOR_ATTRIB(kFanPoint) updateFanPointAttrib(SkPoint fanPoint) {
        SkASSERT(fAttribs & PatchAttribs::kFanPoint);
        fFanPoint = fanPoint;
    }

    // Updates the stroke params that are written out with each patch.
    ENABLE_FOR_ATTRIB(kStrokeParams) updateStrokeParamsAttrib(StrokeParams strokeParams) {
        SkASSERT(fAttribs & PatchAttribs::kStrokeParams);
        fStrokeParams = strokeParams;
    }

    // Updates the color that will be written out with each patch.
    ENABLE_FOR_ATTRIB(kColor) updateColorAttrib(const SkPMColor4f& color) {
        SkASSERT(fAttribs & PatchAttribs::kColor);
        if constexpr (kRequired & PatchAttribs::kWideColorIfEnabled) {
            fColor = color;
        } else if constexpr (kEnabled & PatchAttribs::kWideColorIfEnabled) {
            // FIXME this probably compiles, but optional<Color> can't have set called.
            // -> default init everything? is std::optional<> the best choice for this?
            fColor->set(color, fAttribs & PatchAttribs::kWideColorIfEnabled);
        } else {
            fColor = color.toBytes_RGBA();
        }
    }

    /**
     * writeX functions for supported patch geometry types. Every geometric type is converted to an
     * equivalent cubic or conic, so this will always write at minimum 8 floats for the four control
     * points (cubic) or three control points and {w, inf} (conics). The PatchWriter additionally
     * writes the current values of all attributes enabled in its PatchAttribs flags.
     */

    // Writes four control points manually prepared.
    // NOTE: Can be removed if/when StrokeHardwareTessellator is deleted.
    // TODO: could just make writePatch a protected function since HwPatchWriter extends it
    template <typename Void = void>
    AI std::enable_if_t<has_feature<WriteRawPatches>::value, Void>
    writeRawPatch(const SkPoint p[4]) {
        float4 p0p1 = float4::Load(p);
        float4 p2p3 = float4::Load(p + 2);
        this->writePatch(p0p1.lo, p0p1.hi, p2p3.lo, p2p3.hi, /*unused*/0.f);
    }

    // Write a cubic curve with its four control points.
    AI void writeCubic(float2 p0, float2 p1, float2 p2, float2 p3,
                       const VectorXform& shaderXform,
                       float precision = kTessellationPrecision) {
        float n4 = wangs_formula::cubic_pow4(precision, p0, p1, p2, p3, shaderXform);
        // if constexpr (kDiscardFlatCurves) {
        if constexpr (has_feature<DiscardFlatCurves>::value) {
            if (n4 <= 1.f) {
                // This cubic only needs one segment (e.g. a line) but we're not filling space with
                // fans or stroking, so nothing actually needs to be drawn.
                return;
            }
        }
        if (this->curveFitsInMaxSegments(n4)) {
            this->writeCubicPatch(p0, p1, p2, p3);
        } else {
            int numPatches = SkScalarCeilToInt(wangs_formula::root4(
                    std::min(n4, pow4(kMaxTessellationSegmentsPerCurve)) / fMaxSegments_pow4));
            this->chopAndWriteCubics(p0, p1, p2, p3, numPatches);
        }
    }
    AI void writeCubic(const SkPoint pts[4],
                       const VectorXform& shaderXform,
                       float precision = kTessellationPrecision) {
        float4 p0p1 = float4::Load(pts);
        float4 p2p3 = float4::Load(pts + 2);
        this->writeCubic(p0p1.lo, p0p1.hi, p2p3.lo, p2p3.hi, shaderXform, precision);
    }

    // Write a conic curve with three control points and 'w', with the last coord of the last
    // control point signaling a conic by being set to infinity.
    AI void writeConic(float2 p0, float2 p1, float2 p2, float w,
                       const VectorXform& shaderXform,
                       float precision = kTessellationPrecision) {
        float n2 = wangs_formula::conic_pow2(precision, p0, p1, p2, w, shaderXform);
        if constexpr (kDiscardFlatCurves) {
            if (n2 <= 1.f) {
                // This conic only needs one segment (e.g. a line) but we're not filling space with
                // fans or stroking, so nothing actually needs to be drawn.
                return;
            }
        }
        if (this->curveFitsInMaxSegments(n2*n2)) {
            this->writeConicPatch(p0, p1, p2, w);
        } else {
            int numPatches = SkScalarCeilToInt(sqrtf(
                    std::min(n2, pow2(kMaxTessellationSegmentsPerCurve)) / fMaxSegments_pow2));
            this->chopAndWriteConics(p0, p1, p2, w, numPatches);
        }
    }
    AI void writeConic(const SkPoint pts[3], float w,
                       const VectorXform& shaderXform,
                       float precision = kTessellationPrecision) {
        this->writeConic(skvx::bit_pun<float2>(pts[0]),
                         skvx::bit_pun<float2>(pts[1]),
                         skvx::bit_pun<float2>(pts[2]),
                         w, shaderXform, precision);
    }

    // Write a quadratic curve that automatically converts its three control points into an
    // equivalent cubic.
    AI void writeQuadratic(float2 p0, float2 p1, float2 p2,
                           const VectorXform& shaderXform,
                           float precision = kTessellationPrecision) {
        float n4 = wangs_formula::quadratic_pow4(precision, p0, p1, p2, shaderXform);
        if constexpr (kDiscardFlatCurves) {
            if (n4 <= 1.f) {
                // This quad only needs one segment (e.g. a line) but we're not filling space with
                // fans or stroking, so nothing actually needs to be drawn.
                return;
            }
        }
        if (this->curveFitsInMaxSegments(n4)) {
            this->writeQuadPatch(p0, p1, p2);
        } else {
            int numPatches = SkScalarCeilToInt(wangs_formula::root4(
                    std::min(n4, pow4(kMaxTessellationSegmentsPerCurve)) / fMaxSegments_pow4));
            this->chopAndWriteQuads(p0, p1, p2, numPatches);
        }
    }
    AI void writeQuadratic(const SkPoint pts[3],
                           const VectorXform& shaderXform,
                           float precision = kTessellationPrecision) {
        this->writeQuadratic(skvx::bit_pun<float2>(pts[0]),
                             skvx::bit_pun<float2>(pts[1]),
                             skvx::bit_pun<float2>(pts[2]),
                             shaderXform, precision);
    }

    // Write a line that is automatically converted into an equivalent cubic.
    AI void writeLine(float4 p0p1) {
        // No chopping needed, minimum segments is always at least 1
        this->writeCubicPatch(p0p1.lo, (p0p1.zwxy() - p0p1) * (1/3.f) + p0p1, p0p1.hi);
    }
    AI void writeLine(float2 p0, float2 p1) { this->writeLine({p0, p1}); }
    AI void writeLine(SkPoint p0, SkPoint p1) {
        this->writeLine(skvx::bit_pun<float2>(p0), skvx::bit_pun<float2>(p1));
    }

    // Write a triangle by setting it to a conic with w=Inf, and using a distinct
    // explicit curve type for when inf isn't supported in shaders.
    AI void writeTriangle(float2 p0, float2 p1, float2 p2) {
        // No chopping needed, the max supported segment count should always support 2 lines
        // (which form a triangle when implicitly closed).
        SkAssertResult(this->curveFitsInMaxSegments(2.f * 2.f * 2.f * 2.f));
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
    template <typename Void = void>
    AI std::enable_if_t<kEnabled & PatchAttribs::kJoinControlPoint, Void>
    writeCircle(SkPoint p) {
        // This does not use writePatch() because it uses its own location as the join attribute
        // value instead of fJoin and never defers.
        if (VertexWriter vw = fChunker.appendVertex()) {
            vw << VertexWriter::Repeat<4>(p); // p0,p1,p2,p3 = p -> 4 copies
            this->emitPatchAttribs(std::move(vw), p, kCubicCurveType);
        }
    }

private:
    AI void emitPatchAttribs(VertexWriter vertexWriter,
                             const JoinAttrib& join,
                             float explicitCurveType) {
        // NOTE: operator<< overrides automatically handle optional and disabled attribs.
        vertexWriter << join << fFanPoint << fStrokeParams << fColor;
        if constexpr (kRequired & PatchAttribs::kExplicitCurveType) {
            vertexWriter << explicitCurveType;
        } else if constexpr (kEnabled & PatchAttribs::kExplicitCurveType) {
            vertexWriter << VertexWriter::If(fAttribs & PatchAttribs::kExplicitCurveType,
                                             explicitCurveType);
        } // else disabled so don't write the type out
    }

    AI VertexWriter appendPatch() {
        if constexpr (kDeferPatches) {
            if (fDeferredPatch.fMustDefer) {
                SkASSERT(!fDeferredPatch.fHasPending);
                SkASSERT(fChunker.stride() <= DeferredPatch::kMaxStride);
                fDeferredPatch.fHasPending = true;
                return {fDeferredPatch.fData, fChunker.stride()};
            }
        }
        return fChunker.appendVertex();
    }

    AI void writePatch(float2 p0, float2 p1, float2 p2, float2 p3, float explicitCurveType) {
        if (VertexWriter vw = this->appendPatch()) {
            vw << p0 << p1 << p2 << p3;
            // NOTE: fJoin will be undefined if we're writing to a deferred patch. If that's the
            // case, correct data will overwrite it when the contour is closed (this is fine since a
            // deferred patch writes to CPU memory instead of directly to the GPU buffer).
            this->emitPatchAttribs(std::move(vw), fJoin, explicitCurveType);

            // Automatically update join control point for next patch.
            if constexpr (kUpdateJoin) {
                if (explicitCurveType == kCubicCurveType && any(p3 != p2)) {
                    // p2 is control point defining the tangent vector into the next patch.
                    p2.store(&fJoin);
                } else if (any(p2 != p1)) {
                    // p1 is the control point defining the tangent vector.
                    p1.store(&fJoin);
                } else {
                    // p0 is the control point defining the tangent vector.
                    p0.store(&fJoin);
                }
                if constexpr (kDeferPatches) {
                    fDeferredPatch.fMustDefer = false;
                }
            }
        }
    }
    // Helpers that normalize curves to a generic patch, but does no other work.
    AI void writeCubicPatch(float2 p0, float2 p1, float2 p2, float2 p3) {
        this->writePatch(p0, p1, p2, p3, kCubicCurveType);
    }
    AI void writeCubicPatch(float2 p0, float4 p1p2, float2 p3) {
        this->writeCubicPatch(p0, p1p2.lo, p1p2.hi, p3);
    }
    AI void writeQuadPatch(float2 p0, float2 p1, float2 p2) {
        this->writeCubicPatch(p0, mix(float4(p0, p2), p1.xyxy(), 2/3.f), p2);
    }
    AI void writeConicPatch(float2 p0, float2 p1, float2 p2, float w) {
        this->writePatch(p0, p1, p2, {w, SK_FloatInfinity}, kConicCurveType);
    }

    // Returns true if curve can be written w/o needing to chop (e.g. represented by one instance)
    bool curveFitsInMaxSegments(float n4) {
        if (n4 <= fMaxSegments_pow4) {
            fCurrMinSegments_pow4 = std::max(n4, fCurrMinSegments_pow4);
            return true;
        } else {
            fCurrMinSegments_pow4 = fMaxSegments_pow4;
            return false;
        }
    }

    // Helpers that chop the curve type into 'numPatches' parametrically uniform curves. It is
    // assumed that 'numPatches' is calculated such that the resulting curves require the maximum
    // number of segments to draw appropriately (since the original presumably needed even more).
    void chopAndWriteQuads(float2 p0, float2 p1, float2 p2, int numPatches) {
        InnerTriangulator innerTriangulator(numPatches, skvx::bit_pun<SkPoint>(p0));
        for (; numPatches >= 3; numPatches -= 2) {
            // Chop into 3 quads.
            float4 T = float4(1,1,2,2) / numPatches;
            float4 ab = mix(p0.xyxy(), p1.xyxy(), T);
            float4 bc = mix(p1.xyxy(), p2.xyxy(), T);
            float4 abc = mix(ab, bc, T);
            // p1 & p2 of the cubic representation of the middle quad.
            float4 middle = mix(ab, bc, mix(T, T.zwxy(), 2/3.f));

            this->writeQuadPatch(p0, ab.lo, abc.lo);  // Write the 1st quad.
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangle(p0, abc.lo, abc.hi);
            }
            this->writeCubicPatch(abc.lo, middle, abc.hi);  // Write the 2nd quad (already a cubic)
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangleStack(innerTriangulator.pushVertex(skvx::bit_pun<SkPoint>(abc.hi)));
            }
            std::tie(p0, p1) = {abc.hi, bc.hi};  // Save the 3rd quad.
        }
        if (numPatches == 2) {
            // Chop into 2 quads.
            float2 ab = (p0 + p1) * .5f;
            float2 bc = (p1 + p2) * .5f;
            float2 abc = (ab + bc) * .5f;

            this->writeQuadPatch(p0, ab, abc);  // Write the 1st quad.
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangle(p0, abc, p2);
            }
            this->writeQuadPatch(abc, bc, p2);  // Write the 2nd quad.
        } else {
            SkASSERT(numPatches == 1);
            this->writeQuadPatch(p0, p1, p2);  // Write the single remaining quad.
        }
        if constexpr (kAddTrianglesWhenChopping) {
            this->writeTriangleStack(innerTriangulator.pushVertex(skvx::bit_pun<SkPoint>(p2)));
            this->writeTriangleStack(innerTriangulator.close());
        }
    }

    void chopAndWriteConics(float2 p0, float2 p1, float2 p2, float w, int numPatches) {
        InnerTriangulator innerTriangulator(numPatches, skvx::bit_pun<SkPoint>(p0));
        // Load the conic in 3d homogeneous (unprojected) space.
        float4 h0 = float4(p0,1,1);
        float4 h1 = float4(p1,1,1) * w;
        float4 h2 = float4(p2,1,1);
        for (; numPatches >= 2; --numPatches) {
            // Chop in homogeneous space.
            float T = 1.f/numPatches;
            float4 ab = mix(h0, h1, T);
            float4 bc = mix(h1, h2, T);
            float4 abc = mix(ab, bc, T);

            // Project and write the 1st conic.
            float2 midpoint = abc.xy() / abc.w();
            this->writeConicPatch(h0.xy() / h0.w(),
                                  ab.xy() / ab.w(),
                                  midpoint,
                                  ab.w() / sqrtf(h0.w() * abc.w()));
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangleStack(innerTriangulator.pushVertex(skvx::bit_pun<SkPoint>(midpoint)));
            }
            std::tie(h0, h1) = {abc, bc};  // Save the 2nd conic (in homogeneous space).
        }
        // Project and write the remaining conic.
        SkASSERT(numPatches == 1);
        this->writeConicPatch(h0.xy() / h0.w(),
                              h1.xy() / h1.w(),
                              h2.xy(), // h2.w == 1
                              h1.w() / sqrtf(h0.w()));
        if constexpr (kAddTrianglesWhenChopping) {
            this->writeTriangleStack(innerTriangulator.pushVertex(skvx::bit_pun<SkPoint>(h2.xy())));
            this->writeTriangleStack(innerTriangulator.close());
        }
    }

    void chopAndWriteCubics(float2 p0, float2 p1, float2 p2, float2 p3, int numPatches) {
        InnerTriangulator innerTriangulator(numPatches, skvx::bit_pun<SkPoint>(p0));
        for (; numPatches >= 3; numPatches -= 2) {
            // Chop into 3 cubics.
            float4 T = float4(1,1,2,2) / numPatches;
            float4 ab = mix(p0.xyxy(), p1.xyxy(), T);
            float4 bc = mix(p1.xyxy(), p2.xyxy(), T);
            float4 cd = mix(p2.xyxy(), p3.xyxy(), T);
            float4 abc = mix(ab, bc, T);
            float4 bcd = mix(bc, cd, T);
            float4 abcd = mix(abc, bcd, T);
            float4 middle = mix(abc, bcd, T.zwxy());  // p1 & p2 of the middle cubic.

            this->writeCubicPatch(p0, ab.lo, abc.lo, abcd.lo);  // Write the 1st cubic.
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangle(p0, abcd.lo, abcd.hi);
            }
            this->writeCubicPatch(abcd.lo, middle, abcd.hi);  // Write the 2nd cubic.
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangleStack(innerTriangulator.pushVertex(skvx::bit_pun<SkPoint>(abcd.hi)));
            }
            std::tie(p0, p1, p2) = {abcd.hi, bcd.hi, cd.hi};  // Save the 3rd cubic.
        }
        if (numPatches == 2) {
            // Chop into 2 cubics.
            float2 ab = (p0 + p1) * .5f;
            float2 bc = (p1 + p2) * .5f;
            float2 cd = (p2 + p3) * .5f;
            float2 abc = (ab + bc) * .5f;
            float2 bcd = (bc + cd) * .5f;
            float2 abcd = (abc + bcd) * .5f;

            this->writeCubicPatch(p0, ab, abc, abcd);  // Write the 1st cubic.
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangle(p0, abcd, p3);
            }
            this->writeCubicPatch(abcd, bcd, cd, p3);  // Write the 2nd cubic.
        } else {
            SkASSERT(numPatches == 1);
            this->writeCubicPatch(p0, p1, p2, p3);  // Write the single remaining cubic.
        }
        if constexpr (kAddTrianglesWhenChopping) {
            this->writeTriangleStack(innerTriangulator.pushVertex(skvx::bit_pun<SkPoint>(p3)));
            this->writeTriangleStack(innerTriangulator.close());
        }
    }

    template <typename Void = void>
    std::enable_if_t<kAddTrianglesWhenChopping, Void>
    writeTriangleStack(MiddleOutPolygonTriangulator::PoppedTriangleStack&& stack) {
        for (auto [p0, p1, p2] : stack) {
            this->writeTriangle(p0, p1, p2);
        }
    }

    // Runtime configuration, will always contain kRequired attribs but may not have all optional
    // attribs enabled (e.g. depending on caps or batching).
    const PatchAttribs fAttribs;

    const float fMaxSegments_pow2;
    const float fMaxSegments_pow4;
    float fCurrMinSegments_pow4;

    GrVertexChunkBuilder fChunker;
    DeferredPatch fDeferredPatch; // only usable if kDeferPatches is true

    // Instance attribute state written after the 4 control points of a patch
    JoinAttrib fJoin;
    FanPointAttrib    fFanPoint;
    StrokeAttrib      fStrokeParams;
    ColorAttrib       fColor;
};

}  // namespace skgpu

#undef AI

#endif  // tessellate_PatchWriter_DEFINED
