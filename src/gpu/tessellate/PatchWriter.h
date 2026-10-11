/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_tessellate_PatchWriter_DEFINED
#define skgpu_tessellate_PatchWriter_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/SkDebug.h"
#include "include/private/SkFloatingPoint.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkColorData.h"
#include "src/core/SkUtils.h"
#include "src/core/SkVx.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/tessellate/ArcLengthCalculator.h"
#include "src/gpu/tessellate/LinearTolerances.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/Tessellation.h"
#include "src/gpu/tessellate/WangsFormula.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <math.h>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace skgpu::tess {

/**
 * PatchWriter writes out tessellation patches, formatted with their specific attribs, to a GPU
 * buffer.
 *
 * PatchWriter is a template class that takes traits to configure both its compile-time and runtime
 * behavior for the different tessellation rendering algorithms and GPU backends. The complexity of
 * this system is worthwhile because the attribute writing operations and math already require
 * heavy inlining for performance, and the algorithmic variations tend to only differ slightly, but
 * do so in the inner most loops. Additionally, Graphite and Ganesh use the same fundamental
 * algorithms, but Graphite's architecture and higher required hardware level mean that its
 * attribute configurations can be determined entirely at compile time.
 *
 * Traits are specified in PatchWriter's single var-args template pack. Traits come in two main
 * categories: PatchAttribs configuration and feature/processing configuration. A given PatchAttrib
 * can be always enabled, enabled at runtime, or always disabled. A feature can be either enabled
 * or disabled and are coupled more closely with the control points of the curve. Across the two
 * GPU backends and different path rendering strategies, a "patch" has the following structure:
 *
 *   - 4 control points (8 floats total) defining the curve's geometry
 *      - quadratic curves are converted to equivalent cubics on the CPU during writing
 *      - conic curves store {w, inf} in their last control point
 *      - triangles store {inf, inf} in their last control point
 *      - everything else is presumed to be a cubic defined by all 4 control points
 *   - Enabled PatchAttrib values, constant for the entire instance
 *      - layout is identical to PatchAttrib's definition, skipping disabled attribs
 *      - attribs can be enabled/disabled at runtime by building a mask of attrib values
 *
 * Currently PatchWriter supports the following traits:
 *   - Required<PatchAttrib>
 *   - Optional<PatchAttrib>
 *   - TrackJoinControlPoints
 *   - AddTrianglesWhenChopping
 *   - DiscardFlatCurves
 *
 * In addition to variable traits, PatchWriter's first template argument defines the type used for
 * allocating the GPU instance data. The templated "PatchAllocator" can be any type that provides:
 *    // A GPU-backed vertex writer for a single instance worth of data. The provided
 *    // LinearTolerances value represents the tolerances for the curve that will be written to the
 *    // returned vertex space.
 *    skgpu::VertexWriter append(const LinearTolerances&);
 *
 * Additionally, it must have a constructor that takes the stride as its first argument.
 * PatchWriter forwards any additional constructor args from its ctor to the allocator after
 * computing the necessary stride for its PatchAttribs configuration.
 */

// *** TRAITS ***

// Marks a PatchAttrib is enabled at compile time, i.e. it must always be set and will always be
// written to each patch's instance data. If present, will assert if the runtime attribs do not fit.
template <PatchAttribs A> struct Required {};
// Marks a PatchAttrib as supported, i.e. it can be enabled or disabled at runtime. Optional<A> is
// overridden by Required<A>. If neither Required<A> nor Optional<A> are in a PatchWriter's trait
// list, then the attrib is disabled at compile time and it will assert if the runtime attribs
// attempt to enable it.
template <PatchAttribs A> struct Optional {};

// Enables tracking of the kJoinControlPointAttrib based on control points of the previously
// written patch (automatically taking into account curve chopping). When a patch is first written
// (and there is no prior patch to define the join control point), the PatchWriter automatically
// records the patch to a temporary buffer--sans join--until writeDeferredStrokePatch() is called,
// filling in the now-defined join control point.
//
// This feature must be paired with Required<PatchAttribs::kJoinControlPoint>
struct TrackJoinControlPoints {};

// Write additional triangular patches to fill the resulting empty area when a curve is chopped.
// Normally, the patch geometry covers the curve defined by its control points, up to the implicitly
// closing edge between its first and last control points. When a curve is chopped to fit within
// the maximum segment count, the resulting space between the original closing edge and new closing
// edges is not filled, unless some mechanism of the shader makes it so (e.g. a fan point or
// stroking).
//
// This feature enables automatically writing triangular patches to fill this empty space when a
// curve is chopped.
struct AddTrianglesWhenChopping {};

// If a curve requires at most 1 segment to render accurately, it's effectively a straight line.
// This feature turns on automatically ignoring those curves, with the assumption that some other
// render pass will produce equivalent geometry (e.g. middle-out or inner triangulations).
struct DiscardFlatCurves {};

// Upload lines as a cubic with {a, a, b, b} for control points, instead of the truly linear cubic
// of {a, 2/3a + 1/3b, 1/3a + 2/3b, b}. Wang's formula will not return an tight lower bound on the
// number of segments in this case, but it's convenient to detect in the vertex shader and assume
// only a single segment is required. This bypasses numerical stability issues in Wang's formula
// when evaluated on the ideal linear cubic for very large control point coordinates. Other curve
// types with large coordinates do not need this treatment since they would be pre-chopped and
// culled to lines.
struct ReplicateLineEndPoints {};

// *** PatchWriter internals ***

// AttribValue exposes a consistent store and write interface for a PatchAttrib's value while
// abstracting over compile-time enabled, conditionally-enabled, or compile-time disabled attribs.
template <PatchAttribs A, typename T, bool Required, bool Optional>
struct AttribValue {
    using DataType = std::conditional_t<Required, T,
                     std::conditional_t<Optional, std::pair<T, bool>,
                                       /* else */ std::monostate>>;

    static constexpr bool kEnabled = Required || Optional;

    explicit AttribValue(PatchAttribs attribs) : AttribValue(attribs, {}) {}
    AttribValue(PatchAttribs attribs, const T& t) {
        (void) attribs; // may be unused on release builds
        if constexpr (Required) {
            SkASSERT(attribs & A);
        } else if constexpr (Optional) {
            std::get<1>(fV) = attribs & A;
        } else {
            SkASSERT(!(attribs & A));
        }
        *this = t;
    }

    AttribValue& operator=(const T& v) {
        if constexpr (Required) {
            fV = v;
        } else if constexpr (Optional) {
            // for simplicity, store even if disabled and won't be written out to VertexWriter
            std::get<0>(fV) = v;
        } // else ignore for disabled values
        return *this;
    }

    const T& operator*() const {
        if constexpr (Required) {
            return fV;
        } else if constexpr (Optional) {
            SkASSERT(std::get<1>(fV));
            return std::get<0>(fV);
        } else {
            SkUNREACHABLE;
        }
    }

    DataType fV;
};

template <PatchAttribs A, typename T, bool Required, bool Optional>
VertexWriter& operator<<(VertexWriter& w, const AttribValue<A, T, Required, Optional>& v) {
    if constexpr (Required) {
        w << v.fV; // always write
    } else if constexpr (Optional) {
        if (std::get<1>(v.fV)) {
            w << std::get<0>(v.fV); // write if enabled
        }
    } // else never write
    return w;
}

// Stores state and deferred patch data when TrackJoinControlPoints is used for a PatchWriter.
template <size_t Stride>
struct PatchStorage {
    SkPoint fFirstControlPoint; // This is the first control point of the contour, e.g. moveTo
    SkPoint fLastControlPoint;  // The last control point written within the contour
    float   fStartArcLength = 0.f; // The arclength at the start of the contour

    // The signs of fCurveType and fN_p4 encode some special states:
    //  - If both are negative, then nothing has been written that needs to be deferred
    //  - If fN_p4 == 0, the caller has explicitly managed the join and nothing is deferred
    //  - If fN_p4 < 0 and fCurveType >= 0, then a degenerate verb has been recorded so caps should
    //    be written when the deferred patch is ended.
    //  - If fN_p4 > 0 and fCurveType >= 0, then this is a full deferred patch
    float fCurveType = -1.f;  // The explicit curve type passed to writePatch()
    float fN_p4      = -1.f;  // The parametric segment value to restore on LinearTolerances

    // Holds an entire patch, except with an undefined join control point.
    char fData[Stride];

