/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/sparse_strips/StrokeFlatten.h"

#include "include/core/SkPath.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStrokeRec.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/sparse_strips/CircularArc.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/FlattenUtils.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/tessellate/Tessellation.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>

namespace skgpu::graphite {

using flatten_utils::approx_parabola_integral;
using flatten_utils::approx_parabola_inv_integral;
using flatten_utils::estimate_num_quads_from_cubic;
using flatten_utils::inv_cubic_err_threshold;
using flatten_utils::is_completely_culled;
using flatten_utils::is_completely_left;
using flatten_utils::split_conic_to_cubics;

namespace {

// Quads whose control polygon is this close to collinear contribute no interior samples.
constexpr float kEpsilon = Flatten::kEpsilonF;

// Like SkStroke's "teeny" segments, shorter segments are ignored (apart from producing caps on
// otherwise empty contours).
constexpr float kDegenerateLength = SK_ScalarNearlyZero / 4;

// SkStroke's thresholds for treating quads/conics and cubics as degenerate (collinear) curves.
constexpr float kQuadLineSlop = 0.000005f;
constexpr float kCubicLineSlop = 0.00001f;

// Guards against pathological inputs.
constexpr int kMaxSegmentsPerCurve = 1 << 16;
constexpr int kMaxRefinePerInterval = 1 << 10;
constexpr int kMaxRefineDepth = 8;

// Quads are split at fixed normalized angles when r * kMax exceeds this, which keeps the
// per-piece sqrt(1 + r * k) estimate tight (see addMappedQuadPieces()). Below it, the whole quad
// uses its maximum, over-sampling it by at most sqrt(1.5) - 1 = 22%, which is cheaper than
// splitting.
constexpr float kSplitThreshold = 0.5f;

// x = tan(phi) / 2 for phi = -75..75 degrees in 15 degree steps: the points of the normalized
// parabola y = x^2 where its tangent has turned a multiple of 15 degrees from the vertex.
constexpr float kSplitX[] = {-1.86602540f,
                             -0.86602540f,
                             -0.5f,
                             -0.28867513f,
                             -0.13397460f,
                             0.0f,
                             0.13397460f,
                             0.28867513f,
                             0.5f,
                             0.86602540f,
                             1.86602540f};

// Derivatives shorter than this are not normalized; the previous normal is used instead.
constexpr float kMinDerivLengthSq = 1e-24f;

// SIMD width of the sample and line passes, and the padding of their arrays.
constexpr int kLanes = 8;
// Below these sizes, the SIMD passes don't pay off and the scalar code is used instead.
constexpr int kMinSimdSamples = 3;  // interior samples of one cubic
constexpr int kMinSimdQuads = 2;    // quads of one run
constexpr int kMinLineBatch = 4;    // consecutive lines

constexpr float kInfinity = SK_ScalarInfinity;

SK_ALWAYS_INLINE float vdot(SkVector a, SkVector b) { return a.fX * b.fX + a.fY * b.fY; }
SK_ALWAYS_INLINE float vcross(SkVector a, SkVector b) { return a.fX * b.fY - a.fY * b.fX; }
SK_ALWAYS_INLINE float vlength(SkVector v) { return std::sqrt(vdot(v, v)); }

// Rotates by +90 degrees. The left offset of a curve is P + r * perp(T), the right is
// P - r * perp(T).
SK_ALWAYS_INLINE SkVector vperp(SkVector v) { return {-v.fY, v.fX}; }

SK_ALWAYS_INLINE bool degenerate_vector(SkVector v) {
    return !std::isfinite(v.fX) || !std::isfinite(v.fY) || (v.fX == 0 && v.fY == 0);
}

// The unit normal of 'tangent', or 'fallback' if 'tangent' is too short to normalize.
SK_ALWAYS_INLINE SkVector unit_normal_or(SkVector tangent, SkVector fallback) {
    const float lengthSq = vdot(tangent, tangent);
    if (lengthSq >= kMinDerivLengthSq && std::isfinite(lengthSq)) {
        return vperp(tangent) * (1.0f / std::sqrt(lengthSq));
    }
    return fallback;
}

// Loads the points src[0..7] as x and y vectors.
SK_ALWAYS_INLINE void load_points(const SkPoint* src, skvx::float8* x, skvx::float8* y) {
    const auto xy = skvx::Vec<16, float>::Load(src);
    *x = skvx::shuffle<0, 2, 4, 6, 8, 10, 12, 14>(xy);
    *y = skvx::shuffle<1, 3, 5, 7, 9, 11, 13, 15>(xy);
}

// Stores the points (x[i], y[i]) to dst[0..7].
SK_ALWAYS_INLINE void store_points(SkPoint* dst, skvx::float8 x, skvx::float8 y) {
    skvx::shuffle<0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15>(skvx::join(x, y))
            .store(dst);
}

// Whether each of the 'pairs' consecutive unit normals (nx[k], ny[k]) and (nx[k + 1], ny[k + 1])
// has a dot product of at least 'minDot'. NaNs fail. The SIMD version reads up to the next
// multiple of kLanes past 'pairs', plus one, so the arrays must be padded with passing normals.
bool rotations_within(const float* nx, const float* ny, int pairs, float minDot) {
    for (int k = 0; k < pairs; ++k) {
        if (!(nx[k] * nx[k + 1] + ny[k] * ny[k + 1] >= minDot)) {
            return false;
        }
    }
    return true;
}

bool rotations_within_simd(const float* nx, const float* ny, int pairs, float minDot) {
    using skvx::float8;
    for (int k = 0; k < pairs; k += kLanes) {
        const float8 dot = skvx::fma(float8::Load(nx + k),
                                     float8::Load(nx + k + 1),
                                     float8::Load(ny + k) * float8::Load(ny + k + 1));
        if (!skvx::all(dot >= minDot)) {
            return false;
        }
    }
    return true;
}

// Writes points with branch-free removal of consecutive duplicates.
class PointWriter {
public:
    PointWriter(SkPoint* dst, SkPoint last) : fDst(dst), fLast(last) {}

    SK_ALWAYS_INLINE void write(SkPoint pt) {
        *fDst = pt;
        fDst += pt != fLast ? 1 : 0;
        fLast = pt;
    }

    void write(const SkPoint* pts, int count) {
        for (int i = 0; i < count; ++i) {
            this->write(pts[i]);
        }
    }

    void writeReversed(const SkPoint* pts, int count) {
        for (int i = count - 1; i >= 0; --i) {
            this->write(pts[i]);
        }
    }

    // Ends the current sub-path, unless it is empty.
    void sentinel() {
        if (!std::isnan(fLast.fX)) {
            fLast = {SK_ScalarNaN, SK_ScalarNaN};
            *fDst++ = fLast;
        }
    }

    SkPoint* dst() const { return fDst; }
    SkPoint last() const { return fLast; }

private:
    SkPoint* fDst;
    SkPoint fLast;
};

// Whether the points lie entirely beyond one edge of 'rect'. Then so does the curve they control,
// which lies in their convex hull. Unlike Flatten, the stroker treats all four edges alike (see
// "Culling" in StrokeFlatten.h).
template <int N> SK_ALWAYS_INLINE bool is_culled(const SkPoint pts[N], const SkRect& rect) {
    return is_completely_culled<N>(pts, rect) || is_completely_left<N>(pts, rect);
}

int verb_point_count(SkPath::Verb verb) {
    switch (verb) {
        case SkPath::kMove_Verb:
            return 1;
        case SkPath::kLine_Verb:
            return 2;
        case SkPath::kQuad_Verb:
        case SkPath::kConic_Verb:
            return 3;
        case SkPath::kCubic_Verb:
            return 4;
        default:
            return 0;
    }
}

// Squared distance from 'pt' to the segment [a, b], as in SkStroke's pt_to_line().
float pt_to_line_sq(SkPoint pt, SkPoint a, SkPoint b) {
    const SkVector ab = b - a;
    const SkVector ap = pt - a;
    const float denom = vdot(ab, ab);
    const float t = denom > 0 ? vdot(ab, ap) / denom : -1.0f;
    if (t >= 0 && t <= 1) {
        const SkVector d = pt - (a + ab * t);
        return vdot(d, d);
    }
    return vdot(ap, ap);
}

// SkStroke's quad_in_line() / cubic_in_line(): are the points collinear within a tiny slop,
// relative to the distance between the two points that are farthest apart?
template <int N> bool points_in_line(const SkPoint pts[], float slopFactor) {
    float ptMax = -1;
    int outer1 = 0;
    int outer2 = 1;
    for (int i = 0; i < N - 1; ++i) {
        for (int j = i + 1; j < N; ++j) {
            const SkVector d = pts[j] - pts[i];
            const float m = std::max(std::abs(d.fX), std::abs(d.fY));
            if (ptMax < m) {
                outer1 = i;
                outer2 = j;
                ptMax = m;
            }
        }
    }
    const float lineSlop = ptMax * ptMax * slopFactor;
    for (int i = 0; i < N; ++i) {
        if (i != outer1 && i != outer2 &&
            pt_to_line_sq(pts[i], pts[outer1], pts[outer2]) > lineSlop) {
            return false;
        }
    }
    return true;
}

// Cheap conservative pre-tests for points_in_line(): false means the points are certainly not
// collinear within the slop. If the outer pair is (a, b) and all points are within w of segment
// ab, with w^2 = slop * ptMax^2, then every triangle of the points has a doubled area of at most
// 2 w L, where L <= sqrt(S) and ptMax^2 <= S for S, the sum of all squared pairwise distances.
// Triangles that contain both a and b even have a doubled area of at most w |ab|.
bool maybe_in_line3(const SkPoint p[3], float slop) {
    const SkVector a = p[1] - p[0];
    const SkVector b = p[2] - p[0];
    const SkVector c = p[2] - p[1];
    const float area = vcross(a, b);
    const float s = vdot(a, a) + vdot(b, b) + vdot(c, c);
    return area * area <= slop * s * s;
}

bool maybe_in_line4(const SkPoint p[4], float slop) {
    const SkVector d01 = p[1] - p[0];
    const SkVector d02 = p[2] - p[0];
    const SkVector d03 = p[3] - p[0];
    const SkVector d12 = p[2] - p[1];
    const SkVector d13 = p[3] - p[1];
    const SkVector d23 = p[3] - p[2];
    const float a012 = vcross(d01, d02);
    const float a013 = vcross(d01, d03);
    const float a023 = vcross(d02, d03);
    const float a123 = vcross(d12, d13);
    const float maxAreaSq =
            std::max(std::max(a012 * a012, a013 * a013), std::max(a023 * a023, a123 * a123));
    const float s = vdot(d01, d01) + vdot(d02, d02) + vdot(d03, d03) + vdot(d12, d12) +
                    vdot(d13, d13) + vdot(d23, d23);
    return maxAreaSq <= 4.0f * slop * s * s;
}

// Power basis cubic: P(t) = ((A t + B) t + C) t + D and P'(t) = (3 A t + 2 B) t + C.
struct CubicCoeffs {
    explicit CubicCoeffs(const SkPoint p[4])
            : fA((p[3] - p[0]) + (p[1] - p[2]) * 3.0f)
            , fB(((p[2] - p[1]) - (p[1] - p[0])) * 3.0f)
            , fC((p[1] - p[0]) * 3.0f)
            , fD(p[0]) {}

    SkPoint eval(float t) const { return fD + ((fA * t + fB) * t + fC) * t; }
    SkVector deriv(float t) const { return (fA * (3.0f * t) + fB * 2.0f) * t + fC; }

    SkVector fA;
    SkVector fB;
    SkVector fC;
    SkPoint fD;
};

// End tangents that skip coincident control points (the GPU stroker's tan0/tan1 logic).
SkVector start_tangent(const SkPoint p[4]) {
    if (!degenerate_vector(p[1] - p[0])) {
        return p[1] - p[0];
    }
    if (!degenerate_vector(p[2] - p[0])) {
        return p[2] - p[0];
    }
    return p[3] - p[0];
}

SkVector end_tangent(const SkPoint p[4]) {
    if (!degenerate_vector(p[3] - p[2])) {
        return p[3] - p[2];
    }
    if (!degenerate_vector(p[3] - p[1])) {
        return p[3] - p[1];
    }
    return p[3] - p[0];
}

}  // anonymous namespace

StrokeFlatten::StrokeFlatten() {
    static_assert(std::size(kSplitX) == static_cast<size_t>(kNumSplits));
    static_assert(kTolerance == Flatten::kQuadErrTolerance);
    static_assert(kMaxRunCubics >= static_cast<int>(Flatten::kMaxCubicsFromConic));
    for (int j = 0; j < kNumSplits; ++j) {
        const float x = kSplitX[j];
        const float q = 1.0f + 4.0f * x * x;
        fSplitCos3[j] = 1.0f / (q * std::sqrt(q));
        fSplitIntegral[j] = approx_parabola_integral(x);
        fSplitInvIntegral[j] = approx_parabola_inv_integral(fSplitIntegral[j]);
    }
}

bool StrokeFlatten::processStrokeImpl(const SkPath& path,
                                      const SkMatrix& ctm,
                                      const SkStrokeRec& stroke,
                                      const SkIRect& clip,
                                      Polyline* polyline,
                                      bool simd) {
    if (stroke.getStyle() != SkStrokeRec::kStroke_Style || ctm.hasPerspective() ||
        !path.isFinite()) {
        return false;
    }
    const float width = stroke.getWidth();
    const float maxScale = ctm.getMaxScale();
    if (!(width > 0) || !std::isfinite(width) || !(maxScale > 0) || !std::isfinite(maxScale)) {
        return false;
    }
    fSimd = simd;

    // Similarity transforms commute with stroking, so map the input and scale the radius. Other
    // affine transforms (skew, non-uniform scale) stroke in local space and map the output, with
    // a local tolerance that is conservative along the transform's major axis.
    fMapOutput = !ctm.isSimilarity();
    float radius = 0.5f * width;
    float tolerance = kTolerance;
    if (fMapOutput) {
        tolerance /= maxScale;
        fCTM = ctm;
    } else {
        radius *= maxScale;
    }
    // The radius can underflow or overflow; the math below needs it positive and finite.
    if (!(radius > 0) || !std::isfinite(radius)) {
        return false;
    }
    this->setup(radius, tolerance, stroke.getJoin(), stroke.getCap(), stroke.getMiter());

    // Quick reject: nothing is visible if the inflated bounds miss the clip.
    const float outset = stroke.getInflationRadius() * maxScale;
    const SkRect clipRect = SkRect::Make(clip);
    const SkRect devBounds = ctm.mapRect(path.getBounds()).makeOutset(outset, outset);
    if (!devBounds.intersects(clipRect)) {
        return true;
    }
    // Cull curves only if part of the stroke may be outside the clip (see "Culling"). With
    // fMapOutput, curves are stroked in local space: there, the inverse-mapped clip is bounded
    // by a rectangle, and lines (half-planes) map to lines.
    fCull = !clipRect.contains(devBounds);
    if (fCull) {
        if (fMapOutput) {
            SkMatrix inverse;
            fCull = ctm.invert(&inverse);
            const float localOutset = stroke.getInflationRadius();
            fCullRect = inverse.mapRect(clipRect).makeOutset(localOutset, localOutset);
        } else {
            fCullRect = clipRect.makeOutset(outset, outset);
        }
    }

    fPolyline = polyline;
    const SkTDArray<SkPoint>& existing = polyline->points();
    fOutLast =
            existing.empty() ? SkPoint{SK_ScalarNaN, SK_ScalarNaN} : existing[existing.size() - 1];
    fOutStart = fOut = fOutEnd = nullptr;
    // Most strokes emit 2-8 points per path point. Larger outputs grow the reservation 2x.
    fOutHint = 4 * path.countPoints() + 64;
    fContourStarted = false;
    fLinePts.clear();
    this->resetContour();
    const bool mapInput = !fMapOutput && !ctm.isIdentity();

    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        if (mapInput) {
            const int count = verb_point_count(verb);
            ctm.mapPoints(SkSpan<SkPoint>(pts, static_cast<size_t>(count)));
        }
        if (verb == SkPath::kLine_Verb) {
            if (fSimd) {
                // Batch consecutive lines, see flushLines().
                if (fLinePts.empty()) {
                    fLinePts.push_back(fPrevPt);
                }
                fLinePts.push_back(pts[1]);
            } else {
                this->lineTo(pts[1]);
            }
            continue;
        }
        this->flushLines();
        switch (verb) {
            case SkPath::kMove_Verb:
                this->finishContour(/*closed=*/false);
                this->beginContour(pts[0]);
                break;
            case SkPath::kQuad_Verb:
                if (fCull && is_culled<3>(pts, fCullRect)) {
                    this->chordTo(pts[2]);
                } else {
                    this->quadTo(pts);
                }
                break;
            case SkPath::kConic_Verb:
                if (fCull && is_culled<3>(pts, fCullRect)) {
                    this->chordTo(pts[2]);
                } else {
                    this->conicTo(pts, iter.conicWeight());
                }
                break;
            case SkPath::kCubic_Verb:
                if (fCull && is_culled<4>(pts, fCullRect)) {
                    this->chordTo(pts[3]);
                } else {
                    this->cubicTo(pts);
                }
                break;
            case SkPath::kClose_Verb:
                this->finishContour(/*closed=*/true);
                break;
            default:
                break;
        }
    }
    this->flushLines();
    this->finishContour(/*closed=*/false);
    this->commitOutput();
    fPolyline = nullptr;
    return true;
}

SkPoint* StrokeFlatten::growOutput(int maxCount) {
    this->commitOutput();
    const int request = std::max(maxCount, fOutHint);
    fOutHint = std::min(2 * request, 1 << 24);
    fOutStart = fPolyline->beginBulk(request);
    fOut = fOutStart;
    fOutEnd = fOutStart + request;
    return fOut;
}

void StrokeFlatten::commitOutput() {
    if (fOutStart) {
        const int count = static_cast<int>(fOut - fOutStart);
        if (fMapOutput) {
            // Sentinels stay NaN.
            fCTM.mapPoints(SkSpan<SkPoint>(fOutStart, static_cast<size_t>(count)));
        }
        fPolyline->endBulk(count);
    }
    fOutStart = fOut = fOutEnd = nullptr;
}