    void disableDeferral() {
        fN_p4 = std::max(fN_p4, 0.f); // do nothing if we already have a deferred patch recorded
    }

    bool mustDefer() const {
        return fN_p4 < 0.f;
    }

    bool hasVerb() const {
        return fCurveType >= 0.f;
    }

    bool hasPending() const {
        // fN_p4 = 0 shouldn't happen naturally from Wang's formula and is used to disable deferring
        // patches if the join is defined manually via updateJoinControlPointAttrib()
        return fN_p4 > 0.f;
    }

    void reset(SkPoint moveTo, float startArcLength) {
        fCurveType = -1.f;
        fN_p4 = -1.f;
        fFirstControlPoint = fLastControlPoint = moveTo;
        fStartArcLength = startArcLength;
    }
};

// An empty object that has the same constructor signature as MiddleOutPolygonTriangulator, used
// as a stand-in when AddTrianglesWhenChopping is not a defined trait.
struct NullTriangulator {
    NullTriangulator(int, SkPoint) {}
};

#define AI SK_ALWAYS_INLINE
#define ENABLE_IF(cond) template <typename Void=void> std::enable_if_t<cond, Void>

// *** PatchWriter ***
template <typename PatchAllocator, typename... Traits>
class PatchWriter {
    // Helpers to extract specifics from the template traits pack.
    template <typename F>     struct has_trait  : std::disjunction<std::is_same<F, Traits>...> {};
    template <PatchAttribs A> using  req_attrib = has_trait<Required<A>>;
    template <PatchAttribs A> using  opt_attrib = has_trait<Optional<A>>;

    // Enabled features and attribute configuration
    static constexpr bool kTrackJoinControlPoints   = has_trait<TrackJoinControlPoints>::value;
    static constexpr bool kAddTrianglesWhenChopping = has_trait<AddTrianglesWhenChopping>::value;
    static constexpr bool kDiscardFlatCurves        = has_trait<DiscardFlatCurves>::value;
    static constexpr bool kReplicateLineEndPoints   = has_trait<ReplicateLineEndPoints>::value;

    // NOTE: MSVC 19.24 cannot compile constexpr fold expressions referenced in templates, so
    // extract everything into constexpr bool's instead of using `req_attrib` directly, etc. :(
    template <PatchAttribs A, typename T, bool Req/*=req_attrib<A>*/, bool Opt/*=opt_attrib<A>*/>
    using attrib_t = AttribValue<A, T, Req, Opt>;

    // TODO: Remove when MSVC compiler is fixed, in favor of `using Name = attrib_t<>` directly.
#define DEF_ATTRIB_TYPE(name, A, T) \
    static constexpr bool kRequire##name = req_attrib<A>::value; \
    static constexpr bool kOptional##name = opt_attrib<A>::value; \
    using name = attrib_t<A, T, kRequire##name, kOptional##name>

    DEF_ATTRIB_TYPE(JoinAttrib,      PatchAttribs::kJoinControlPoint,  SkPoint);
    DEF_ATTRIB_TYPE(FanPointAttrib,  PatchAttribs::kFanPoint,          SkPoint);
    DEF_ATTRIB_TYPE(StrokeAttrib,    PatchAttribs::kStrokeParams,      StrokeParams);

    // kWideColorIfEnabled does not define an attribute, but changes the type of the kColor attrib.
    static constexpr bool kRequireWideColor  = req_attrib<PatchAttribs::kWideColorIfEnabled>::value;
    static constexpr bool kOptionalWideColor = opt_attrib<PatchAttribs::kWideColorIfEnabled>::value;
    using Color = std::conditional_t<kRequireWideColor,  SkPMColor4f,
                  std::conditional_t<kOptionalWideColor, VertexColor,
                                              /* else */ uint32_t>>;

    DEF_ATTRIB_TYPE(ColorAttrib,     PatchAttribs::kColor,             Color);
    DEF_ATTRIB_TYPE(DepthAttrib,     PatchAttribs::kPaintDepth,        float);
    DEF_ATTRIB_TYPE(CurveTypeAttrib, PatchAttribs::kExplicitCurveType, float);
    DEF_ATTRIB_TYPE(SsboIndexAttrib, PatchAttribs::kSsboIndex,         uint32_t);
    DEF_ATTRIB_TYPE(VertexStorageIndexAttrib, PatchAttribs::kVertexStorageIndex, uint32_t);
    // Arc length parameterization of a patch (kArcLength), see ArcLengthAccumulator. The patch's
    // preceding join covers [fStart, fStart + fJoinLength] and its curve covers the next
    // fCurveLength, except where the joins at either end extend into it. Those joins have corner
    // radii fStartRadius and fEndRadius (0 if they have no corner), and the vertex shader works out
    // how far they extend from the radii and the tangents on either side of them. The tangent that
    // leaves the end join points towards fNextPoint. fNextPoint and fEndRadius are only known once
    // the next patch is written (see completeArcLengthPatch()).
    //
    // A patch inside a fold (see ArcLengthFold) describes the fold instead, which parameterizes all
    // of it. That's marked by a negative fStartRadius:
    //   - fStart is where the fold's sweep starts, and fJoinLength is the sweep's length.
    //   - fCurveLength is the angle the fold turns by, from fA to fB (signed, like their cross
    //     product).
    //   - fNextPoint is its corner C, fStartRadius is -(its corner radius), and fEndRadius is the
    //     angle of fA (from the +x axis towards +y).
    struct ArcLengthData {
        float fStart;
        float fJoinLength;
        float fCurveLength;
        float fTotalLength;  // Only read by shaders that can't read it from vertex storage
        SkPoint fNextPoint;
        float fStartRadius;
        float fEndRadius;
    };
    DEF_ATTRIB_TYPE(ArcLengthAttrib, PatchAttribs::kArcLength,         ArcLengthData);
#undef DEF_ATTRIB_TYPE

    static constexpr size_t kMaxStride = 4 * sizeof(SkPoint) + // control points
            (JoinAttrib::kEnabled      ? sizeof(SkPoint)                              : 0) +
            (FanPointAttrib::kEnabled  ? sizeof(SkPoint)                              : 0) +
            (StrokeAttrib::kEnabled    ? sizeof(StrokeParams)                         : 0) +
            (ColorAttrib::kEnabled     ? std::min(sizeof(Color), sizeof(SkPMColor4f)) : 0) +
            (DepthAttrib::kEnabled     ? sizeof(float)                                : 0) +
            (CurveTypeAttrib::kEnabled ? sizeof(float)                                : 0) +
            (SsboIndexAttrib::kEnabled ? 2 * sizeof(uint32_t)                         : 0) +
            (VertexStorageIndexAttrib::kEnabled ? sizeof(uint32_t)                    : 0) +
            (ArcLengthAttrib::kEnabled ? sizeof(ArcLengthData)                        : 0);

    // Types that vary depending on the activated features, but do not define the patch data.
    using DeferredPatch = std::conditional_t<kTrackJoinControlPoints,
            PatchStorage<kMaxStride>, std::monostate>;
    using InnerTriangulator = std::conditional_t<kAddTrianglesWhenChopping,
            MiddleOutPolygonTriangulator, NullTriangulator>;

    // State for writing kArcLength, only present if the attrib can be enabled.
    struct ArcLengthState {
        // Verb-level accumulation, shared with ArcLengthPrecalculator so totals match exactly.
        ArcLengthAccumulator fAccumulator;

        // The contour's most recent verb, held back until the accumulator finishes it (see
        // holdArcLengthVerb()).
        enum class VerbType { kNone, kLine, kQuad, kConic, kCubic };
        struct Verb {
            VerbType fType = VerbType::kNone;
            skvx::float2 fPts[4];
            float fW = 1.f;

            skvx::float2 end() const {
                return fPts[fType == VerbType::kLine ? 1 : (fType == VerbType::kCubic ? 3 : 2)];
            }
        };
        Verb fHeldVerb;
        // The curve type of the last degenerate verb written after fHeldVerb, or -1 if none was.
        float fTrailingCurveType = -1.f;

        // The verb currently being written, which chopping may split into several patches.
        ArcLengthAccumulator::Span fVerb = {};
        float fPieceStart = 0.f;  // Arc length at the start of fVerb's next patch
        bool fFirstPiece = true;  // Whether fVerb's next patch is its first (which owns the join)
        float fTotalPathLength = 0.f;