void StrokeFlatten::setup(float radius,
                          float tolerance,
                          SkPaint::Join joinType,
                          SkPaint::Cap capType,
                          float miterLimit) {
    if (radius != fRadius || tolerance != fTolerance) {
        fRadius = radius;
        fTolerance = tolerance;
        fSqrtTolerance = std::sqrt(tolerance);
        fCountScale = 0.5f / fSqrtTolerance;
        // Like Flatten, budget 10% of the tolerance for lowering cubics to quads and conics to
        // cubics. Here the quads only steer where samples go, the output is evaluated exactly.
        const float lowerTol = 0.1f * tolerance;
        fInvCubicErr = inv_cubic_err_threshold(lowerTol);
        fInvConicTol = 1.0f / lowerTol;
        // The largest rotation of the normal per segment for which the chord stays within
        // 'tolerance' of a circle of radius r. Once that reaches 180 degrees (r <= tol), any
        // rotation between two samples is within tolerance.
        const float cosHalf = 1.0f - tolerance / radius;
        fRadialDelta = cosHalf > -1.0f ? 2.0f * std::acos(cosHalf) : 2.0f * SK_ScalarPI;
        if (!(fRadialDelta > 0)) {
            // 1 - tolerance / radius rounded to 1 (a huge radius): acos(1 - x) = 2 asin(sqrt(x / 2)).
            fRadialDelta = 4.0f * std::asin(std::sqrt(0.5f * tolerance / radius));
        }
        fRotationLimited = fRadialDelta < SK_ScalarPI;
        fCosDelta = std::cos(fRadialDelta);
        fSinDelta = std::sin(fRadialDelta);
        fMinDot = fRotationLimited ? fCosDelta : -2.0f;
        // Clamped before converting, which would overflow for huge radii.
        fCapSegments =
                fRotationLimited
                        ? static_cast<int>(std::min(std::ceil(SK_ScalarPI / fRadialDelta), 1024.0f))
                        : 1;
        fMaxArcSteps = fRotationLimited ? fCapSegments + 1 : 0;
        fCapCos.resize(fCapSegments + 1);
        fCapSin.resize(fCapSegments + 1);
        for (int k = 0; k <= fCapSegments; ++k) {
            const float theta =
                    SK_ScalarPI * static_cast<float>(k) / static_cast<float>(fCapSegments);
            fCapCos[k] = std::cos(theta);
            fCapSin[k] = std::sin(theta);
        }
    }
    fCap = capType;
    // SkStroke bevels miter joins whose limit is <= 1.
    fJoin = (joinType == SkPaint::kMiter_Join && miterLimit <= 1.0f) ? SkPaint::kBevel_Join
                                                                     : joinType;
    // SkStroke miters iff sin(theta / 2) >= 1 / miterLimit, i.e. 1 + dot(n0, n1) >= 2 / limit^2.
    fMiterThreshold = fJoin == SkPaint::kMiter_Join ? 2.0f / (miterLimit * miterLimit) : 0.0f;
    fActiveJoin = fJoin;
}

void StrokeFlatten::resetContour() {
    fLeft.clear();
    fRight.clear();
    fSegmentCount = 0;
    fSawDegenerate = false;
    fActiveJoin = fJoin;
}

void StrokeFlatten::beginContour(SkPoint pt) {
    this->resetContour();
    fFirstPt = pt;
    fPrevPt = pt;
    fContourStarted = true;
}

void StrokeFlatten::finishContour(bool closed) {
    if (!fContourStarted) {
        return;
    }
    if (fSegmentCount == 0) {
        // Like SkStroke, zero-length contours (moveTo + zero-length segments, or moveTo + close)
        // draw a dot when the cap is round or square.
        if ((fSawDegenerate || closed) && fCap != SkPaint::kButt_Cap) {
            this->appendDot(fFirstPt, fCap == SkPaint::kRound_Cap);
        }
    } else if (closed) {
        if (fPrevPt != fFirstPt) {
            // SkPath::Iter normally emits the closing line itself.
            this->lineTo(fFirstPt);
        }
        this->join(fPrevU,
                   fFirstU,
                   fPrevIsLine,
                   fFirstIsLine,
                   fPrevLength,
                   fFirstLength,
                   /*closing=*/true);
        // Two closed loops: the left side, and the right side reversed.
        const int numLeft = fLeft.size();
        const int numRight = fRight.size();
        PointWriter writer(this->reserveOutput(numLeft + numRight + 5), fOutLast);
        writer.sentinel();
        writer.write(fLeft.data(), numLeft);
        writer.write(fLeft[0]);
        writer.sentinel();
        writer.write(fRight[0]);
        writer.writeReversed(fRight.data(), numRight);
        writer.sentinel();
        fOut = writer.dst();
        fOutLast = writer.last();
    } else {
        // One closed loop: left side forward, end cap, right side reversed, start cap.
        fScratch.clear();
        if (fCap != SkPaint::kButt_Cap) {
            this->appendCap(&fLeft, fPrevPt, fPrevU, /*atEnd=*/true);
            this->appendCap(&fScratch, fFirstPt, fFirstU, /*atEnd=*/false);
        }
        const int numLeft = fLeft.size();
        const int numRight = fRight.size();
        const int numStartCap = fScratch.size();
        PointWriter writer(this->reserveOutput(numLeft + numRight + numStartCap + 3), fOutLast);
        writer.sentinel();
        writer.write(fLeft.data(), numLeft);
        writer.writeReversed(fRight.data(), numRight);
        writer.write(fScratch.data(), numStartCap);
        writer.write(fLeft[0]);
        writer.sentinel();
        fOut = writer.dst();
        fOutLast = writer.last();
    }
    this->resetContour();
    fContourStarted = false;
}

void StrokeFlatten::lineTo(SkPoint pt, bool forceRoundJoin) {
    const SkVector d = pt - fPrevPt;
    const float lengthSq = vdot(d, d);
    if (!(lengthSq > kDegenerateLength * kDegenerateLength) || !std::isfinite(lengthSq)) {
        fSawDegenerate = true;
        return;
    }
    const float length = std::sqrt(lengthSq);
    const SkVector u = d * (1.0f / length);
    if (forceRoundJoin) {
        fActiveJoin = SkPaint::kRound_Join;
    }
    this->segmentStart(u, /*isLine=*/true, length);
    fActiveJoin = fJoin;
    const SkVector n = vperp(u) * fRadius;
    fLeft.push_back(pt + n);
    fRight.push_back(pt - n);
    fPrevPt = pt;
    fPrevU = u;
    fPrevIsLine = true;
    fPrevLength = length;
}

void StrokeFlatten::chordTo(SkPoint pt) {
    const SkVector d = pt - fPrevPt;
    const float lengthSq = vdot(d, d);
    if (!(lengthSq > kDegenerateLength * kDegenerateLength) || !std::isfinite(lengthSq)) {
        fSawDegenerate = true;
        return;
    }
    const SkVector u = d * (1.0f / std::sqrt(lengthSq));
    // As a curve, its joins don't take the inner miter shortcut, whose point can lie far from
    // the pivot along the neighboring segment.
    this->segmentStart(u, /*isLine=*/false, /*length=*/0.0f);
    const SkVector n = vperp(u) * fRadius;
    fLeft.push_back(pt + n);
    fRight.push_back(pt - n);
    fPrevPt = pt;
    fPrevU = u;
    fPrevIsLine = false;
    fPrevLength = 0.0f;
}

void StrokeFlatten::flushLinesImpl() {
    const int numPts = fLinePts.size();
    SkASSERT(numPts > 0);
    const int numLines = numPts - 1;
    if (numLines < kMinLineBatch) {
        for (int k = 1; k < numPts; ++k) {
            this->lineTo(fLinePts[k]);
        }
        fLinePts.clear();
        return;
    }

    using skvx::float8;
    // Line k goes from q[k - 1] to q[k]. Pad so that whole vectors can be read past the end.
    const int padded = numPts + kLanes;
    fLinePts.reserve(padded);
    SkPoint* q = fLinePts.data();
    for (int k = numPts; k < padded; ++k) {
        q[k] = q[numPts - 1];
    }
    fLineUX.resize(padded);
    fLineUY.resize(padded);
    fLineLength.resize(padded);
    fLineDot.resize(padded);
    fLineCross.resize(padded);
    fLineInvOnePlusDot.resize(padded);
    float* ux = fLineUX.data();
    float* uy = fLineUY.data();
    float* len = fLineLength.data();
    float* dot = fLineDot.data();
    float* cross = fLineCross.data();
    float* inv = fLineInvOnePlusDot.data();
    std::fill(ux, ux + padded, 0.0f);
    std::fill(uy, uy + padded, 0.0f);

    // Unit tangents and lengths of lines 1..numLines.
    for (int k = 1; k <= numLines; k += kLanes) {
        float8 sx, sy, ex, ey;
        load_points(q + k - 1, &sx, &sy);
        load_points(q + k, &ex, &ey);
        const float8 dx = ex - sx;
        const float8 dy = ey - sy;
        const float8 length = skvx::sqrt(skvx::fma(dx, dx, dy * dy));
        const float8 invLength = 1.0f / length;
        (dx * invLength).store(ux + k);
        (dy * invLength).store(uy + k);
        length.store(len + k);
    }
    // Join terms of lines k - 1 and k, for k = 2..numLines.
    for (int k = 2; k <= numLines; k += kLanes) {
        const float8 ax = float8::Load(ux + k - 1);
        const float8 ay = float8::Load(uy + k - 1);
        const float8 bx = float8::Load(ux + k);
        const float8 by = float8::Load(uy + k);
        const float8 d = skvx::fma(ax, bx, ay * by);
        d.store(dot + k);
        skvx::fma(ax, by, -(ay * bx)).store(cross + k);
        (1.0f / (1.0f + d)).store(inv + k);
    }

    // Joins and offsets, in order. Miter and bevel joins between two lines of the batch are
    // branch-free, since their data-dependent branches (which side is outer, miter or bevel,
    // inner intersection or pivot) mispredict on real data. Each chain either replaces its last
    // point (miter tip, inner intersection) or gets up to two points, then the line's end offset.
    const bool branchFree = fJoin != SkPaint::kRound_Join;
    fLeft.reserve(fLeft.size() + 3 * numLines + 3);
    fRight.reserve(fRight.size() + 3 * numLines + 3);
    int prev = -1;  // the last line of this batch that was stroked
    for (int k = 1; k <= numLines; ++k) {
        const SkPoint pt = q[k];
        if (q[k - 1] != fPrevPt) {
            // A degenerate line was skipped: measure from the actual previous point.
            this->lineTo(pt);
            prev = -1;
            continue;
        }
        const float length = len[k];
        if (!(length > kDegenerateLength) || !(length < kInfinity)) {
            fSawDegenerate = true;
            continue;
        }
        const SkVector u = {ux[k], uy[k]};
        const SkVector n = vperp(u) * fRadius;
        if (branchFree && fSegmentCount > 0 && prev == k - 1) {
            // joinTerms() for two lines, followed by the end offsets.
            const float d = dot[k];
            const float c = cross[k];
            const bool nearlyLine = d >= 0 && 1 - d <= SK_ScalarNearlyZero;
            const bool nearly180 = d < 0 && 1 + d <= SK_ScalarNearlyZero;
            const bool leftIsOuter = c < 0 || (c == 0 && d < 0);
            const float so = leftIsOuter ? fRadius : -fRadius;
            const float invD = nearly180 ? 0.0f : inv[k];
            const SkPoint pivot = fPrevPt;
            const SkVector miter = (vperp(fPrevU) + vperp(u)) * (invD * so);
            const SkVector side = vperp(u) * so;
            const bool tip = !nearlyLine && fJoin == SkPaint::kMiter_Join && !nearly180 &&
                             1 + d >= fMiterThreshold;
            const bool innerMiter =
                    !nearlyLine && !nearly180 &&
                    2 * fRadius * std::abs(c) * invD <= std::min(fPrevLength, length);
            const int outerCount = nearlyLine || tip ? 0 : 1;
            const int innerCount = nearlyLine || innerMiter ? 0 : 2;
            const int leftSize = fLeft.size();
            const int rightSize = fRight.size();
            SkPoint* outer = leftIsOuter ? fLeft.data() : fRight.data();
            SkPoint* inner = leftIsOuter ? fRight.data() : fLeft.data();
            const int outerSize = leftIsOuter ? leftSize : rightSize;
            const int innerSize = leftIsOuter ? rightSize : leftSize;
            outer[outerSize - 1] = tip ? pivot + miter : outer[outerSize - 1];
            inner[innerSize - 1] = innerMiter ? pivot - miter : inner[innerSize - 1];
            outer[outerSize] = pivot + side;
            inner[innerSize] = pivot;
            inner[innerSize + 1] = pivot - side;
            outer[outerSize + outerCount] = leftIsOuter ? pt + n : pt - n;
            inner[innerSize + innerCount] = leftIsOuter ? pt - n : pt + n;
            const int outerEnd = outerSize + outerCount + 1;
            const int innerEnd = innerSize + innerCount + 1;
            fLeft.resize(leftIsOuter ? outerEnd : innerEnd);
            fRight.resize(leftIsOuter ? innerEnd : outerEnd);
            ++fSegmentCount;
        } else {
            if (fSegmentCount > 0 && prev == k - 1) {
                this->joinTerms(fPrevU,
                                u,
                                dot[k],
                                cross[k],
                                inv[k],
                                /*prevIsLine=*/true,
                                /*currIsLine=*/true,
                                fPrevLength,
                                length,
                                /*closing=*/false);
                ++fSegmentCount;
            } else {
                this->segmentStart(u, /*isLine=*/true, length);
            }
            fLeft.push_back(pt + n);
            fRight.push_back(pt - n);
        }
        fPrevPt = pt;
        fPrevU = u;
        fPrevIsLine = true;
        fPrevLength = length;
        prev = k;
    }
    fLinePts.clear();
}