        // While writing the pieces of a curve inside a fold (see beginFold()), every patch gets
        // the fold's arc length data instead of its own.
        bool fInFold = false;
        ArcLengthData fFold = {};

        // Which patch was written most recently, if its end join still has to be filled in (see
        // completeArcLengthPatch()): the deferred first patch of the contour, or fHeldPatch, which
        // is held back from the PatchAllocator until then.
        enum class PrevPatch { kNone, kDeferred, kHeld };
        PrevPatch fPrevPatch = PrevPatch::kNone;
        float fHeldPatchN_p4 = 0.f;  // The tolerance fHeldPatch was written with
        char fHeldPatch[kMaxStride];
    };
    using ArcState = std::conditional_t<ArcLengthAttrib::kEnabled, ArcLengthState, std::monostate>;

    using float2 = skvx::float2;
    using float4 = skvx::float4;

    static_assert(!kTrackJoinControlPoints || req_attrib<PatchAttribs::kJoinControlPoint>::value,
                  "Deferred patches and auto-updating joins requires kJoinControlPoint attrib");
    static_assert(!ArcLengthAttrib::kEnabled || kTrackJoinControlPoints,
                  "kArcLength relies on TrackJoinControlPoints to find the ends of contours");
public:
    template <typename... Args> // forwarded to PatchAllocator
    PatchWriter(PatchAttribs attribs,
                Args&&... allocArgs)
            : fAttribs(attribs)
            , fPatchAllocator(PatchStride(attribs), std::forward<Args>(allocArgs)...)
            , fJoin(attribs)
            , fFanPoint(attribs)
            , fStrokeParams(attribs)
            , fColor(attribs)
            , fDepth(attribs)
            , fSsboIndex(attribs)
            , fVertexStorageIndex(attribs)
            , fArcLength(attribs) {
        // Explicit curve types are provided on the writePatch signature, and not a field of
        // PatchWriter, so initialize one in the ctor to validate the provided runtime attribs.
        SkDEBUGCODE((void) CurveTypeAttrib(attribs);)
        // Validate the kWideColorIfEnabled attribute variant flag as well
        if constexpr (req_attrib<PatchAttribs::kWideColorIfEnabled>::value) {
            SkASSERT(attribs & PatchAttribs::kWideColorIfEnabled);    // required
        } else if constexpr (!opt_attrib<PatchAttribs::kWideColorIfEnabled>::value) {
            SkASSERT(!(attribs & PatchAttribs::kWideColorIfEnabled)); // disabled
        }
    }

    ~PatchWriter() {
        if constexpr (kTrackJoinControlPoints) {
            // There shouldn't be any deferred flush; PatchWriter doesn't know the cap style that
            // needs to be applied. Callers are responsible for calling writeDeferredStrokePatch()
            SkASSERT(!fDeferredPatch.hasPending());
        }
        if constexpr (ArcLengthAttrib::kEnabled) {
            // writeDeferredStrokePatch() also writes out any held verb or patch.
            SkASSERT(fArc.fHeldVerb.fType == ArcLengthState::VerbType::kNone);
            SkASSERT(fArc.fPrevPatch != ArcLengthState::PrevPatch::kHeld);
            SkASSERT(!fArc.fInFold);
        }
    }

    PatchAttribs attribs() const { return fAttribs; }

    // Returns the arc length of every verb written so far (0 unless kArcLength is supported). Each
    // verb is held back until the next one (see holdArcLengthVerb()), so this is only the total
    // once writeDeferredStrokePatch() has ended the last contour.
    float accumulatedLength() const {
        if constexpr (ArcLengthAttrib::kEnabled) {
            return fArc.fAccumulator.accumulated();
        } else {
            return 0.f;
        }
    }

    // Sets the total arc length written with each patch, for shaders that can't read it from
    // vertex storage. It must be known before writing any patches.
    void setTotalPathLength([[maybe_unused]] float totalPathLength) {
        if constexpr (ArcLengthAttrib::kEnabled) {
            fArc.fTotalPathLength = totalPathLength;
        }
    }

    // The max scale factor should be derived from the same matrix that 'xform' was. It's only used
    // in stroking calculations, so can be ignored for path filling.
    void setShaderTransform(const wangs_formula::VectorXform& xform,
                            float maxScale = 1.f) {
        fApproxTransform = xform;
        fMaxScale = maxScale;
    }

    // Completes a closed contour of a stroke by rewriting a deferred patch with now-available
    // join control point information. Automatically resets the join control point attribute.
    // If `cap` is provided, instead of joining the last patch to the deferred patch, it ends the
    // contour by adding caps to the end of the last and start of the deferred.
    //
    // `moveTo` represents the start of the next contour.
    ENABLE_IF(kTrackJoinControlPoints) writeDeferredStrokePatch(SkPoint moveTo,
                                                                std::optional<SkPaint::Cap> cap) {
        if constexpr (ArcLengthAttrib::kEnabled) {
            // Nothing joins to the end of the contour's last verb: either a cap follows it, or the
            // closing join, which has no corner (see ArcLengthAccumulator::close()). So the
            // accumulator can finish that verb, and its last patch can be completed.
            this->writeHeldVerb(fArc.fAccumulator.endContour());
            this->completeArcLengthPatch();
        }
        if (fDeferredPatch.hasPending()) {
            SkASSERT(fDeferredPatch.hasVerb());
            SkPoint join;
            if (!cap.has_value()) {
                // When the contour is closed, there are no caps. We just have to update the join
                // attribute to reflect the last patch and write the deferred patch.
                join = *fJoin;
            } else {
                // When the contour is not closed, the last patch and the deferred patch both need
                // to have caps added to them. Set the join point to the first control point since
                // all caps either don't require joins or are oriented to join with that point.
                join = fDeferredPatch.fFirstControlPoint;

                // Caps add no arc length; each one keeps the arc length of the end it caps.
                if (cap == SkPaint::kRound_Cap) {
                    // Also add circles for the start and end (these don't join with anything)
                    this->writeCircleAt(fDeferredPatch.fLastControlPoint,
                                        this->arcLengthPosition());
                    this->writeCircleAt(fDeferredPatch.fFirstControlPoint,
                                        fDeferredPatch.fStartArcLength);
                    // Since these caps are full circles, there's no need to correct the deferred
                    // patch's join attribute. The circle will cover up the butt-end just fine.
                } else if (cap == SkPaint::kSquare_Cap) {
                    // First write a square cap that joins against the last recorded join point
                    this->writeSquareAt(fDeferredPatch.fLastControlPoint, *fJoin,
                                        this->arcLengthPosition());
                    // Next write a square cap that joins against the incoming tangent point of the
                    // first contour.
                    float4 p01 = float4::Load(fDeferredPatch.fData);
                    float4 p23 = float4::Load(fDeferredPatch.fData + 4*sizeof(float));
                    float2 last = fDeferredPatch.fCurveType == kCubicCurveType
                            ? p23.zw() : p23.xy();
                    float2 capPt = TangentPoint(p01.xy(), p01.zw(), p23.xy(), last);
                    this->writeSquareAt(SkPoint{p01[0], p01[1]}, SkPoint{capPt[0], capPt[1]},
                                        fDeferredPatch.fStartArcLength);
                }
            }

            // Overwrite join control point with the updated value, which is the first attribute
            // after the 4 control points.
            memcpy(SkTAddOffset<void>(fDeferredPatch.fData, 4 * sizeof(SkPoint)),
                   &join, sizeof(SkPoint));
            // Assuming that the stroke parameters aren't changing within a contour, we only have
            // to set the parametric segments in order to recover the LinearTolerances state at the
            // time the deferred patch was recorded.
            fTolerances.setParametricSegments(fDeferredPatch.fN_p4);
            if (VertexWriter vw = fPatchAllocator.append(fTolerances)) {
                vw << VertexWriter::Array<char>(fDeferredPatch.fData, PatchStride(fAttribs));
            }
        } else {
            // Either nothing has been recorded yet or it was all degenerate (in which case we
            // should record a cap).
            if (cap.has_value() && fDeferredPatch.hasVerb()) {
                switch (*cap) {
                    case SkPaint::kButt_Cap:
                        // No actual cap geometry to produce
                        break;
                    case SkPaint::kRound_Cap:
                        this->writeCircleAt(fDeferredPatch.fFirstControlPoint,
                                            this->arcLengthPosition());
                        break;
                    case SkPaint::kSquare_Cap: {
                        // Write a square with no joining direction to get a full square (vs. a cap)
                        this->writeSquareAt(fDeferredPatch.fFirstControlPoint, std::nullopt,
                                            this->arcLengthPosition());
                        break;
                    }
                }
                // Neither writeCircle() or writeSquare() should add anything that was deferred.
                SkASSERT(!fDeferredPatch.hasPending());
            }
        }

        // Remember the first control point in the event we need to produce a cap, or a close()
        fDeferredPatch.reset(moveTo, this->arcLengthPosition());
        if constexpr (ArcLengthAttrib::kEnabled) {
            fArc.fAccumulator.moveTo(sk_bit_cast<float2>(moveTo));
        }
    }

    ENABLE_IF(kTrackJoinControlPoints) closeDeferredStrokePatch(SkPaint::Cap cap) {
        SkPoint lastControlPoint = fDeferredPatch.fLastControlPoint;
        if constexpr (ArcLengthAttrib::kEnabled) {
            // The contour's last verb may still be held back, in which case its patches haven't
            // updated fLastControlPoint yet.
            if (fArc.fHeldVerb.fType != ArcLengthState::VerbType::kNone) {
                lastControlPoint = sk_bit_cast<SkPoint>(fArc.fHeldVerb.end());
            }
        }
        // Write a line from the last control point to the first control point, and then
        // write the deferred patch, which will use the join information generated by the lineTo
        this->writeLine(lastControlPoint, fDeferredPatch.fFirstControlPoint);
        if constexpr (ArcLengthAttrib::kEnabled) {
            // Also write out the line (or the verb before it, if the line is degenerate) so the
            // check below sees it. The closing join has no corner, so nothing joins to its end.
            this->writeHeldVerb(fArc.fAccumulator.endContour());
        }
        if (fDeferredPatch.hasPending()) {
            // This will be true if the prior writeLine() was not degenerate or there was other
            // non-degenerate patches written previously. In this case we should join, so don't pass
            // the cap style in.
            this->writeDeferredStrokePatch(fDeferredPatch.fFirstControlPoint, std::nullopt);
        } else {
            // Explicitly closing empty geometry will produce caps. This is handled by
            // writeDeferredStrokePatch so it can catch the equivalent case of
            // "moveTo(p0); lineTo(p0); moveTo(p1)" which never includes a close verb but also
            // produces cap geometry.
            SkASSERT(fDeferredPatch.hasVerb());
            this->writeDeferredStrokePatch(fDeferredPatch.fFirstControlPoint, cap);
        }
    }

    // DEPRECATED: Only used by Ganesh with StrokeIter that manages caps on its own. When used this
    // way, the moveTo point is always ignored and we only ever want to produce joins.
    ENABLE_IF(JoinAttrib::kEnabled) writeDeferredStrokePatch() {
        this->writeDeferredStrokePatch({0.f, 0.f}, std::nullopt);
    }
    // DEPRECATED
    ENABLE_IF(JoinAttrib::kEnabled) updateJoinControlPointAttrib(SkPoint lastControlPoint) {
        SkASSERT(fAttribs & PatchAttribs::kJoinControlPoint); // must be runtime enabled as well
        fJoin = lastControlPoint;
        fDeferredPatch.disableDeferral(); // fJoin is valid, so no need to defer next real patch
    }

    // Updates the fan point that will be written out with each patch (i.e., the point that wedges
    // fan around).
    ENABLE_IF(FanPointAttrib::kEnabled) updateFanPointAttrib(SkPoint fanPoint) {
        SkASSERT(fAttribs & PatchAttribs::kFanPoint);
        fFanPoint = fanPoint;
    }

    // Updates the stroke params that are written out with each patch.
    ENABLE_IF(StrokeAttrib::kEnabled) updateStrokeParamsAttrib(StrokeParams strokeParams) {
        SkASSERT(fAttribs & PatchAttribs::kStrokeParams);
        fStrokeParams = strokeParams;
        fTolerances.setStroke(strokeParams, fMaxScale);
        if constexpr (ArcLengthAttrib::kEnabled) {
            fArc.fAccumulator.setJoinRadius(ArcLengthAccumulator::JoinRadius(strokeParams));
        }
    }
    // Updates tolerances to account for stroke params that are stored as uniforms instead of
    // dynamic instance attributes.
    void updateUniformStrokeParams(StrokeParams strokeParams) {
        if constexpr (!StrokeAttrib::kEnabled) {
            fTolerances.setStroke(strokeParams, fMaxScale);
        }
        if constexpr (ArcLengthAttrib::kEnabled) {
            fArc.fAccumulator.setJoinRadius(ArcLengthAccumulator::JoinRadius(strokeParams));
        }
    }

    // Updates the color that will be written out with each patch.
    ENABLE_IF(ColorAttrib::kEnabled) updateColorAttrib(const SkPMColor4f& color) {
        SkASSERT(fAttribs & PatchAttribs::kColor);
        // Converts SkPMColor4f to the selected 'Color' attrib type. The always-wide and never-wide
        // branches match what VertexColor does based on the runtime check.
        if constexpr (req_attrib<PatchAttribs::kWideColorIfEnabled>::value) {
            fColor = color;
        } else if constexpr (opt_attrib<PatchAttribs::kWideColorIfEnabled>::value) {
            fColor = VertexColor(color, fAttribs & PatchAttribs::kWideColorIfEnabled);
        } else {
            fColor = color.toBytes_RGBA();
        }
    }

    // Updates the paint depth written out with each patch.
    ENABLE_IF(DepthAttrib::kEnabled) updatePaintDepthAttrib(float depth) {
        SkASSERT(fAttribs & PatchAttribs::kPaintDepth);
        fDepth = depth;
    }

    // Updates the vertex storage buffer index.
    ENABLE_IF(VertexStorageIndexAttrib::kEnabled)
    updateVertexStorageIndexAttrib(uint32_t vertexStorageIndex) {
        SkASSERT(fAttribs & PatchAttribs::kVertexStorageIndex);
        fVertexStorageIndex = vertexStorageIndex;
    }

    // Updates the storage buffer index used to access uniforms.
    ENABLE_IF(SsboIndexAttrib::kEnabled)
    updateSsboIndexAttrib(uint32_t ssboIndex) {
        SkASSERT(fAttribs & PatchAttribs::kSsboIndex);
        fSsboIndex = ssboIndex;
    }

    /**
     * writeX functions for supported patch geometry types. Every geometric type is converted to an
     * equivalent cubic or conic, so this will always write at minimum 8 floats for the four control
     * points (cubic) or three control points and {w, inf} (conics). The PatchWriter additionally
     * writes the current values of all attributes enabled in its PatchAttribs flags.
     */

    // Write a cubic curve with its four control points.
    AI void writeCubic(float2 p0, float2 p1, float2 p2, float2 p3) {
        if constexpr (ArcLengthAttrib::kEnabled) {
            this->holdArcLengthVerb(fArc.fAccumulator.cubic(p0, p1, p2, p3),
                                    {ArcLengthState::VerbType::kCubic, {p0, p1, p2, p3}});
        } else {
            this->writeCubicVerb(p0, p1, p2, p3);
        }
    }
    AI void writeCubic(const SkPoint pts[4]) {
        float4 p0p1 = float4::Load(pts);
        float4 p2p3 = float4::Load(pts + 2);
        this->writeCubic(p0p1.lo, p0p1.hi, p2p3.lo, p2p3.hi);
    }

    // Write a conic curve with three control points and 'w', with the last coord of the last
    // control point signaling a conic by being set to infinity.
    AI void writeConic(float2 p0, float2 p1, float2 p2, float w) {
        if constexpr (ArcLengthAttrib::kEnabled) {
            this->holdArcLengthVerb(fArc.fAccumulator.conic(p0, p1, p2, w),
                                    {ArcLengthState::VerbType::kConic, {p0, p1, p2}, w});
        } else {
            this->writeConicVerb(p0, p1, p2, w);
        }
    }
    AI void writeConic(const SkPoint pts[3], float w) {
        this->writeConic(sk_bit_cast<float2>(pts[0]),
                         sk_bit_cast<float2>(pts[1]),
                         sk_bit_cast<float2>(pts[2]),
                         w);
    }