void StrokeFlatten::quadTo(const SkPoint pts[3]) {
    // Degenerate and flat quads are handled like SkStroke::CheckQuadLinear().
    const bool degenerateAB = degenerate_vector(pts[1] - pts[0]);
    const bool degenerateBC = degenerate_vector(pts[2] - pts[1]);
    if (degenerateAB && degenerateBC) {
        fSawDegenerate = true;
        return;
    }
    if (degenerateAB || degenerateBC) {
        this->lineTo(pts[2]);
        return;
    }
    if (maybe_in_line3(pts, kQuadLineSlop) && points_in_line<3>(pts, kQuadLineSlop)) {
        const float t = SkFindQuadMaxCurvature(pts);
        if (t > 0 && t < 1) {
            this->lineTo(SkEvalQuadAt(pts, t));
            this->lineTo(pts[2], /*forceRoundJoin=*/true);
        } else {
            this->lineTo(pts[2]);
        }
        return;
    }
    // A quadratic never inflects nor turns 180 degrees, so it is a run on its own.
    this->strokeQuad(pts);
}

void StrokeFlatten::conicTo(const SkPoint pts[3], float weight) {
    // Degenerate and flat conics are handled like SkStroke::CheckConicLinear().
    const bool degenerateAB = degenerate_vector(pts[1] - pts[0]);
    const bool degenerateBC = degenerate_vector(pts[2] - pts[1]);
    if (degenerateAB && degenerateBC) {
        fSawDegenerate = true;
        return;
    }
    if (degenerateAB || degenerateBC) {
        this->lineTo(pts[2]);
        return;
    }
    if (maybe_in_line3(pts, kQuadLineSlop) && points_in_line<3>(pts, kQuadLineSlop)) {
        // As in SkStroke, quad curvature is a reasonable substitute for conic curvature.
        const float t = SkFindQuadMaxCurvature(pts);
        if (t > 0 && t < 1) {
            this->lineTo(SkConic(pts, weight).evalAt(t));
            this->lineTo(pts[2], /*forceRoundJoin=*/true);
        } else {
            this->lineTo(pts[2]);
        }
        return;
    }
    if (this->strokeCircularArc(pts, weight)) {
        return;
    }
    // Each cubic approximates its conic piece within 10% of the tolerance and turns well under
    // 180 degrees, so the pieces can be stroked as one run of exact curves.
    SkPoint cubics[Flatten::kMaxCubicsFromConic][4];
    const int count = static_cast<int>(split_conic_to_cubics(pts, weight, fInvConicTol, cubics));
    this->strokeRun(&cubics[0][0], count, /*stride=*/4);
}

bool StrokeFlatten::strokeCircularArc(const SkPoint pts[3], float weight) {
    CircularArc arc;
    if (!CircularArc::Make(pts, weight, &arc)) {
        return false;
    }
    this->segmentStart(arc.fU0, /*isLine=*/false, /*length=*/0.0f);

    // Uniform angular steps, as many as the outer offset (radius R + r) needs.
    const int n = arc.steps(arc.fRadius + fRadius, fTolerance);
    if (n > 1) {
        // With v the unit vector from the center, the tangent is turn * perp(v). So the left
        // offset, P + r perp(T), is C + v (R - turn r) and the right one is C + v (R + turn r).
        const int count = n - 1;
        const int leftBase = fLeft.size();
        const int rightBase = fRight.size();
        // sample() writes whole vectors; the padding past 'count' is dropped by the resize below.
        fLeft.reserve(leftBase + count + kLanes);
        fRight.reserve(rightBase + count + kLanes);
        arc.sample<2>(n,
                      {arc.fRadius - arc.fTurn * fRadius, arc.fRadius + arc.fTurn * fRadius},
                      {fLeft.data() + leftBase, fRight.data() + rightBase});
        fLeft.resize(leftBase + count);
        fRight.resize(rightBase + count);
    }
    const SkVector offset = vperp(arc.fU1) * fRadius;
    fLeft.push_back(pts[2] + offset);
    fRight.push_back(pts[2] - offset);
    fPrevPt = pts[2];
    fPrevU = arc.fU1;
    fPrevIsLine = false;
    fPrevLength = 0.0f;
    return true;
}

void StrokeFlatten::cubicTo(const SkPoint pts[4]) {
    // Degenerate and flat cubics are handled like SkStroke::CheckCubicLinear().
    const bool degenerateAB = degenerate_vector(pts[1] - pts[0]);
    const bool degenerateBC = degenerate_vector(pts[2] - pts[1]);
    const bool degenerateCD = degenerate_vector(pts[3] - pts[2]);
    if (degenerateAB && degenerateBC && degenerateCD) {
        fSawDegenerate = true;
        return;
    }
    if (static_cast<int>(degenerateAB) + static_cast<int>(degenerateBC) +
                static_cast<int>(degenerateCD) ==
        2) {
        this->lineTo(pts[3]);
        return;
    }
    if (maybe_in_line4(pts, kCubicLineSlop) && points_in_line<4>(pts, kCubicLineSlop)) {
        // A flat cubic may double back on itself: stroke lines through its maximum curvature
        // points with round joins at the turnarounds, like SkStroke.
        float tValues[3];
        const int count = SkFindCubicMaxCurvature(pts, tValues);
        const CubicCoeffs cubic(pts);
        SkPoint reduction[3];
        int reductionCount = 0;
        for (int i = 0; i < count; ++i) {
            const float t = tValues[i];
            if (!(t > 0 && t < 1)) {
                continue;
            }
            const SkPoint pt = cubic.eval(t);
            if (pt != pts[0] && pt != pts[3]) {
                reduction[reductionCount++] = pt;
            }
        }
        if (reductionCount == 0) {
            this->lineTo(pts[3]);
            return;
        }
        this->lineTo(reduction[0]);
        for (int i = 1; i < reductionCount; ++i) {
            this->lineTo(reduction[i], /*forceRoundJoin=*/true);
        }
        this->lineTo(pts[3], /*forceRoundJoin=*/true);
        return;
    }

    float chopT[2];
    bool areCusps = false;
    const int numChops = skgpu::tess::FindCubicConvex180Chops(pts, chopT, &areCusps);
    if (numChops == 0) {
        this->strokeRun(pts, /*count=*/1, /*stride=*/4);
    } else if (numChops == 1) {
        SkPoint chops[7];
        SkChopCubicAt(pts, chops, chopT[0]);
        if (areCusps) {
            // Like SkStroke and the tessellating stroker, cusps get a round dot. Collapsing the
            // control points onto the cusp makes the tangents there come from the outer points.
            this->appendDot(chops[3], /*round=*/true);
            chops[2] = chops[3];
            chops[4] = chops[3];
            this->strokeRun(chops, /*count=*/1, /*stride=*/3);
            this->strokeRun(chops + 3, /*count=*/1, /*stride=*/3);
        } else {
            this->strokeRun(chops, /*count=*/2, /*stride=*/3);
        }
    } else {
        SkPoint chops[10];
        SkChopCubicAt(pts, chops, chopT[0], chopT[1]);
        if (areCusps) {
            // Two cusps are only possible on a flat line with two 180 degree turnarounds.
            this->appendDot(chops[3], /*round=*/true);
            this->appendDot(chops[6], /*round=*/true);
            this->lineTo(chops[3]);
            this->lineTo(chops[6]);
            this->lineTo(chops[9]);
        } else {
            this->strokeRun(chops, /*count=*/3, /*stride=*/3);
        }
    }
}