    // Write a quadratic curve that automatically converts its three control points into an
    // equivalent cubic.
    AI void writeQuadratic(float2 p0, float2 p1, float2 p2) {
        if constexpr (ArcLengthAttrib::kEnabled) {
            this->holdArcLengthVerb(fArc.fAccumulator.quad(p0, p1, p2),
                                    {ArcLengthState::VerbType::kQuad, {p0, p1, p2}});
        } else {
            this->writeQuadraticVerb(p0, p1, p2);
        }
    }
    AI void writeQuadratic(const SkPoint pts[3]) {
        this->writeQuadratic(sk_bit_cast<float2>(pts[0]),
                             sk_bit_cast<float2>(pts[1]),
                             sk_bit_cast<float2>(pts[2]));
    }

    // Write a line that is automatically converted into an equivalent cubic.
    AI void writeLine(float4 p0p1) {
        if constexpr (kDiscardFlatCurves) {
            return;
        }

        // No chopping needed, a line only ever requires one segment (the minimum required already).
        fTolerances.setParametricSegments(1.f);
        if constexpr (kReplicateLineEndPoints) {
            // Visually this cubic is still a line, but 't' does not move linearly over the line,
            // so Wang's formula is more pessimistic. Shaders should avoid evaluating Wang's
            // formula when a patch has control points in this arrangement.
            this->writeCubicPatch(p0p1.lo, p0p1.lo, p0p1.hi, p0p1.hi);
        } else {
            // In exact math, this cubic structure should have Wang's formula return 0. Due to
            // floating point math, this isn't always the case, so shaders need some way to restrict
            // the number of parametric segments if Wang's formula numerically blows up.
            this->writeCubicPatch(p0p1.lo, (p0p1.zwxy() - p0p1) * (1/3.f) + p0p1, p0p1.hi);
        }
    }
    AI void writeLine(float2 p0, float2 p1) {
        if constexpr (ArcLengthAttrib::kEnabled) {
            this->holdArcLengthVerb(fArc.fAccumulator.line(p0, p1),
                                    {ArcLengthState::VerbType::kLine, {p0, p1}});
        } else {
            this->writeLine({p0, p1});
        }
    }
    AI void writeLine(SkPoint p0, SkPoint p1) {
        this->writeLine(sk_bit_cast<float2>(p0), sk_bit_cast<float2>(p1));
    }

    // Write a triangle by setting it to a conic with w=Inf, and using a distinct
    // explicit curve type for when inf isn't supported in shaders.
    AI void writeTriangle(float2 p0, float2 p1, float2 p2) {
        // No chopping needed, the max supported segment count should always support 2 lines
        // (which form a triangle when implicitly closed).
        static constexpr float kTriangleSegments_p4 = 2.f * 2.f * 2.f * 2.f;
        fTolerances.setParametricSegments(kTriangleSegments_p4);
        this->writePatch(p0, p1, p2, {SK_FloatInfinity, SK_FloatInfinity},
                         kTriangularConicCurveType);
    }
    AI void writeTriangle(SkPoint p0, SkPoint p1, SkPoint p2) {
        this->writeTriangle(sk_bit_cast<float2>(p0),
                            sk_bit_cast<float2>(p1),
                            sk_bit_cast<float2>(p2));
    }

    // Writes a circle used for round caps and cusps in stroking, encoded as a cubic with identical
    // control points and an empty join.
    AI void writeCircle(SkPoint p) {
        if constexpr (ArcLengthAttrib::kEnabled) {
            // PatchWriter writes caps itself with writeCircleAt(), so with kArcLength, a circle
            // written here is a cusp. It adds no arc length, and neither does the join after it.
            // Nothing joins to the end of the verb before it, so that verb can be written out now.
            this->writeHeldVerb(fArc.fAccumulator.cusp());
        }
        this->writeCircleAt(p, this->arcLengthPosition());
    }

    // Writes either a square in isolation, or a half-square that will be oriented and joined
    // with `joinTo` (e.g. it will be a cap extending the stroke radius from `p` opposite of
    // the join control point). It is encoded as a conic with identical control points (vs. circles
    // which are cubics with identical control points).
    AI void writeSquare(SkPoint p, std::optional<SkPoint> joinTo) {
        if constexpr (ArcLengthAttrib::kEnabled) {
            // Like a cusp (see writeCircle()), a square has no length, and nothing joins across it.
            this->writeHeldVerb(fArc.fAccumulator.cusp());
        }
        this->writeSquareAt(p, joinTo, this->arcLengthPosition());
    }
    AI void writeSquare(float2 p, float2 joinTo) {
        this->writeSquare(SkPoint{p[0], p[1]}, SkPoint{joinTo[0], joinTo[1]});
    }

    // Brackets the pieces of a curve that lie inside 'fold' (see ArcLengthFold), which
    // WriteStrokedPathVerbs() chops the curve at. Every patch written in between (including a
    // circle for a cusp) describes the fold instead of its own arc lengths and joins (see
    // ArcLengthData), so the shaders parameterize all of them by the fold's sweep. Without
    // kArcLength, these do nothing.
    void beginFold([[maybe_unused]] const ArcLengthFold& fold) {
        if constexpr (ArcLengthAttrib::kEnabled) {
            // Nothing joins to the end of the piece before the fold, so it can be written out.
            this->writeHeldVerb(fArc.fAccumulator.beginFold(fold));
            float2 a = {fold.fA.fX, fold.fA.fY};
            float2 b = {fold.fB.fX, fold.fB.fY};
            fArc.fFold = ArcLengthData{/*fStart=*/fArc.fAccumulator.accumulated(),
                                       /*fJoinLength=*/fold.length(),
                                       /*fCurveLength=*/std::atan2(cross(a, b), dot(a, b)),
                                       fArc.fTotalPathLength,
                                       /*fNextPoint=*/fold.fCorner,
                                       /*fStartRadius=*/-fold.fRadius,
                                       /*fEndRadius=*/std::atan2(a[1], a[0])};
            fArc.fInFold = true;
        }
    }
    void endFold() {
        if constexpr (ArcLengthAttrib::kEnabled) {
            fArc.fAccumulator.endFold();
            fArc.fInFold = false;
        }
    }

private:
    // These write the verbs passed to the public writeX() functions. With kArcLength, the verbs are
    // held back first, and these are called by writeHeldVerb().
    AI void writeCubicVerb(float2 p0, float2 p1, float2 p2, float2 p3) {
        float n4 = wangs_formula::cubic_p4(kPrecision, p0, p1, p2, p3, fApproxTransform);
        if constexpr (kDiscardFlatCurves) {
            if (n4 <= 1.f) {
                // This cubic only needs one segment (e.g. a line) but we're not filling space with
                // fans or stroking, so nothing actually needs to be drawn.
                return;
            }
        }
        if (int numPatches = this->accountForCurve(n4)) {
            this->chopAndWriteCubics(p0, p1, p2, p3, numPatches);
        } else {
            if constexpr (ArcLengthAttrib::kEnabled) {
                this->setArcLengthFinalPiece();
            }
            this->writeCubicPatch(p0, p1, p2, p3);
        }
    }

    AI void writeConicVerb(float2 p0, float2 p1, float2 p2, float w) {
        float n2 = wangs_formula::conic_p2(kPrecision, p0, p1, p2, w, fApproxTransform);
        if constexpr (kDiscardFlatCurves) {
            if (n2 <= 1.f) {
                // This conic only needs one segment (e.g. a line) but we're not filling space with
                // fans or stroking, so nothing actually needs to be drawn.
                return;
            }
        }
        if (int numPatches = this->accountForCurve(n2 * n2)) {
            this->chopAndWriteConics(p0, p1, p2, w, numPatches);
        } else {
            if constexpr (ArcLengthAttrib::kEnabled) {
                this->setArcLengthFinalPiece();
            }
            this->writeConicPatch(p0, p1, p2, w);
        }
    }

    AI void writeQuadraticVerb(float2 p0, float2 p1, float2 p2) {
        float n4 = wangs_formula::quadratic_p4(kPrecision, p0, p1, p2, fApproxTransform);
        if constexpr (kDiscardFlatCurves) {
            if (n4 <= 1.f) {
                // This quad only needs one segment (e.g. a line) but we're not filling space with
                // fans or stroking, so nothing actually needs to be drawn.
                return;
            }
        }
        if (int numPatches = this->accountForCurve(n4)) {
            this->chopAndWriteQuads(p0, p1, p2, numPatches);
        } else {
            if constexpr (ArcLengthAttrib::kEnabled) {
                this->setArcLengthFinalPiece();
            }
            this->writeQuadPatch(p0, p1, p2);
        }
    }