void StrokeFlatten::strokeRun(const SkPoint* cubics, int count, int stride) {
    SkASSERT(count >= 1 && count <= kMaxRunCubics);
    const SkPoint* lastCubic = cubics + (count - 1) * stride;
    const SkVector d0 = start_tangent(cubics);
    const SkVector d1 = end_tangent(lastCubic);
    const float l0 = vlength(d0);
    const float l1 = vlength(d1);
    if (!(l0 > 0) || !(l1 > 0) || !std::isfinite(l0) || !std::isfinite(l1)) {
        fSawDegenerate = true;
        return;
    }
    const SkVector u0 = d0 * (1.0f / l0);
    const SkVector u1 = d1 * (1.0f / l1);
    this->segmentStart(u0, /*isLine=*/false, /*length=*/0.0f);

    // Distribute the samples jointly over all cubics of the run.
    this->addRunPieces(cubics, count, stride);
    float sumWeights = 0.0f;
    for (int p = 0; p < fPieces.size(); ++p) {
        sumWeights += fPieces[p].fWeight;
    }
    const int numInterior = this->generateSamples(count, sumWeights);
    this->finishRun(cubics, count, stride, numInterior, u0, u1);
}

void StrokeFlatten::strokeQuad(const SkPoint pts[3]) {
    // quadTo() rejected zero-length legs, so the end tangents are the legs.
    const SkVector d0 = pts[1] - pts[0];
    const SkVector d1 = pts[2] - pts[1];
    const float l0 = vlength(d0);
    const float l1 = vlength(d1);
    if (!(l0 > 0) || !(l1 > 0) || !std::isfinite(l0) || !std::isfinite(l1)) {
        fSawDegenerate = true;
        return;
    }
    const SkVector u0 = d0 * (1.0f / l0);
    const SkVector u1 = d1 * (1.0f / l1);
    this->segmentStart(u0, /*isLine=*/false, /*length=*/0.0f);

    // The quad steers its own samples, with the same parameterization.
    fPieces.clear();
    this->addQuadPieces({pts[0], pts[1], pts[2], /*fT0=*/0.0f, /*fT1=*/1.0f, /*fCubic=*/0});
    float sumWeights = 0.0f;
    for (int p = 0; p < fPieces.size(); ++p) {
        sumWeights += fPieces[p].fWeight;
    }
    const int numInterior = this->generateSamples(/*count=*/1, sumWeights);
    // The evaluation works on the quad's exact cubic representation.
    const SkPoint cubic[4] = {
            pts[0], pts[0] + d0 * (2.0f / 3.0f), pts[2] - d1 * (2.0f / 3.0f), pts[2]};
    this->finishRun(cubic, /*count=*/1, /*stride=*/4, numInterior, u0, u1);
}

void StrokeFlatten::finishRun(
        const SkPoint* cubics, int count, int stride, int numInterior, SkVector u0, SkVector u1) {
    const SkPoint* lastCubic = cubics + (count - 1) * stride;

    // Evaluate the interior samples on their exact cubics, writing both offsets straight into
    // the chains. The normals go to fNormalX/Y[1..numInterior] for the rotation check, between
    // the run's start and end normals.
    const SkVector n0 = vperp(u0);
    const SkVector n1 = vperp(u1);
    const int leftBase = fLeft.size();
    const int rightBase = fRight.size();
    fLeft.reserve(leftBase + numInterior + kLanes + 1);
    fRight.reserve(rightBase + numInterior + kLanes + 1);
    const int numNormals = numInterior + 2 + kLanes;
    fNormalX.resize(numNormals);
    fNormalY.resize(numNormals);
    fNormalX[0] = n0.fX;
    fNormalY[0] = n0.fY;
    SkPoint* left = fLeft.data() + leftBase;
    SkPoint* right = fRight.data() + rightBase;
    bool evaluated = true;  // false if a derivative was too short to normalize
    int begin = 0;
    for (int j = 0; j < count; ++j) {
        const int end = fCubicSampleEnd[j];
        if (fSimd && end - begin >= kMinSimdSamples) {
            const bool ok = this->evaluateSimd(cubics + j * stride, begin, end, left, right);
            evaluated = evaluated && ok;
        } else if (end > begin) {
            this->evaluateScalar(cubics + j * stride, begin, end, left, right);
        }
        begin = end;
    }
    // The end normal, and padding that passes the rotation check. Written last, since the SIMD
    // evaluation may write past its samples.
    for (int k = numInterior + 1; k < numNormals; ++k) {
        fNormalX[k] = n1.fX;
        fNormalY[k] = n1.fY;
    }
    const bool rotationsOk =
            evaluated &&
            (fSimd ? rotations_within_simd(
                             fNormalX.data(), fNormalY.data(), numInterior + 1, fMinDot)
                   : rotations_within(fNormalX.data(), fNormalY.data(), numInterior + 1, fMinDot));
    if (rotationsOk) {
        fLeft.resize(leftBase + numInterior);
        fRight.resize(rightBase + numInterior);
    } else if (evaluated) {
        this->refineRun(cubics, count, stride, leftBase, rightBase, u0, u1);
    } else {
        fLeft.resize(leftBase);
        fRight.resize(rightBase);
        this->strokeRunSlow(cubics, count, stride, u0, u1);
    }

    const SkPoint end = lastCubic[3];
    const SkVector offset = n1 * fRadius;
    fLeft.push_back(end + offset);
    fRight.push_back(end - offset);
    fPrevPt = end;
    fPrevU = u1;
    fPrevIsLine = false;
    fPrevLength = 0.0f;
}

void StrokeFlatten::addRunPieces(const SkPoint* cubics, int count, int stride) {
    fPieces.clear();
    // Lower the cubics to quads like Flatten: they only steer where samples go.
    fQuads.clear();
    for (int j = 0; j < count; ++j) {
        const SkPoint* pts = cubics + j * stride;
        const CubicCoeffs coeffs(pts);
        const int cubicQuads = static_cast<int>(estimate_num_quads_from_cubic(pts, fInvCubicErr));
        const float dt = 1.0f / static_cast<float>(cubicQuads);
        SkPoint q0 = pts[0];
        for (int i = 0; i < cubicQuads; ++i) {
            const bool last = i + 1 == cubicQuads;
            const float t0 = static_cast<float>(i) * dt;
            const float t1 = last ? 1.0f : static_cast<float>(i + 1) * dt;
            const SkPoint q2 = last ? pts[3] : coeffs.eval(t1);
            const SkPoint qm = coeffs.eval(0.5f * (t0 + t1));
            fQuads.push_back({q0, qm * 2.0f - (q0 + q2) * 0.5f, q2, t0, t1, j});
            q0 = q2;
        }
    }
    const int numQuads = fQuads.size();
    if (fSimd && numQuads >= kMinSimdQuads) {
        for (int i = 0; i < numQuads; i += 4) {
            this->addQuadPiecesSimd(i, std::min(4, numQuads - i));
        }
    } else {
        for (int i = 0; i < numQuads; ++i) {
            this->addQuadPieces(fQuads[i]);
        }
    }
}

void StrokeFlatten::addQuadPieces(const Quad& quad) {
    // Map the quad onto the normalized parabola y = x^2, x in [x0, x2], scaled by 'scale'
    // (Flatten's estimate_lines_from_quad()).
    const SkVector d01 = quad.fP1 - quad.fP0;
    const SkVector d12 = quad.fP2 - quad.fP1;
    const SkVector dd = d01 - d12;
    const float crs = vcross(quad.fP2 - quad.fP0, dd);
    const float ddSq = vdot(dd, dd);
    if (!(std::abs(crs) > kEpsilon) || !(ddSq > 0)) {
        return;  // Straight: needs no interior samples.
    }
    const float invCrs = 1.0f / crs;
    const float x0 = vdot(d01, dd) * invCrs;
    const float x2 = vdot(d12, dd) * invCrs;
    const float scale = (crs * crs) / (std::sqrt(ddSq) * ddSq);
    if (!(scale > 0) || !(scale < kInfinity) || !std::isfinite(x0) || !std::isfinite(x2) ||
        x0 == x2) {
        return;
    }
    // Curvature at normalized coordinate x: k(x) = 2 / (scale * (1 + 4 x^2)^1.5). It peaks at the
    // vertex (x = 0), so the end nearest the vertex has the maximum.
    const float kappaScale = 2.0f / scale;
    const bool straddles = (x0 < 0) != (x2 < 0);
    const float xNearest = straddles ? 0.0f : std::min(std::abs(x0), std::abs(x2));
    const float qNearest = 1.0f + 4.0f * xNearest * xNearest;
    const float kNearest = kappaScale / (qNearest * std::sqrt(qNearest));
    const float i0 = approx_parabola_integral(x0);
    const float i2 = approx_parabola_integral(x2);
    QuadMap map = {x0,
                   quad.fT0,
                   (quad.fT1 - quad.fT0) / (x2 - x0),
                   std::sqrt(scale),
                   kappaScale,
                   /*fVertexScale=*/-1.0f,
                   quad.fCubic};
    this->addMappedQuadPieces(&map,
                              x2,
                              i0,
                              i2,
                              approx_parabola_inv_integral(i0),
                              approx_parabola_inv_integral(i2),
                              kNearest,
                              fRadius * kNearest > kSplitThreshold);
}