    // Circles and squares have no length, so they're written at a single arc length position.
    AI void writeCircleAt(SkPoint p, [[maybe_unused]] float arcLengthPosition) {
        if constexpr (ArcLengthAttrib::kEnabled) {
            // Nothing joins to the patch before a cap or a cusp (see ArcLengthAccumulator::cusp()).
            // Completing it now also writes it out before this one, so the order doesn't change.
            this->completeArcLengthPatch();
            this->setArcLengthPoint(arcLengthPosition);
        }
        // This does not use writePatch() because it uses its own location as the join attribute
        // value instead of fJoin and never defers.
        fTolerances.setParametricSegments(1.f);
        if (VertexWriter vw = fPatchAllocator.append(fTolerances)) {
            vw << VertexWriter::Repeat<4>(p); // p0,p1,p2,p3 = p -> 4 copies
            this->emitPatchAttribs(std::move(vw), {fAttribs, p}, kCubicCurveType);
        }
    }

    AI void writeSquareAt(SkPoint p,
                          std::optional<SkPoint> joinTo,
                          [[maybe_unused]] float arcLengthPosition) {
        if constexpr (ArcLengthAttrib::kEnabled) {
            // See writeCircleAt().
            this->completeArcLengthPatch();
            this->setArcLengthPoint(arcLengthPosition);
        }
        fTolerances.setParametricSegments(1.f);
        if (VertexWriter vw = fPatchAllocator.append(fTolerances)) {
            vw << VertexWriter::Repeat<3>(p) // p0,p1,p2 = p -> 3 copies
               << float2{1.f, SK_FloatInfinity}; // conic with w = 1
            this->emitPatchAttribs(std::move(vw), {fAttribs, joinTo.value_or(p)}, kConicCurveType);
        }
    }

    // kArcLength helpers. The accumulator can only finish a verb once it knows about the join after
    // it (see ArcLengthAccumulator::Share()), so holdArcLengthVerb() holds each verb back until
    // then, and writeHeldVerb() writes it out. That describes the whole verb with
    // beginArcLengthVerb(). Then, before writing each of the verb's patches, it calls
    // setArcLengthPiece() for all but the last patch of a chopped verb, and
    // setArcLengthFinalPiece() for the last (or only) one.
    //
    // A verb's join and its end position come from the accumulator, not from the chopped
    // pieces, so the arc lengths written don't depend on how (or whether) curves get chopped. That
    // keeps them identical to ArcLengthPrecalculator's, which never chops.
    //
    // E2: That includes the joins' corner radii, which are limited by the lengths of the whole
    // verbs on either side (see ArcLengthAccumulator::Share()). The first piece of a chopped
    // verb gets its start join and the last piece gets its end join, but the pieces in between
    // know about neither. If a chopped piece next to a sharp join is shorter than the join's trim,
    // the join reaches past it into the next piece, which draws its own arc length there instead
    // and leaves a seam. That takes extreme scales, or a trim that covers most of its verb.
    // Alternatives: limit the corner radii by the lengths of the pieces next to the joins (then the
    // arc lengths would depend on chopping, so ArcLengthPrecalculator would have to chop the same
    // way to match), or give every piece of a verb both of its joins.

    // Gives 'verb' to the accumulator ('added' is what that returned) and holds it back, after
    // writing out the verb held before it if that's now finished.
    ENABLE_IF(ArcLengthAttrib::kEnabled)
    holdArcLengthVerb(const ArcLengthAccumulator::Added& added,
                      const typename ArcLengthState::Verb& verb) {
        using VerbType = typename ArcLengthState::VerbType;
        if (added.fDegenerate) {
            // A degenerate verb doesn't write any patches, but its curve type still tells
            // writeDeferredStrokePatch() to draw caps for an otherwise empty contour (see
            // writePatch()). If a verb is held, that has to wait until the held verb is written.
            float curveType = verb.fType == VerbType::kConic ? kConicCurveType : kCubicCurveType;
            if (fArc.fHeldVerb.fType != VerbType::kNone) {
                fArc.fTrailingCurveType = curveType;
            } else {
                fDeferredPatch.fCurveType = curveType;
            }
            return;
        }
        this->writeHeldVerb(added.fFinished);
        SkASSERT(fArc.fHeldVerb.fType == VerbType::kNone);
        if (fArc.fInFold) {
            // A piece of a curve inside a fold describes the fold (see setArcLengthAttrib()),
            // which is already known, so it doesn't have to wait for anything.
            this->writeArcLengthVerb(verb);
            return;
        }
        fArc.fHeldVerb = verb;
    }

    // Writes out the held verb, if the accumulator just finished it as 'span'.
    ENABLE_IF(ArcLengthAttrib::kEnabled)
    writeHeldVerb(const std::optional<ArcLengthAccumulator::Span>& span) {
        using VerbType = typename ArcLengthState::VerbType;
        if (!span) {
            // The accumulator holds back the same verbs, so it can't have finished one.
            SkASSERT(fArc.fHeldVerb.fType == VerbType::kNone);
            return;
        }
        SkASSERT(fArc.fHeldVerb.fType != VerbType::kNone);
        const typename ArcLengthState::Verb verb = fArc.fHeldVerb;
        fArc.fHeldVerb.fType = VerbType::kNone;
        this->beginArcLengthVerb(*span);
        this->writeArcLengthVerb(verb);
        if (fArc.fTrailingCurveType >= 0.f) {
            fDeferredPatch.fCurveType = fArc.fTrailingCurveType;
            fArc.fTrailingCurveType = -1.f;
        }
    }

    ENABLE_IF(ArcLengthAttrib::kEnabled)
    writeArcLengthVerb(const typename ArcLengthState::Verb& verb) {
        using VerbType = typename ArcLengthState::VerbType;
        const float2* p = verb.fPts;
        switch (verb.fType) {
            case VerbType::kLine:
                this->setArcLengthFinalPiece();
                this->writeLine({p[0], p[1]});
                break;
            case VerbType::kQuad:
                this->writeQuadraticVerb(p[0], p[1], p[2]);
                break;
            case VerbType::kConic:
                this->writeConicVerb(p[0], p[1], p[2], verb.fW);
                break;
            case VerbType::kCubic:
                this->writeCubicVerb(p[0], p[1], p[2], p[3]);
                break;
            case VerbType::kNone:
                break;
        }
    }

    ENABLE_IF(ArcLengthAttrib::kEnabled)
    beginArcLengthVerb(const ArcLengthAccumulator::Span& verb) {
        fArc.fVerb = verb;
        fArc.fPieceStart = verb.fStart;
        fArc.fFirstPiece = true;
    }

    ENABLE_IF(ArcLengthAttrib::kEnabled) setArcLengthPiece(float pieceLength) {
        // Only the first piece has the verb's join.
        const bool hasJoin = fArc.fFirstPiece;
        float joinLength = hasJoin ? fArc.fVerb.fJoinLength : 0.f;
        float joinTrim = hasJoin ? fArc.fVerb.fJoinTrim : 0.f;
        this->setArcLengthAttrib(fArc.fPieceStart, joinLength, pieceLength,
                                 hasJoin ? fArc.fVerb.fJoinRadius : 0.f);
        fArc.fPieceStart = (fArc.fPieceStart + joinLength) + (pieceLength - joinTrim);
        fArc.fFirstPiece = false;
    }

    ENABLE_IF(ArcLengthAttrib::kEnabled) setArcLengthFinalPiece() {
        if (fArc.fFirstPiece) {
            // The verb wasn't chopped.
            this->setArcLengthAttrib(fArc.fPieceStart, fArc.fVerb.fJoinLength,
                                     fArc.fVerb.fCurveLength, fArc.fVerb.fJoinRadius);
        } else {
            // The last piece absorbs any difference between the sum of the pieces' lengths and the
            // verb's length, so it ends exactly where the verb does.
            this->setArcLengthAttrib(fArc.fPieceStart, 0.f,
                                     std::max(fArc.fVerb.fEnd - fArc.fPieceStart, 0.f), 0.f);
        }
        fArc.fPieceStart = fArc.fVerb.fEnd;
        fArc.fFirstPiece = false;
    }

    ENABLE_IF(ArcLengthAttrib::kEnabled) setArcLengthPoint(float position) {
        this->setArcLengthAttrib(position, 0.f, 0.f, 0.f);
    }