void StrokeFlatten::addQuadPiecesSimd(int first, int lanes) {
    using skvx::float4;
    using skvx::int4;
    // addQuadPieces() for up to 4 quads at once; unused lanes repeat the last quad.
    const Quad* quads = fQuads.data() + first;
    const Quad& qa = quads[0];
    const Quad& qb = quads[std::min(1, lanes - 1)];
    const Quad& qc = quads[std::min(2, lanes - 1)];
    const Quad& qd = quads[std::min(3, lanes - 1)];
    const float4 p0x(qa.fP0.fX, qb.fP0.fX, qc.fP0.fX, qd.fP0.fX);
    const float4 p0y(qa.fP0.fY, qb.fP0.fY, qc.fP0.fY, qd.fP0.fY);
    const float4 p1x(qa.fP1.fX, qb.fP1.fX, qc.fP1.fX, qd.fP1.fX);
    const float4 p1y(qa.fP1.fY, qb.fP1.fY, qc.fP1.fY, qd.fP1.fY);
    const float4 p2x(qa.fP2.fX, qb.fP2.fX, qc.fP2.fX, qd.fP2.fX);
    const float4 p2y(qa.fP2.fY, qb.fP2.fY, qc.fP2.fY, qd.fP2.fY);
    const float4 t0(qa.fT0, qb.fT0, qc.fT0, qd.fT0);
    const float4 t1(qa.fT1, qb.fT1, qc.fT1, qd.fT1);

    const float4 d01x = p1x - p0x;
    const float4 d01y = p1y - p0y;
    const float4 d12x = p2x - p1x;
    const float4 d12y = p2y - p1y;
    const float4 ddx = d01x - d12x;
    const float4 ddy = d01y - d12y;
    const float4 crs = (p2x - p0x) * ddy - (p2y - p0y) * ddx;
    const float4 ddSq = ddx * ddx + ddy * ddy;
    const float4 invCrs = 1.0f / crs;
    const float4 x0 = (d01x * ddx + d01y * ddy) * invCrs;
    const float4 x2 = (d12x * ddx + d12y * ddy) * invCrs;
    const float4 scale = (crs * crs) / (skvx::sqrt(ddSq) * ddSq);
    const int4 mapped = (skvx::abs(crs) > kEpsilon) & (ddSq > 0.0f) & (scale > 0.0f) &
                        (scale < kInfinity) & (skvx::abs(x0) < kInfinity) &
                        (skvx::abs(x2) < kInfinity) & (x0 != x2);
    const float4 sqrtScale = skvx::sqrt(scale);
    const float4 kappaScale = 2.0f / scale;
    const int4 straddles = (x0 < 0.0f) ^ (x2 < 0.0f);
    const float4 xNearest =
            skvx::if_then_else(straddles, float4(0.0f), skvx::min(skvx::abs(x0), skvx::abs(x2)));
    const float4 qNearest = 1.0f + 4.0f * xNearest * xNearest;
    const float4 kNearest = kappaScale / (qNearest * skvx::sqrt(qNearest));
    const int4 split = fRadius * kNearest > kSplitThreshold;
    const float4 i0 = approx_parabola_integral(x0);
    const float4 i2 = approx_parabola_integral(x2);
    const float4 u0 = approx_parabola_inv_integral(i0);
    const float4 u2 = approx_parabola_inv_integral(i2);
    const float4 vertexScale =
            fSqrtTolerance / approx_parabola_integral(fSqrtTolerance / sqrtScale);

    // The unsplit piece, as in pushPiece().
    const float4 val = skvx::abs(i2 - i0) * skvx::if_then_else(straddles, vertexScale, sqrtScale);
    const float4 weight = val * skvx::sqrt(1.0f + fRadius * kNearest);
    const float4 slope = (i2 - i0) / weight;
    const float4 du = u2 - u0;
    const float4 beta = (t1 - t0) / du;
    const float4 alpha = t0 - beta * u0;
    const int4 whole =
            mapped & ~split & (skvx::abs(du) > 0.0f) & (weight > 0.0f) & (weight < kInfinity);

    for (int l = 0; l < lanes; ++l) {
        if (whole[l]) {
            fPieces.push_back({i0[l], slope[l], alpha[l], beta[l], weight[l], quads[l].fCubic});
        } else if (mapped[l] && split[l]) {
            QuadMap map = {x0[l],
                           t0[l],
                           (t1[l] - t0[l]) / (x2[l] - x0[l]),
                           sqrtScale[l],
                           kappaScale[l],
                           vertexScale[l],
                           quads[l].fCubic};
            this->addMappedQuadPieces(&map,
                                      x2[l],
                                      i0[l],
                                      i2[l],
                                      u0[l],
                                      u2[l],
                                      kNearest[l],
                                      /*split=*/true);
        }
    }
}

void StrokeFlatten::addMappedQuadPieces(QuadMap* map,
                                        float x2,
                                        float i0,
                                        float i2,
                                        float u0,
                                        float u2,
                                        float kNearest,
                                        bool split) {
    const float x0 = map->fX0;
    if (!split) {
        this->pushPiece(map, x0, x2, i0, i2, u0, u2, kNearest);
        return;
    }
    // Split at the fixed angles strictly between x0 and x2. Their integrals and curvatures come
    // from tables. Pieces never contain the vertex (x = 0 is a split), so a piece's maximum
    // curvature is at one of its ends.
    const float kappaScale = map->fKappaScale;
    auto kappa = [kappaScale](float x) {
        const float q = 1.0f + 4.0f * x * x;
        return kappaScale / (q * std::sqrt(q));
    };
    float prevX = x0;
    float prevI = i0;
    float prevU = u0;
    float prevK = kappa(x0);
    auto cut = [&](float x, float i, float u, float k) {
        this->pushPiece(map, prevX, x, prevI, i, prevU, u, std::max(prevK, k));
        prevX = x;
        prevI = i;
        prevU = u;
        prevK = k;
    };
    if (x0 < x2) {
        for (int j = 0; j < kNumSplits; ++j) {
            if (kSplitX[j] > x0 && kSplitX[j] < x2) {
                cut(kSplitX[j],
                    fSplitIntegral[j],
                    fSplitInvIntegral[j],
                    kappaScale * fSplitCos3[j]);
            }
        }
    } else {
        for (int j = kNumSplits - 1; j >= 0; --j) {
            if (kSplitX[j] < x0 && kSplitX[j] > x2) {
                cut(kSplitX[j],
                    fSplitIntegral[j],
                    fSplitInvIntegral[j],
                    kappaScale * fSplitCos3[j]);
            }
        }
    }
    cut(x2, i2, u2, kappa(x2));
}

void StrokeFlatten::pushPiece(
        QuadMap* map, float xa, float xb, float ia, float ib, float ua, float ub, float kMax) {
    if (!(std::abs(ub - ua) > 0)) {
        return;
    }
    // Flatten's curvature integral for the piece, including Levien's adjustment for pieces that
    // contain the parabola's vertex...
    float val = std::abs(ib - ia);
    if ((xa < 0) != (xb < 0)) {
        if (map->fVertexScale < 0) {
            map->fVertexScale =
                    fSqrtTolerance / approx_parabola_integral(fSqrtTolerance / map->fSqrtScale);
        }
        val *= map->fVertexScale;
    } else {
        val *= map->fSqrtScale;
    }
    // ...scaled for the offset curve. The outer offset needs sqrt(1 + r k) times the samples of
    // the centerline at the same curvature. Using the piece's maximum curvature keeps the error
    // within tolerance. A piece of the inner side folds into a swallowtail iff r * kMax > 1 (see
    // the class comment).
    const float weight = val * std::sqrt(1.0f + fRadius * kMax);
    if (!(weight > 0) || !(weight < kInfinity)) {
        return;
    }
    const float ta = map->fT0 + (xa - map->fX0) * map->fTScale;
    const float tb = map->fT0 + (xb - map->fX0) * map->fTScale;
    const float beta = (tb - ta) / (ub - ua);
    fPieces.push_back({ia, (ib - ia) / weight, ta - beta * ua, beta, weight, map->fCubic});
}

int StrokeFlatten::generateSamples(int count, float sumWeights) {
    int n = 1;
    if (sumWeights > 0 && std::isfinite(sumWeights)) {
        const float segments = std::ceil(sumWeights * fCountScale);
        n = static_cast<int>(std::clamp(segments, 1.0f, static_cast<float>(kMaxSegmentsPerCurve)));
    }
    const int numInterior = n - 1;
    fSampleA.resize(numInterior + kLanes);
    fSampleAlpha.resize(numInterior + kLanes);
    fSampleBeta.resize(numInterior + kLanes);
    float* a = fSampleA.data();
    float* alpha = fSampleAlpha.data();
    float* beta = fSampleBeta.data();
    if (fPieces.size() == 1) {
        // A single piece: the samples are uniform in its parabola integral. All belong to the
        // piece's cubic.
        const Piece& piece = fPieces[0];
        for (int j = 0; j < count; ++j) {
            fCubicSampleEnd[j] = j < piece.fCubic ? 0 : numInterior;
        }
        // The integral changes by fSlope * fWeight over the piece, and sumWeights == fWeight.
        const float da = piece.fSlope * piece.fWeight / static_cast<float>(n);
        using skvx::float8;
        const float8 index = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
        // Whole vectors: this covers the reads of the SIMD evaluation past numInterior with
        // samples just past the piece, which evaluate within [0, 1] like any other.
        for (int k = 0; k < numInterior; k += kLanes) {
            skvx::fma(index + static_cast<float>(k), float8(da), float8(piece.fA0)).store(a + k);
            float8(piece.fAlpha).store(alpha + k);
            float8(piece.fBeta).store(beta + k);
        }
        return numInterior;
    }
    // Samples are generated in cubic order, so the cubics before the current piece's are done.
    int k = 0;
    int doneCubics = 0;
    auto finishCubicsBefore = [&](int cubic) {
        for (; doneCubics < cubic; ++doneCubics) {
            fCubicSampleEnd[doneCubics] = k;
        }
    };
    if (numInterior > 0) {
        // Walk the pieces with a uniform step in (weighted) parabola-integral space, as Flatten
        // does across the quads of a cubic. Interior sample k sits at (k + 1) * step.
        const float step = sumWeights / static_cast<float>(n);
        float cum = 0.0f;
        for (int p = 0; p < fPieces.size() && k < numInterior; ++p) {
            const Piece& piece = fPieces[p];
            finishCubicsBefore(piece.fCubic);
            const float pieceEnd = cum + piece.fWeight;
            float target = static_cast<float>(k + 1) * step;
            while (k < numInterior && target <= pieceEnd) {
                a[k] = piece.fA0 + piece.fSlope * (target - cum);
                alpha[k] = piece.fAlpha;
                beta[k] = piece.fBeta;
                ++k;
                target = static_cast<float>(k + 1) * step;
            }
            cum = pieceEnd;
        }
        // Float drift can leave a target or two past the last piece: put them at its end.
        if (k < numInterior) {
            const Piece& piece = fPieces.back();
            finishCubicsBefore(piece.fCubic);
            for (; k < numInterior; ++k) {
                a[k] = piece.fA0 + piece.fSlope * piece.fWeight;
                alpha[k] = piece.fAlpha;
                beta[k] = piece.fBeta;
            }
        }
    }
    finishCubicsBefore(count);
    // Padding read by the SIMD evaluation: harmless samples at t = 0.
    static_assert(kLanes == 8);
    skvx::float8(0.0f).store(a + numInterior);
    skvx::float8(0.0f).store(alpha + numInterior);
    skvx::float8(0.0f).store(beta + numInterior);
    return numInterior;
}

void StrokeFlatten::evaluateScalar(
        const SkPoint pts[4], int begin, int end, SkPoint* left, SkPoint* right) {
    const CubicCoeffs cubic(pts);
    const float* a = fSampleA.data();
    const float* alpha = fSampleAlpha.data();
    const float* beta = fSampleBeta.data();
    float* nx = fNormalX.data();
    float* ny = fNormalY.data();
    for (int k = begin; k < end; ++k) {
        const float u = approx_parabola_inv_integral(a[k]);
        const float t = std::clamp(alpha[k] + beta[k] * u, 0.0f, 1.0f);
        const SkPoint pt = cubic.eval(t);
        // Normal k + 1 belongs to sample k, normal k to the previous sample (or the run start).
        const SkVector normal = unit_normal_or(cubic.deriv(t), {nx[k], ny[k]});
        nx[k + 1] = normal.fX;
        ny[k + 1] = normal.fY;
        const SkVector offset = normal * fRadius;
        left[k] = pt + offset;
        right[k] = pt - offset;
    }
}

bool StrokeFlatten::evaluateSimd(
        const SkPoint pts[4], int begin, int end, SkPoint* left, SkPoint* right) {
    using skvx::float8;
    using skvx::int8;
    const CubicCoeffs cubic(pts);
    const float8 ax(cubic.fA.fX);
    const float8 ay(cubic.fA.fY);
    const float8 bx(cubic.fB.fX);
    const float8 by(cubic.fB.fY);
    const float8 cx(cubic.fC.fX);
    const float8 cy(cubic.fC.fY);
    const float8 dx(cubic.fD.fX);
    const float8 dy(cubic.fD.fY);
    const float8 ax3 = ax * 3.0f;
    const float8 ay3 = ay * 3.0f;
    const float8 bx2 = bx * 2.0f;
    const float8 by2 = by * 2.0f;
    const float8 radius(fRadius);
    const int8 iota = {0, 1, 2, 3, 4, 5, 6, 7};
    int8 degenerate(0);
    float* nx = fNormalX.data() + 1;
    float* ny = fNormalY.data() + 1;
    // The last iteration may evaluate up to kLanes - 1 samples past 'end'. Their results land in
    // padding or are overwritten by the next cubic.
    for (int k = begin; k < end; k += kLanes) {
        const float8 a = float8::Load(fSampleA.data() + k);
        const float8 alpha = float8::Load(fSampleAlpha.data() + k);
        const float8 beta = float8::Load(fSampleBeta.data() + k);
        const float8 u = approx_parabola_inv_integral(a);
        const float8 t = skvx::min(skvx::max(skvx::fma(beta, u, alpha), 0.0f), 1.0f);
        const float8 px = skvx::fma(skvx::fma(skvx::fma(ax, t, bx), t, cx), t, dx);
        const float8 py = skvx::fma(skvx::fma(skvx::fma(ay, t, by), t, cy), t, dy);
        const float8 tx = skvx::fma(skvx::fma(ax3, t, bx2), t, cx);
        const float8 ty = skvx::fma(skvx::fma(ay3, t, by2), t, cy);
        const float8 lengthSq = skvx::fma(tx, tx, ty * ty);
        degenerate = degenerate | ((lengthSq < kMinDerivLengthSq) & (iota < end - k));
        const float8 invLength = 1.0f / skvx::sqrt(lengthSq);
        const float8 normalX = -ty * invLength;
        const float8 normalY = tx * invLength;
        normalX.store(nx + k);
        normalY.store(ny + k);
        const float8 ox = normalX * radius;
        const float8 oy = normalY * radius;
        store_points(left + k, px + ox, py + oy);
        store_points(right + k, px - ox, py - oy);
    }
    return !skvx::any(degenerate);
}

void StrokeFlatten::strokeRunSlow(
        const SkPoint* cubics, int count, int stride, SkVector u0, SkVector u1) {
    // Rebuild the samples robustly, bracketing every seam between two cubics with the end of the
    // one and the start of the other, so that every interval between samples lies within a
    // single cubic.
    fSamples.clear();
    SkVector prevNormal = vperp(u0);
    fSamples.push_back({cubics[0], prevNormal, 0.0f, 0});
    int begin = 0;
    for (int j = 0; j < count; ++j) {
        const SkPoint* pts = cubics + j * stride;
        if (j > 0) {
            const SkPoint* prevPts = cubics + (j - 1) * stride;
            prevNormal = unit_normal_or(end_tangent(prevPts), prevNormal);
            fSamples.push_back({prevPts[3], prevNormal, 1.0f, j - 1});
            prevNormal = unit_normal_or(start_tangent(pts), prevNormal);
            fSamples.push_back({pts[0], prevNormal, 0.0f, j});
        }
        const CubicCoeffs cubic(pts);
        const int end = fCubicSampleEnd[j];
        for (int k = begin; k < end; ++k) {
            const float u = approx_parabola_inv_integral(fSampleA[k]);
            const float t = std::clamp(fSampleAlpha[k] + fSampleBeta[k] * u, 0.0f, 1.0f);
            prevNormal = unit_normal_or(cubic.deriv(t), prevNormal);
            fSamples.push_back({cubic.eval(t), prevNormal, t, j});
        }
        begin = end;
    }
    const SkPoint* lastCubic = cubics + (count - 1) * stride;
    fSamples.push_back({lastCubic[3], vperp(u1), 1.0f, count - 1});

    if (fRotationLimited) {
        this->refineRotation(cubics, stride);
    }

    // segmentStart() emitted the first sample's offsets, strokeRun() emits the last's.
    for (int k = 1; k + 1 < fSamples.size(); ++k) {
        const Sample& s = fSamples[k];
        const SkVector offset = s.fNormal * fRadius;
        fLeft.push_back(s.fPt + offset);
        fRight.push_back(s.fPt - offset);
    }
}

void StrokeFlatten::refineRun(const SkPoint* cubics,
                              int count,
                              int stride,
                              int leftBase,
                              int rightBase,
                              SkVector u0,
                              SkVector u1) {
    // The samples as evaluated, written past the ends of fLeft and fRight: their points are the
    // midpoints of their offsets.
    const SkPoint* left = fLeft.data() + leftBase;
    const SkPoint* right = fRight.data() + rightBase;
    fSamples.clear();
    fSamples.push_back({cubics[0], vperp(u0), 0.0f, 0});
    int begin = 0;
    for (int j = 0; j < count; ++j) {
        const int end = fCubicSampleEnd[j];
        for (int k = begin; k < end; ++k) {
            const float u = approx_parabola_inv_integral(fSampleA[k]);
            const float t = std::clamp(fSampleAlpha[k] + fSampleBeta[k] * u, 0.0f, 1.0f);
            fSamples.push_back(
                    {(left[k] + right[k]) * 0.5f, {fNormalX[k + 1], fNormalY[k + 1]}, t, j});
        }
        begin = end;
    }
    const SkPoint* lastCubic = cubics + (count - 1) * stride;
    fSamples.push_back({lastCubic[3], vperp(u1), 1.0f, count - 1});

    this->refineRotation(cubics, stride);

    // segmentStart() emitted the first sample's offsets, finishRun() emits the last's.
    fLeft.resize(leftBase);
    fRight.resize(rightBase);
    for (int k = 1; k + 1 < fSamples.size(); ++k) {
        const Sample& s = fSamples[k];
        const SkVector offset = s.fNormal * fRadius;
        fLeft.push_back(s.fPt + offset);
        fRight.push_back(s.fPt - offset);
    }
}