    ENABLE_IF(ArcLengthAttrib::kEnabled) setArcLengthAttrib(float start,
                                                            float joinLength,
                                                            float curveLength,
                                                            float joinRadius) {
        SkASSERT(fAttribs & PatchAttribs::kArcLength);
        if (fArc.fInFold) {
            // Every patch inside a fold describes the fold instead (see ArcLengthData).
            fArcLength = fArc.fFold;
            return;
        }
        // The end join is filled in by completeArcLengthPatch().
        fArcLength = ArcLengthData{start, joinLength, curveLength, fArc.fTotalPathLength,
                                   /*fNextPoint=*/{0.f, 0.f}, joinRadius, /*fEndRadius=*/0.f};
    }

    // Fills in the end join of the most recently written patch now that it's known what follows
    // it: either the next patch, which starts by heading towards 'nextPoint' after a join with
    // corner radius 'nextJoinRadius', or (by default) nothing that joins to it. Then writes the
    // patch out, unless it's the deferred first patch of its contour, which
    // writeDeferredStrokePatch() writes out once its own join is known.
    ENABLE_IF(ArcLengthAttrib::kEnabled) completeArcLengthPatch(float2 nextPoint = 0.f,
                                                                float nextJoinRadius = 0.f) {
        using PrevPatch = typename ArcLengthState::PrevPatch;
        if (fArc.fPrevPatch == PrevPatch::kNone) {
            return;
        }
        char* patch = fArc.fHeldPatch;
        if constexpr (kTrackJoinControlPoints) {
            if (fArc.fPrevPatch == PrevPatch::kDeferred) {
                patch = fDeferredPatch.fData;
            }
        }
        // kArcLength is the last attribute of the patch.
        char* arcLength = patch + PatchStride(fAttribs) - sizeof(ArcLengthData);
        float startRadius;
        memcpy(&startRadius, arcLength + offsetof(ArcLengthData, fStartRadius), sizeof(float));
        // A patch inside a fold describes the fold instead of its end join (see ArcLengthData).
        if (!(startRadius < 0.f)) {
            nextPoint.store(arcLength + offsetof(ArcLengthData, fNextPoint));
            memcpy(arcLength + offsetof(ArcLengthData, fEndRadius), &nextJoinRadius,
                   sizeof(float));
        }
        if (fArc.fPrevPatch == PrevPatch::kHeld) {
            // Restore the tolerance the patch was written with, like for the deferred patch.
            float n4 = fTolerances.numParametricSegments_p4();
            fTolerances.setParametricSegments(fArc.fHeldPatchN_p4);
            if (VertexWriter vw = fPatchAllocator.append(fTolerances)) {
                vw << VertexWriter::Array<char>(fArc.fHeldPatch, PatchStride(fAttribs));
            }
            fTolerances.setParametricSegments(n4);
        }
        fArc.fPrevPatch = PrevPatch::kNone;
    }

    float arcLengthPosition() const { return this->accumulatedLength(); }

    AI void emitPatchAttribs(VertexWriter vertexWriter,
                             const JoinAttrib& join,
                             float explicitCurveType) {
        vertexWriter << join << fFanPoint << fStrokeParams << fColor << fDepth
                     << CurveTypeAttrib{fAttribs, explicitCurveType}
                     << fVertexStorageIndex
                     << fSsboIndex
                     << fArcLength;
    }

    AI VertexWriter appendPatch(float explicitCurveType) {
        if constexpr (kTrackJoinControlPoints) {
            // Can switch to !hasPending() once updateJoinControlPointAttrib goes away.
            if (fDeferredPatch.mustDefer()) {
                SkASSERT(PatchStride(fAttribs) <= kMaxStride);
                // Save the computed parametric segment tolerance value so that we can pass that to
                // the PatchAllocator when flushing the deferred patch.
                fDeferredPatch.fCurveType = explicitCurveType;
                fDeferredPatch.fN_p4 = fTolerances.numParametricSegments_p4();
                SkASSERT(!fDeferredPatch.mustDefer() && fDeferredPatch.hasPending());
                if constexpr (ArcLengthAttrib::kEnabled) {
                    if (fAttribs & PatchAttribs::kArcLength) {
                        fArc.fPrevPatch = ArcLengthState::PrevPatch::kDeferred;
                    }
                }
                return {fDeferredPatch.fData, PatchStride(fAttribs)};
            }
        }
        if constexpr (ArcLengthAttrib::kEnabled) {
            if (fAttribs & PatchAttribs::kArcLength) {
                // Hold the patch back until its end join is known (see completeArcLengthPatch()).
                SkASSERT(PatchStride(fAttribs) <= kMaxStride);
                SkASSERT(fArc.fPrevPatch == ArcLengthState::PrevPatch::kNone);
                fArc.fPrevPatch = ArcLengthState::PrevPatch::kHeld;
                fArc.fHeldPatchN_p4 = fTolerances.numParametricSegments_p4();
                return {fArc.fHeldPatch, PatchStride(fAttribs)};
            }
        }
        return fPatchAllocator.append(fTolerances);
    }

    AI void writePatch(float2 p0, float2 p1, float2 p2, float2 p3, float explicitCurveType) {
        if constexpr (kTrackJoinControlPoints) {
            // Store this even if we don't write a patch to remember we should add caps
            fDeferredPatch.fCurveType = explicitCurveType;
        }

        float2 last = explicitCurveType == kCubicCurveType ? p3 : p2;
        if (all(p0 == p1 & p1 == p2 & p2 == last)) {
            // Degenerate patch, so skip writing. In a regular fill or stroke, this won't affect
            // rendering. If there is a subsequent close verb, we may add a cap later.
            return;
        }

        if constexpr (ArcLengthAttrib::kEnabled) {
            if (fAttribs & PatchAttribs::kArcLength) {
                // This patch's start tangent and join complete the previous patch's end join. A
                // patch inside a fold has no start join (its negative fStartRadius describes the
                // fold instead), since nothing joins to the end of the piece before the fold.
                this->completeArcLengthPatch(TangentPoint(p0, p1, p2, last),
                                             std::max((*fArcLength).fStartRadius, 0.f));
            }
        }

        if (VertexWriter vw = this->appendPatch(explicitCurveType)) {
            // NOTE: fJoin will be undefined if we're writing to a deferred patch. If that's the
            // case, correct data will overwrite it when the contour is closed (this is fine since a
            // deferred patch writes to CPU memory instead of directly to the GPU buffer).
            vw << p0 << p1 << p2 << p3;
            this->emitPatchAttribs(std::move(vw), fJoin, explicitCurveType);

            // Automatically update join control point for next patch.
            if constexpr (kTrackJoinControlPoints) {
                last.store(&fDeferredPatch.fLastControlPoint);
                // Points are ordered in reverse order to get outgoing tangent control point
                TangentPoint(last, p2, p1, p0).store(&fJoin);
            }
        }
    }

    // Helpers that normalize curves to a generic patch, but do no other work.
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

    int accountForCurve(float n4) {
        if (n4 <= kMaxParametricSegments_p4) {
            // Record n^4 and return 0 to signal no chopping, but don't let n go below 1 as that
            // will cause problems when we take the log of it (particularly reaching 0, which can
            // happen for control points that are incredibly close to each other).
            fTolerances.setParametricSegments(std::max(1.f, n4));
            return 0;
        } else {
            // Clamp to max allowed segmentation for a patch and return required number of chops
            // to achieve visual correctness.
            fTolerances.setParametricSegments(kMaxParametricSegments_p4);
            return SkScalarCeilToInt(wangs_formula::root4(std::min(n4, kMaxSegmentsPerCurve_p4) /
                                                          kMaxParametricSegments_p4));
        }
    }

    // This does not return b when t==1, but it otherwise seems to get better precision than
    // "a*(1 - t) + b*t" for things like chopping cubics on exact cusp points.
    // The responsibility falls on the caller to check that t != 1 before calling.
    static AI float4 mix(float4 a, float4 b, float4 T) {
        SkASSERT(all((0 <= T) & (T < 1)));
        return (b - a)*T + a;
    }

    // Helpers that chop the curve type into 'numPatches' parametrically uniform curves. It is
    // assumed that 'numPatches' is calculated such that the resulting curves require the maximum
    // number of segments to draw appropriately (since the original presumably needed even more).
    void chopAndWriteQuads(float2 p0, float2 p1, float2 p2, int numPatches) {
        InnerTriangulator triangulator(numPatches, sk_bit_cast<SkPoint>(p0));
        for (; numPatches >= 3; numPatches -= 2) {
            // Chop into 3 quads.
            float4 T = float4(1,1,2,2) / numPatches;
            float4 ab = mix(p0.xyxy(), p1.xyxy(), T);
            float4 bc = mix(p1.xyxy(), p2.xyxy(), T);
            float4 abc = mix(ab, bc, T);
            // p1 & p2 of the cubic representation of the middle quad.
            float4 middle = mix(ab, bc, mix(T, T.zwxy(), 2/3.f));

            if constexpr (ArcLengthAttrib::kEnabled) {
                this->setArcLengthPiece(ArcLengthCalculator::ConicLength(p0, ab.lo, abc.lo, 1.f));
            }
            this->writeQuadPatch(p0, ab.lo, abc.lo);  // Write the 1st quad.
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangle(p0, abc.lo, abc.hi);
            }
            if constexpr (ArcLengthAttrib::kEnabled) {
                this->setArcLengthPiece(
                        ArcLengthCalculator::CubicLength(abc.lo, middle.lo, middle.hi, abc.hi));
            }
            this->writeCubicPatch(abc.lo, middle, abc.hi);  // Write the 2nd quad (already a cubic)
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangleStack(triangulator.pushVertex(sk_bit_cast<SkPoint>(abc.hi)));
            }
            std::tie(p0, p1) = {abc.hi, bc.hi};  // Save the 3rd quad.
        }
        if (numPatches == 2) {
            // Chop into 2 quads.
            float2 ab = (p0 + p1) * .5f;
            float2 bc = (p1 + p2) * .5f;
            float2 abc = (ab + bc) * .5f;

            if constexpr (ArcLengthAttrib::kEnabled) {
                this->setArcLengthPiece(ArcLengthCalculator::ConicLength(p0, ab, abc, 1.f));
            }
            this->writeQuadPatch(p0, ab, abc);  // Write the 1st quad.
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangle(p0, abc, p2);
            }
            if constexpr (ArcLengthAttrib::kEnabled) {
                this->setArcLengthFinalPiece();
            }
            this->writeQuadPatch(abc, bc, p2);  // Write the 2nd quad.
        } else {
            SkASSERT(numPatches == 1);
            if constexpr (ArcLengthAttrib::kEnabled) {
                this->setArcLengthFinalPiece();
            }
            this->writeQuadPatch(p0, p1, p2);  // Write the single remaining quad.
        }
        if constexpr (kAddTrianglesWhenChopping) {
            this->writeTriangleStack(triangulator.pushVertex(sk_bit_cast<SkPoint>(p2)));
            this->writeTriangleStack(triangulator.close());
        }
    }

    void chopAndWriteConics(float2 p0, float2 p1, float2 p2, float w, int numPatches) {
        InnerTriangulator triangulator(numPatches, sk_bit_cast<SkPoint>(p0));
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
            float2 p0_chopped = h0.xy() / h0.w();
            float2 p1_chopped = ab.xy() / ab.w();
            float w_chopped = ab.w() / sqrtf(h0.w() * abc.w());
            if constexpr (ArcLengthAttrib::kEnabled) {
                this->setArcLengthPiece(ArcLengthCalculator::ConicLength(
                        p0_chopped, p1_chopped, midpoint, w_chopped));
            }
            this->writeConicPatch(p0_chopped,
                                  p1_chopped,
                                  midpoint,
                                  w_chopped);
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangleStack(triangulator.pushVertex(sk_bit_cast<SkPoint>(midpoint)));
            }
            std::tie(h0, h1) = {abc, bc};  // Save the 2nd conic (in homogeneous space).
        }
        // Project and write the remaining conic.
        SkASSERT(numPatches == 1);
        float2 p0_chopped = h0.xy() / h0.w();
        float2 p1_chopped = h1.xy() / h1.w();
        float w_chopped = h1.w() / sqrtf(h0.w());
        if constexpr (ArcLengthAttrib::kEnabled) {
            this->setArcLengthFinalPiece();
        }
        this->writeConicPatch(p0_chopped,
                              p1_chopped,
                              h2.xy(), // h2.w == 1
                              w_chopped);
        if constexpr (kAddTrianglesWhenChopping) {
            this->writeTriangleStack(triangulator.pushVertex(sk_bit_cast<SkPoint>(h2.xy())));
            this->writeTriangleStack(triangulator.close());
        }
    }

    void chopAndWriteCubics(float2 p0, float2 p1, float2 p2, float2 p3, int numPatches) {
        InnerTriangulator triangulator(numPatches, sk_bit_cast<SkPoint>(p0));
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

            if constexpr (ArcLengthAttrib::kEnabled) {
                this->setArcLengthPiece(
                        ArcLengthCalculator::CubicLength(p0, ab.lo, abc.lo, abcd.lo));
            }
            this->writeCubicPatch(p0, ab.lo, abc.lo, abcd.lo);  // Write the 1st cubic.
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangle(p0, abcd.lo, abcd.hi);
            }
            if constexpr (ArcLengthAttrib::kEnabled) {
                this->setArcLengthPiece(
                        ArcLengthCalculator::CubicLength(abcd.lo, middle.lo, middle.hi, abcd.hi));
            }
            this->writeCubicPatch(abcd.lo, middle, abcd.hi);  // Write the 2nd cubic.
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangleStack(triangulator.pushVertex(sk_bit_cast<SkPoint>(abcd.hi)));
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

            if constexpr (ArcLengthAttrib::kEnabled) {
                this->setArcLengthPiece(ArcLengthCalculator::CubicLength(p0, ab, abc, abcd));
            }
            this->writeCubicPatch(p0, ab, abc, abcd);  // Write the 1st cubic.
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangle(p0, abcd, p3);
            }
            if constexpr (ArcLengthAttrib::kEnabled) {
                this->setArcLengthFinalPiece();
            }
            this->writeCubicPatch(abcd, bcd, cd, p3);  // Write the 2nd cubic.
        } else {
            SkASSERT(numPatches == 1);
            if constexpr (ArcLengthAttrib::kEnabled) {
                this->setArcLengthFinalPiece();
            }
            this->writeCubicPatch(p0, p1, p2, p3);  // Write the single remaining cubic.
        }
        if constexpr (kAddTrianglesWhenChopping) {
            this->writeTriangleStack(triangulator.pushVertex(sk_bit_cast<SkPoint>(p3)));
            this->writeTriangleStack(triangulator.close());
        }
    }

    ENABLE_IF(kAddTrianglesWhenChopping)
    writeTriangleStack(MiddleOutPolygonTriangulator::PoppedTriangleStack&& stack) {
        for (auto [p0, p1, p2] : stack) {
            this->writeTriangle(p0, p1, p2);
        }
    }

    static float2 TangentPoint(float2 p0, float2 p1, float2 p2, float2 p3) {
        if (any(p0 != p1)) {
            return p1;
        } else if (any(p2 != p1)) {
            return p2;
        } else {
            return p3;
        }
    }

    // Runtime configuration, will always contain required attribs but may not have all optional
    // attribs enabled (e.g. depending on caps or batching).
    const PatchAttribs fAttribs;

    // The 2x2 approximation of the local-to-device transform that will affect subsequently
    // recorded curves (when fully transformed in the vertex shader).
    wangs_formula::VectorXform fApproxTransform = {};
    // A maximum scale factor extracted from the current approximate transform.
    float fMaxScale = 1.0f;
    // Tracks the linear tolerances for the most recently written patches.
    LinearTolerances fTolerances;

    PatchAllocator fPatchAllocator;
    DeferredPatch  fDeferredPatch; // only usable if kTrackJoinControlPoints is true

    // Instance attribute state written after the 4 control points of a patch
    JoinAttrib     fJoin;
    FanPointAttrib fFanPoint;
    StrokeAttrib   fStrokeParams;
    ColorAttrib    fColor;
    DepthAttrib    fDepth;

    // Index into a shared storage buffer containing this PatchWriter's patches' corresponding
    // uniforms. Written out as an attribute with every patch, to read the appropriate uniform
    // values from the storage buffer on draw.
    SsboIndexAttrib fSsboIndex;

    VertexStorageIndexAttrib fVertexStorageIndex;

    // Used for arc length parameterized path effects. See ArcLengthData for the layout of the
    // attrib, and ArcLengthAccumulator for how lengths are assigned.
    ArcLengthAttrib fArcLength;
    ArcState fArc;
};

}  // namespace skgpu::tess

#undef ENABLE_IF
#undef AI

#endif  // skgpu_tessellate_PatchWriter_DEFINED