void StrokeFlatten::refineRotation(const SkPoint* cubics, int stride) {
    const int count = fSamples.size();
    fRefined.clear();
    fRefined.push_back(fSamples[0]);
    for (int k = 0; k + 1 < count; ++k) {
        Sample a = fSamples[k];
        const Sample b = fSamples[k + 1];
        if (vdot(a.fNormal, b.fNormal) < fMinDot) {
            // Split at the seams between the two samples' cubics, so that each part lies within
            // one cubic, then subdivide the parts that still rotate too much.
            for (int c = a.fCubic; c < b.fCubic; ++c) {
                const SkPoint* pts = cubics + c * stride;
                const SkVector seamNormal = unit_normal_or(end_tangent(pts), a.fNormal);
                const Sample seam = {pts[3], seamNormal, 1.0f, c};
                this->subdivide(pts, a, seam, /*depth=*/0);
                fRefined.push_back(seam);
                const SkPoint* next = pts + stride;
                a = {next[0], unit_normal_or(start_tangent(next), seam.fNormal), 0.0f, c + 1};
            }
            this->subdivide(cubics + b.fCubic * stride, a, b, /*depth=*/0);
        }
        fRefined.push_back(b);
    }
    fSamples.swap(fRefined);
}

void StrokeFlatten::subdivide(const SkPoint pts[4], const Sample& a, const Sample& b, int depth) {
    const float cosAngle = vdot(a.fNormal, b.fNormal);
    if (cosAngle >= fMinDot || !(b.fT > a.fT) || depth >= kMaxRefineDepth ||
        fRefined.size() >= kMaxSegmentsPerCurve) {
        return;
    }
    // Split [a, b] into as many equal steps in t as its rotation needs if it turned at a uniform
    // rate, then subdivide the steps that still rotate too much. The cubic is convex, so the
    // normal turns monotonically and each step rotates less than [a, b] does.
    const float angle = std::acos(std::clamp(cosAngle, -1.0f, 1.0f));
    // Clamped before converting, which would overflow for huge radii (tiny fRadialDelta); the
    // order of max() and min() also maps NaN to 2.
    const int m = static_cast<int>(std::max(
            2.0f,
            std::min(std::ceil(angle / fRadialDelta), static_cast<float>(kMaxRefinePerInterval))));
    const CubicCoeffs cubic(pts);
    const float dt = (b.fT - a.fT) / static_cast<float>(m);
    Sample prev = a;
    for (int j = 1; j < m; ++j) {
        const float t = a.fT + dt * static_cast<float>(j);
        const Sample s = {cubic.eval(t), unit_normal_or(cubic.deriv(t), prev.fNormal), t, a.fCubic};
        this->subdivide(pts, prev, s, depth + 1);
        fRefined.push_back(s);
        prev = s;
    }
    this->subdivide(pts, prev, b, depth + 1);
}

void StrokeFlatten::segmentStart(SkVector unitTangent, bool isLine, float length) {
    if (fSegmentCount == 0) {
        const SkVector n = vperp(unitTangent) * fRadius;
        fFirstU = unitTangent;
        fFirstIsLine = isLine;
        fFirstLength = length;
        fLeft.push_back(fPrevPt + n);
        fRight.push_back(fPrevPt - n);
    } else {
        this->join(fPrevU,
                   unitTangent,
                   fPrevIsLine,
                   isLine,
                   fPrevLength,
                   length,
                   /*closing=*/false);
    }
    ++fSegmentCount;
}

void StrokeFlatten::join(SkVector u0,
                         SkVector u1,
                         bool prevIsLine,
                         bool currIsLine,
                         float prevLength,
                         float currLength,
                         bool closing) {
    const float d = vdot(u0, u1);
    // SkStroke's kNearlyLine: no join at all (checked here too, to skip the division).
    if (d >= 0 && 1 - d <= SK_ScalarNearlyZero) {
        return;
    }
    // A reversal (1 + d = 0) has no miter, and joinTerms() doesn't use 1 / (1 + d) then.
    const float onePlusD = 1.0f + d;
    this->joinTerms(u0,
                    u1,
                    d,
                    vcross(u0, u1),
                    onePlusD > 0 ? 1.0f / onePlusD : 0.0f,
                    prevIsLine,
                    currIsLine,
                    prevLength,
                    currLength,
                    closing);
}

void StrokeFlatten::joinTerms(SkVector u0,
                              SkVector u1,
                              float d,
                              float c,
                              float invOnePlusD,
                              bool prevIsLine,
                              bool currIsLine,
                              float prevLength,
                              float currLength,
                              bool closing) {
    // SkStroke's kNearlyLine: no join at all, the chains just continue. The next segment's start
    // offsets are skipped as well, like SkStroke does.
    if (d >= 0 && 1 - d <= SK_ScalarNearlyZero) {
        return;
    }
    const SkPoint pivot = fPrevPt;
    const SkVector n0 = vperp(u0);
    const SkVector n1 = vperp(u1);
    const bool nearly180 = d < 0 && 1 + d <= SK_ScalarNearlyZero;
    // The outer side is the one turned away from. A reversal has no preferred side: pick the left
    // side, whose arc then bulges forward along u0 (rotating n0 by -90 degrees yields u0).
    const bool leftIsOuter = c < 0 || (c == 0 && d < 0);
    Buffer<SkPoint>& outer = leftIsOuter ? fLeft : fRight;
    Buffer<SkPoint>& inner = leftIsOuter ? fRight : fLeft;
    const float so = leftIsOuter ? fRadius : -fRadius;
    // (n0 + n1) / (1 + d) has length 1 / cos(theta / 2), so scaled by the radius it reaches the
    // intersection of the two offset lines.
    const SkVector miter = (n0 + n1) * (nearly180 ? 0.0f : invOnePlusD);

    // Outer side. A bevel needs no extra points, the chain edge between the offsets is the bevel.
    if (fActiveJoin == SkPaint::kMiter_Join && !nearly180 && 1 + d >= fMiterThreshold) {
        const SkPoint tip = pivot + miter * so;
        // As in SkStroke, the tip replaces the end offset of a preceding line and the start
        // offset of a following line, since those are collinear with it.
        if (prevIsLine) {
            outer.back() = tip;
        } else {
            outer.push_back(tip);
        }
        if (closing) {
            if (currIsLine) {
                outer[0] = tip;
            }
        } else if (!currIsLine) {
            outer.push_back(pivot + n1 * so);
        }
    } else {
        if (fActiveJoin == SkPaint::kRound_Join) {
            this->appendArc(&outer, pivot, n0 * so, n1 * so, leftIsOuter ? -1.0f : 1.0f);
        }
        if (!closing) {
            outer.push_back(pivot + n1 * so);
        }
    }

    // Inner side. Between two lines that are long enough, the inner offsets intersect at a
    // distance r * tan(theta / 2) from the pivot along each line. Using that point avoids the
    // pivot detour and fills the same area. Otherwise go through the pivot like SkStroke's
    // HandleInnerJoin, which keeps the nonzero winding correct even for short segments.
    bool innerMiter = false;
    if (prevIsLine && currIsLine && !nearly180) {
        const float extent = fRadius * std::abs(c) * invOnePlusD;
        innerMiter = 2 * extent <= std::min(prevLength, currLength);
    }
    if (innerMiter) {
        const SkPoint x = pivot - miter * so;
        inner.back() = x;
        if (closing) {
            inner[0] = x;
        }
    } else {
        inner.push_back(pivot);
        if (!closing) {
            inner.push_back(pivot - n1 * so);
        }
    }
}

void StrokeFlatten::appendArc(
        Buffer<SkPoint>* chain, SkPoint center, SkVector from, SkVector to, float dir) const {
    // Fixed angular steps of fRadialDelta, so joins need no trigonometry. Only the last segment of
    // the arc is shorter.
    const float c = fCosDelta;
    const float s = dir * fSinDelta;
    SkVector v = from;
    for (int i = 0; i < fMaxArcSteps; ++i) {
        v = {v.fX * c - v.fY * s, v.fX * s + v.fY * c};
        // Stop once 'v' reaches or passes 'to' (joins never turn more than 180 degrees).
        if (dir * vcross(v, to) <= 0) {
            break;
        }
        chain->push_back(center + v);
    }
}

void StrokeFlatten::appendCap(Buffer<SkPoint>* chain,
                              SkPoint pivot,
                              SkVector unitTangent,
                              bool atEnd) const {
    // At the end of a contour, go from pivot + r n (left) to pivot - r n (right) around the front.
    // At the start, go from the right offset back to the left offset around the back.
    const float sign = atEnd ? fRadius : -fRadius;
    const SkVector n = vperp(unitTangent) * sign;
    const SkVector f = unitTangent * sign;
    if (fCap == SkPaint::kSquare_Cap) {
        chain->push_back(pivot + n + f);
        chain->push_back(pivot - n + f);
    } else if (fCap == SkPaint::kRound_Cap) {
        for (int k = 1; k < fCapSegments; ++k) {
            chain->push_back(pivot + n * fCapCos[k] + f * fCapSin[k]);
        }
    }
}

void StrokeFlatten::appendDot(SkPoint center, bool round) {
    const int count = round ? 2 * fCapSegments : 4;
    PointWriter writer(this->reserveOutput(count + 3), fOutLast);
    writer.sentinel();
    SkPoint first;
    if (round) {
        first = center + SkVector{fCapCos[0], fCapSin[0]} * fRadius;
        for (int k = 0; k < fCapSegments; ++k) {
            writer.write(center + SkVector{fCapCos[k], fCapSin[k]} * fRadius);
        }
        for (int k = 0; k < fCapSegments; ++k) {
            writer.write(center - SkVector{fCapCos[k], fCapSin[k]} * fRadius);
        }
    } else {
        // SkStroke orients zero-length square caps upright, i.e. an axis-aligned square.
        first = center + SkVector{-fRadius, -fRadius};
        writer.write(first);
        writer.write(center + SkVector{fRadius, -fRadius});
        writer.write(center + SkVector{fRadius, fRadius});
        writer.write(center + SkVector{-fRadius, fRadius});
    }
    writer.write(first);
    writer.sentinel();
    fOut = writer.dst();
    fOutLast = writer.last();
}

}  // namespace skgpu::graphite
