/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkContourMeasure.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/tessellate/ArcLengthCalculator.h"
#include "src/gpu/tessellate/LinearTolerances.h"
#include "src/gpu/tessellate/PatchWriter.h"
#include "src/gpu/tessellate/Tessellation.h"
#include "src/gpu/tessellate/WangsFormula.h"
#include "tests/Test.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <optional>
#include <vector>

using namespace skgpu::tess;

namespace {

// The kArcLength attribute of a patch (see PatchWriter's ArcLengthData).
struct ArcLengthRecord {
    float fStart;
    float fJoinLength;
    float fCurveLength;
    float fTotalLength;
    SkPoint fNextPoint;
    float fStartRadius;
    float fEndRadius;
    SkPoint fPartnerCorner;
    float fPartnerAngle;
    float fPartnerRadius;
};

// A patch written by ArcLengthPatchWriter.
struct Patch {
    SkPoint fPts[4];
    SkPoint fJoin;
    StrokeParams fStroke;
    ArcLengthRecord fArc;

    bool isConic() const { return std::isinf(fPts[3].fY); }
    SkPoint last() const { return this->isConic() ? fPts[2] : fPts[3]; }
    // Caps and cusps are written as circles (cubics whose control points are all the same) and
    // squares (conics whose control points are all the same).
    bool isPoint() const {
        return fPts[0] == fPts[1] && fPts[1] == fPts[2] && (this->isConic() || fPts[2] == fPts[3]);
    }

    // The control points that define the tangents at the start and the end of the curve, picked
    // the way PatchWriter and the vertex shader pick them.
    SkPoint startTangentPoint() const {
        return fPts[1] != fPts[0] ? fPts[1] : (fPts[2] != fPts[0] ? fPts[2] : this->last());
    }
    SkPoint endTangentPoint() const {
        SkPoint end = this->last();
        return fPts[2] != end ? fPts[2] : (fPts[1] != end ? fPts[1] : fPts[0]);
    }
};

// A PatchAllocator that keeps a copy of every patch.
class CollectingPatchAllocator {
public:
    CollectingPatchAllocator(size_t stride, std::vector<std::vector<char>>* patches)
            : fStride(stride), fPatches(patches) {}

    skgpu::VertexWriter append(const LinearTolerances&) {
        fPatches->emplace_back(fStride);
        return {fPatches->back().data(), fStride};
    }

private:
    size_t fStride;
    std::vector<std::vector<char>>* fPatches;
};

// Matches the traits ArcLengthStrokeRenderStep uses that affect arc lengths.
using ArcLengthPatchWriter = PatchWriter<CollectingPatchAllocator,
                                         Required<PatchAttribs::kJoinControlPoint>,
                                         Required<PatchAttribs::kStrokeParams>,
                                         Required<PatchAttribs::kArcLength>,
                                         ReplicateLineEndPoints,
                                         TrackJoinControlPoints>;
constexpr PatchAttribs kAttribs =
        PatchAttribs::kJoinControlPoint | PatchAttribs::kStrokeParams | PatchAttribs::kArcLength;
static_assert(PatchStride(kAttribs) == sizeof(Patch));

// The same, without arc lengths.
using PlainPatchWriter = PatchWriter<CollectingPatchAllocator,
                                     Required<PatchAttribs::kJoinControlPoint>,
                                     Required<PatchAttribs::kStrokeParams>,
                                     ReplicateLineEndPoints,
                                     TrackJoinControlPoints>;
constexpr PatchAttribs kPlainAttribs =
        PatchAttribs::kJoinControlPoint | PatchAttribs::kStrokeParams;

constexpr float kRadius = 10.f;
StrokeParams round_join(float radius = kRadius) { return {radius, -1.f}; }
StrokeParams bevel_join() { return {kRadius, 0.f}; }
StrokeParams miter_join() { return {kRadius, 4.f}; }
StrokeParams hairline() { return {0.f, -1.f}; }

// Writes 'path' the way ArcLengthStrokeRenderStep does, with a uniform 'scale' as the transform,
// and returns the raw patches. Also returns the writer's accumulated length, if it has one.
template <typename Writer>
std::vector<std::vector<char>> collect_patches(const SkPath& path,
                                               PatchAttribs attribs,
                                               StrokeParams stroke,
                                               SkPaint::Cap cap,
                                               float scale,
                                               float totalPathLength = 0.f,
                                               float* accumulatedLength = nullptr) {
    std::vector<std::vector<char>> patches;
    Writer writer(attribs, &patches);
    writer.setShaderTransform(skgpu::wangs_formula::VectorXform(SkMatrix::Scale(scale, scale)),
                              scale);
    writer.updateStrokeParamsAttrib(stroke);
    if constexpr (std::is_same_v<Writer, ArcLengthPatchWriter>) {
        writer.setTotalPathLength(totalPathLength);
    }
    WriteStrokedPathVerbs(writer, path, cap, ArcLengthAccumulator::JoinRadius(stroke));
    if (accumulatedLength) {
        *accumulatedLength = writer.accumulatedLength();
    }
    return patches;
}

struct WriteResult {
    float fTotalLength;
    std::vector<Patch> fPatches;
};

WriteResult write_patches(const SkPath& path,
                          StrokeParams stroke,
                          SkPaint::Cap cap,
                          float scale,
                          float totalPathLength = 0.f) {
    WriteResult result;
    std::vector<std::vector<char>> patches = collect_patches<ArcLengthPatchWriter>(
            path, kAttribs, stroke, cap, scale, totalPathLength, &result.fTotalLength);
    for (const std::vector<char>& data : patches) {
        Patch patch;
        memcpy(&patch, data.data(), sizeof(Patch));
        result.fPatches.push_back(patch);
    }
    return result;
}

float precalculate(const SkPath& path, StrokeParams stroke, SkPaint::Cap cap) {
    ArcLengthPrecalculator precalculator(stroke);
    WriteStrokedPathVerbs(precalculator, path, cap, ArcLengthAccumulator::JoinRadius(stroke));
    return precalculator.totalLength();
}

ArcLengthAccumulator::float2 to_float2(SkPoint p) { return {p.fX, p.fY}; }

// Whether a patch lies inside a fold, which its arc length data describes instead of the patch's
// own (see PatchWriter's ArcLengthData).
bool inside_fold(const Patch& patch) { return patch.fArc.fStartRadius < 0.f; }

bool same_fold(const Patch& a, const Patch& b) {
    return memcmp(&a.fArc, &b.fArc, sizeof(ArcLengthRecord)) == 0;
}

// How far a patch's start join extends into it. The vertex shader works this out from the join's
// radius and tangents too.
float start_trim(const Patch& patch) {
    if (inside_fold(patch)) {
        // The fold's sweep starts where the piece of its curve before it ends.
        return 0.f;
    }
    std::optional<ArcLengthAccumulator::JoinShape> shape =
            ArcLengthAccumulator::Shape(to_float2(patch.fJoin),
                                        to_float2(patch.fPts[0]),
                                        to_float2(patch.startTangentPoint()));
    return shape ? patch.fArc.fStartRadius * shape->fTanHalfAngle : 0.f;
}

// How much arc length the partner of the fold that a patch describes sweeps over, or 0 if it has
// none (see PatchWriter's ArcLengthData). It turns from the fold's fB (the border's normal) to its
// own fB.
float partner_sweep(const Patch& patch) {
    const ArcLengthRecord& arc = patch.fArc;
    if (!inside_fold(patch) || !(arc.fPartnerRadius > 0.f)) {
        return 0.f;
    }
    const float borderAngle = arc.fEndRadius + arc.fCurveLength;
    const SkVector normal = {std::cos(borderAngle), std::sin(borderAngle)};
    const SkVector b = {std::cos(arc.fPartnerAngle), std::sin(arc.fPartnerAngle)};
    return std::atan2(std::abs(normal.cross(b)), normal.dot(b)) * arc.fPartnerRadius;
}

// Where the arc length along a patch's curve would reach the end of the curve, if no join followed.
// For a patch inside a fold, that's where the fold's sweep ends (or its partner's, if it has one).
float linear_end(const Patch& patch) {
    if (inside_fold(patch)) {
        return (patch.fArc.fStart + patch.fArc.fJoinLength) + partner_sweep(patch);
    }
    return (patch.fArc.fStart + patch.fArc.fJoinLength) +
           (patch.fArc.fCurveLength - start_trim(patch));
}

// Checks that the joins, folds, and curves of the written patches cover [0, total] in order,
// without gaps or overlaps: each join starts its trim before the end of the curve before it, but
// not before the join at the other end of that curve ends, and each fold starts where the curve
// before it ends. Also checks that every cap and cusp outside a fold lies within [0, total].
void check_continuity(skiatest::Reporter* reporter, const WriteResult& result) {
    // Errors scale with the lengths of the curves and trims, which can add up to much more than
    // the total (e.g. when a join nearly reverses).
    float pathLength = 0.f;
    for (const Patch& patch : result.fPatches) {
        if (!inside_fold(patch)) {
            pathLength += patch.fArc.fCurveLength;
        }
    }
    const float tolerance = 1e-5f * std::max({1.f, result.fTotalLength, pathLength});
    std::vector<const Patch*> spans;
    for (const Patch& patch : result.fPatches) {
        REPORTER_ASSERT(reporter, patch.fArc.fStart >= 0.f);
        if (inside_fold(patch)) {
            // All the patches inside a fold (pieces of its curve, and a cusp) describe the same
            // fold, whose sweep covers [fStart, fStart + fJoinLength]. Check each fold once.
            REPORTER_ASSERT(reporter, patch.fArc.fJoinLength > 0.f);
            REPORTER_ASSERT(reporter, std::abs(patch.fArc.fCurveLength) < SK_ScalarPI);
            // The fragment shader's sweep starts at (fStart + fJoinLength) - angle * radius, so
            // that has to be fStart.
            float sweep = std::abs(patch.fArc.fCurveLength) * -patch.fArc.fStartRadius;
            REPORTER_ASSERT(reporter, std::abs(patch.fArc.fJoinLength - sweep) <= tolerance,
                            "fold sweep %f vs angle * radius %f", patch.fArc.fJoinLength, sweep);
            if (std::none_of(spans.begin(), spans.end(), [&](const Patch* span) {
                    return same_fold(*span, patch);
                })) {
                spans.push_back(&patch);
            }
            continue;
        }
        if (patch.isPoint()) {
            REPORTER_ASSERT(reporter, patch.fArc.fJoinLength == 0.f);
            REPORTER_ASSERT(reporter, patch.fArc.fCurveLength == 0.f);
            REPORTER_ASSERT(reporter, patch.fArc.fStart <= result.fTotalLength + tolerance);
            continue;
        }
        spans.push_back(&patch);
    }
    // Order the spans by where their joins' sweeps end, which never decreases along the path.
    // (Where the span before each one ends can decrease after a bordered line whose start join
    // sweeps less than its trim. Starts alone tie where a join takes up all of the span before
    // it.) A fold's sweep ends where the piece of its curve after it starts, and so does a join
    // that takes up all of its span, so ties go to the span that starts first.
    std::sort(spans.begin(), spans.end(), [](const Patch* a, const Patch* b) {
        float aJoinEnd = a->fArc.fStart + a->fArc.fJoinLength;
        float bJoinEnd = b->fArc.fStart + b->fArc.fJoinLength;
        return aJoinEnd != bJoinEnd ? aJoinEnd < bJoinEnd : a->fArc.fStart < b->fArc.fStart;
    });
    float prevEnd = 0.f;
    float prevJoinEnd = 0.f;
    for (const Patch* span : spans) {
        float expectedStart = prevEnd - start_trim(*span);
        REPORTER_ASSERT(reporter, std::abs(span->fArc.fStart - expectedStart) <= tolerance,
                        "span starts at %f, expected %f", span->fArc.fStart, expectedStart);
        REPORTER_ASSERT(reporter, span->fArc.fStart >= prevJoinEnd - tolerance,
                        "join starts at %f, before the previous one ends at %f",
                        span->fArc.fStart, prevJoinEnd);
        prevEnd = linear_end(*span);
        prevJoinEnd = span->fArc.fStart + span->fArc.fJoinLength;
    }
    REPORTER_ASSERT(reporter, std::abs(prevEnd - result.fTotalLength) <= tolerance,
                    "spans end at %f, expected %f", prevEnd, result.fTotalLength);
}

// Whether a patch is a line (see PatchWriter's ReplicateLineEndPoints).
bool is_line(const Patch& patch) {
    return !patch.isConic() && patch.fPts[0] == patch.fPts[1] && patch.fPts[2] == patch.fPts[3] &&
           patch.fPts[0] != patch.fPts[3];
}

// Whether a patch is a bordered line, whose joins sweep up to the line through their corners (see
// PatchWriter's ArcLengthData).
bool bordered(const Patch& patch) { return !inside_fold(patch) && patch.fArc.fEndRadius < 0.f; }

SkVector unit(SkVector v) {
    v.normalize();
    return v;
}

float angle_between(SkVector a, SkVector b) { return std::atan2(std::abs(a.cross(b)), a.dot(b)); }

// The inner corner of the join at 'p' between unit tangents 'a' and 'b', with corner radius
// 'radius', found like the vertex shader's $arclength_corner() finds it.
SkPoint join_corner(SkPoint p, SkVector a, SkVector b, float radius) {
    float sin = a.cross(b);
    float cos = a.dot(b);
    float tanHalfAngle = cos >= 0 ? std::abs(sin) / (1 + cos) : (1 - cos) / std::abs(sin);
    SkVector innerNormal = SkVector{-a.fY, a.fX} * (sin < 0 ? -1.f : 1.f);
    return p + innerNormal * radius - a * (radius * tanHalfAngle);
}

// The normal of a bordered line's border, pointing along the line, found like the vertex shader
// finds it: from the corners of the joins at either end.
SkVector border_normal(const Patch& patch) {
    SkPoint p0 = patch.fPts[0];
    SkPoint p1 = patch.last();
    SkVector d = unit(p1 - p0);
    SkPoint startCorner = join_corner(p0, unit(p0 - patch.fJoin), d, patch.fArc.fStartRadius);
    SkPoint endCorner =
            join_corner(p1, d, unit(patch.fArc.fNextPoint - p1), -patch.fArc.fEndRadius);
    SkVector border = endCorner - startCorner;
    SkVector n = unit({-border.fY, border.fX});
    return n.dot(d) < 0 ? -n : n;
}

// Checks that every patch with a start join was told about it as the end join of the patch before
// it: the same corner radius, and the direction it leaves in. Also checks that the join sweeps the
// angle between the directions it arrives from and leaves in, times its radius, where next to a
// bordered line, that direction is the border's normal. Patches inside folds have no joins.
void check_end_joins(skiatest::Reporter* reporter, const WriteResult& result) {
    int startJoins = 0;
    int endJoins = 0;
    for (const Patch& patch : result.fPatches) {
        if (inside_fold(patch)) {
            continue;
        }
        // Outside folds, the end radius is a corner radius, or 0 where nothing joins to the end
        // of the patch (like before a fold). Only bordered lines negate it, and they always have a
        // start join.
        if (bordered(patch)) {
            REPORTER_ASSERT(reporter, is_line(patch) && patch.fArc.fStartRadius > 0.f,
                            "bordered patch at (%f, %f)", patch.fPts[0].fX, patch.fPts[0].fY);
        }
        if (patch.fArc.fEndRadius != 0.f) {
            ++endJoins;
        }
        if (!(patch.fArc.fStartRadius > 0.f)) {
            continue;
        }
        ++startJoins;
        // The patch before it ends where it starts, in the direction its join arrives from.
        const Patch* prev = nullptr;
        for (const Patch& other : result.fPatches) {
            if (!other.isPoint() && !inside_fold(other) && other.last() == patch.fPts[0] &&
                other.endTangentPoint() == patch.fJoin) {
                REPORTER_ASSERT(reporter, !prev, "more than one patch joins to (%f, %f)",
                                patch.fPts[0].fX, patch.fPts[0].fY);
                prev = &other;
            }
        }
        REPORTER_ASSERT(reporter, prev, "nothing joins to (%f, %f)",
                        patch.fPts[0].fX, patch.fPts[0].fY);
        if (prev) {
            REPORTER_ASSERT(reporter, std::abs(prev->fArc.fEndRadius) == patch.fArc.fStartRadius,
                            "end radius %f vs start radius %f",
                            prev->fArc.fEndRadius, patch.fArc.fStartRadius);
            REPORTER_ASSERT(reporter, prev->fArc.fNextPoint == patch.startTangentPoint());
            SkVector arriving = bordered(*prev) ? border_normal(*prev)
                                                : unit(patch.fPts[0] - patch.fJoin);
            SkVector leaving = bordered(patch) ? border_normal(patch)
                                               : unit(patch.startTangentPoint() - patch.fPts[0]);
            float sweep = angle_between(arriving, leaving) * patch.fArc.fStartRadius;
            REPORTER_ASSERT(reporter, std::abs(patch.fArc.fJoinLength - sweep) <= 1e-3f,
                            "join at (%f, %f) sweeps %f, expected %f", patch.fPts[0].fX,
                            patch.fPts[0].fY, patch.fArc.fJoinLength, sweep);
        }
    }
    REPORTER_ASSERT(reporter, startJoins == endJoins, "%d start joins vs %d end joins",
                    startJoins, endJoins);
}

// The arc length of a polyline with joins, computed independently of ArcLengthAccumulator (and in
// double precision), assuming every join gets its full trim (see ArcLengthAccumulator::Share()).
double polyline_length(const std::vector<SkPoint>& pts, double radius) {
    double total = 0;
    for (size_t i = 1; i < pts.size(); ++i) {
        total += SkPoint::Distance(pts[i - 1], pts[i]);
        if (i + 1 < pts.size()) {
            SkVector a = pts[i] - pts[i - 1];
            SkVector b = pts[i + 1] - pts[i];
            double angle = std::atan2(std::abs((double)a.cross(b)), (double)a.dot(b));
            total += angle * radius - 2 * radius * std::tan(angle / 2);
        }
    }
    return total;
}

SkPath polyline(const std::vector<SkPoint>& pts) {
    SkPathBuilder builder;
    builder.moveTo(pts[0]);
    for (size_t i = 1; i < pts.size(); ++i) {
        builder.lineTo(pts[i]);
    }
    return builder.detach();
}

}  // namespace

DEF_TEST(ArcLength_ConicLengthWithoutBulge, reporter) {
    // Conics that bulge so little that ConicLength() doesn't split them used to recurse forever.
    float flatQuad = ArcLengthCalculator::ConicLength({0, 0}, {50, 0}, {100, 0}, 1.f);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(flatQuad, 100.f, 1e-3f), "%f", flatQuad);

    float flatConic = ArcLengthCalculator::ConicLength({0, 0}, {50, 0}, {100, 0}, 2.f);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(flatConic, 100.f, 1e-3f), "%f", flatConic);

    // A tiny quarter circle, like the corner of a small rounded rect.
    constexpr float kTinyRadius = 0.05f;
    float tinyArc = ArcLengthCalculator::ConicLength(
            {kTinyRadius, 0}, {kTinyRadius, kTinyRadius}, {0, kTinyRadius}, SK_ScalarRoot2Over2);
    float expected = SK_ScalarPI / 2 * kTinyRadius;
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(tinyArc, expected, expected * 1e-3f),
                    "%f vs %f", tinyArc, expected);
}

DEF_TEST(ArcLength_CurveLengthsMatchContourMeasure, reporter) {
    // Hairline joins have no corners, so the total is just the length of the path.
    const SkPath paths[] = {
        SkPathBuilder().moveTo(10, 10).lineTo(200, 40).quadTo(300, 300, 50, 250).detach(),
        SkPathBuilder().moveTo(0, 0).cubicTo(100, 200, 300, -100, 400, 100).detach(),
        SkPathBuilder().moveTo(0, 0).conicTo(200, 0, 200, 200, 0.6f)
                                    .conicTo(200, 400, 0, 400, 2.f).detach(),
        SkPathBuilder().moveTo(0, 0).lineTo(100, 0).lineTo(100, 100).close()
                       .moveTo(300, 300).cubicTo(400, 300, 300, 400, 400, 400).detach(),
    };
    for (const SkPath& path : paths) {
        float expected = 0.f;
        SkContourMeasureIter iter(path, /*forceClosed=*/false, /*resScale=*/64.f);
        while (sk_sp<SkContourMeasure> contour = iter.next()) {
            expected += contour->length();
        }
        float actual = precalculate(path, hairline(), SkPaint::kButt_Cap);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(actual, expected, expected * 1e-3f),
                        "%f vs %f", actual, expected);
    }
}

DEF_TEST(ArcLength_JoinLengths, reporter) {
    // A join sweeps a quarter circle of the stroke radius around its inner corner, and the strokes
    // on either side end the stroke radius before the corner (tan(45 degrees) = 1).
    constexpr float kQuarterArc = SK_ScalarPI / 2 * kRadius;
    SkPath corner = SkPathBuilder().moveTo(0, 0).lineTo(100, 0).lineTo(100, 100).detach();
    float expected = 200.f + kQuarterArc - 2.f * kRadius;
    for (SkPaint::Cap cap : {SkPaint::kButt_Cap, SkPaint::kRound_Cap, SkPaint::kSquare_Cap}) {
        float precalculated = precalculate(corner, round_join(), cap);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(precalculated, expected, 1e-3f),
                        "%f vs %f", precalculated, expected);
    }

    // In a closed square, the joins at the second, third, and fourth corners sweep around their
    // corners, but the closing join (back at the first corner) doesn't.
    SkPath square = SkPathBuilder().moveTo(0, 0).lineTo(100, 0).lineTo(100, 100).lineTo(0, 100)
                                   .close().detach();
    expected = 400.f + 3.f * (kQuarterArc - 2.f * kRadius);
    float roundTotal = precalculate(square, round_join(), SkPaint::kButt_Cap);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(roundTotal, expected, 1e-3f),
                    "%f vs %f", roundTotal, expected);
    float written = write_patches(square, round_join(), SkPaint::kButt_Cap, 1.f).fTotalLength;
    REPORTER_ASSERT(reporter, written == roundTotal, "%f vs %f", written, roundTotal);

    // Bevel and miter joins are parameterized the same way, but hairlines have no corners.
    for (StrokeParams stroke : {bevel_join(), miter_join()}) {
        float total = precalculate(square, stroke, SkPaint::kButt_Cap);
        REPORTER_ASSERT(reporter, total == roundTotal, "%f vs %f", total, roundTotal);
    }
    float hairlineTotal = precalculate(square, hairline(), SkPaint::kButt_Cap);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(hairlineTotal, 400.f, 1e-3f),
                    "%f", hairlineTotal);

    // Joins between smooth pieces of a curve (chopped where they inflect) change ~nothing.
    SkPath sCurve = SkPathBuilder().moveTo(0, 0).cubicTo(100, 200, 300, -100, 400, 100).detach();
    roundTotal = precalculate(sCurve, round_join(), SkPaint::kButt_Cap);
    hairlineTotal = precalculate(sCurve, hairline(), SkPaint::kButt_Cap);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(roundTotal, hairlineTotal, 1e-3f),
                    "%f vs %f", roundTotal, hairlineTotal);

    // The zigzag in the arclengthjoins GM, whose ~140 degree turns still fit their trims.
    const std::vector<SkPoint> zigzag = {{25, 150}, {65, 45}, {105, 155}, {145, 45}, {180, 150}};
    double zigzagLength = polyline_length(zigzag, 15);
    float precalculated = precalculate(polyline(zigzag), round_join(15), SkPaint::kRound_Cap);
    REPORTER_ASSERT(reporter, std::abs(precalculated - zigzagLength) < 1e-3,
                    "%f vs %f", precalculated, zigzagLength);
}

DEF_TEST(ArcLength_JoinCornerRadius, reporter) {
    using float2 = ArcLengthAccumulator::float2;
    using JoinShape = ArcLengthAccumulator::JoinShape;
    constexpr float kHalfPi = SK_ScalarPI / 2;

    // Every join type uses the stroke radius, so only hairlines have no corners.
    REPORTER_ASSERT(reporter, ArcLengthAccumulator::JoinRadius(round_join()) == kRadius);
    REPORTER_ASSERT(reporter, ArcLengthAccumulator::JoinRadius(bevel_join()) == kRadius);
    REPORTER_ASSERT(reporter, ArcLengthAccumulator::JoinRadius(miter_join()) == kRadius);
    REPORTER_ASSERT(reporter, ArcLengthAccumulator::JoinRadius(hairline()) == 0.f);

    // A right angle: a corner of radius R has a trim of R (tan(45 degrees) = 1).
    std::optional<JoinShape> shape = ArcLengthAccumulator::Shape({0, 0}, {100, 0}, {100, 100});
    REPORTER_ASSERT(reporter, shape && SkScalarNearlyEqual(shape->fAngle, kHalfPi));
    REPORTER_ASSERT(reporter, shape && SkScalarNearlyEqual(shape->fTanHalfAngle, 1.f));

    // 60 degrees: tan(30 degrees).
    shape = ArcLengthAccumulator::Shape({0, 0}, {100, 0}, float2{100, 0} + float2{50, 86.6025f});
    REPORTER_ASSERT(reporter, shape && SkScalarNearlyEqual(shape->fAngle, SK_ScalarPI / 3, 1e-5f));
    REPORTER_ASSERT(reporter, shape && SkScalarNearlyEqual(shape->fTanHalfAngle,
                                                           std::tan(SK_ScalarPI / 6), 1e-5f));

    // Parallel tangents don't turn. Opposite or undefined ones can't have a corner.
    shape = ArcLengthAccumulator::Shape({0, 0}, {100, 0}, {200, 0});
    REPORTER_ASSERT(reporter, shape && shape->fAngle == 0.f && shape->fTanHalfAngle == 0.f);
    REPORTER_ASSERT(reporter, !ArcLengthAccumulator::Shape({0, 0}, {100, 0}, {0, 0}));
    REPORTER_ASSERT(reporter, !ArcLengthAccumulator::Shape({100, 0}, {100, 0}, {200, 0}));

    // The joins at either end of a verb both get their full trims if those fit, and otherwise
    // split the verb in proportion to them.
    REPORTER_ASSERT(reporter, ArcLengthAccumulator::Share(30, 60, 100) == 30);
    REPORTER_ASSERT(reporter, ArcLengthAccumulator::Share(30, 70, 100) == 30);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(ArcLengthAccumulator::Share(30, 90, 100), 25.f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(ArcLengthAccumulator::Share(90, 30, 100), 75.f));
    REPORTER_ASSERT(reporter, ArcLengthAccumulator::Share(200, 0, 100) == 100);
    REPORTER_ASSERT(reporter, ArcLengthAccumulator::Share(0, 200, 100) == 0);

    // A right angle after an 8-long verb that nothing else joins to: the join's full trim (10)
    // doesn't fit, so it takes the whole verb, with a corner radius of 8.
    float expected = 108.f + kHalfPi * 8.f - 2.f * 8.f;
    float precalculated =
            precalculate(polyline({{0, 0}, {8, 0}, {8, 100}}), round_join(), SkPaint::kButt_Cap);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(precalculated, expected, 1e-3f),
                    "%f vs %f", precalculated, expected);

    // Two right angles at either end of a 10-long verb split it, with corner radii of 5.
    expected = 210.f + 2.f * (kHalfPi * 5.f - 2.f * 5.f);
    precalculated = precalculate(polyline({{0, 0}, {100, 0}, {100, 10}, {0, 10}}), round_join(),
                                 SkPaint::kButt_Cap);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(precalculated, expected, 1e-3f),
                    "%f vs %f", precalculated, expected);

    // The left path of the arclengthcorners GM: the ~132 degree join's full trim (~35.6) fits on
    // the ~60-long verb after it, next to the ~0.2 trim of the join at that verb's other end.
    const std::vector<SkPoint> corners = {
            {20, 200}, {100, 150}, {170, 150}, {170, 70}, {215, 110}, {300, 190}};
    double cornersLength = polyline_length(corners, 16);
    precalculated = precalculate(polyline(corners), round_join(16), SkPaint::kButt_Cap);
    REPORTER_ASSERT(reporter, std::abs(precalculated - cornersLength) < 1e-3,
                    "%f vs %f", precalculated, cornersLength);

    // Nearly reversing (by 0.01 radians short of 180 degrees): tan(angle / 2) is ~200, so the full
    // trim would be ~2000. The join takes the whole first verb instead, with a corner radius of
    // ~100 / 200.
    WriteResult reversal = write_patches(polyline({{0, 0}, {100, 0}, {0, 1}}), round_join(),
                                         SkPaint::kButt_Cap, 1.f);
    const Patch* second = nullptr;
    for (const Patch& patch : reversal.fPatches) {
        if (patch.fPts[0] == SkPoint{100, 0}) {
            second = &patch;
        }
    }
    REPORTER_ASSERT(reporter, second);
    if (second) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(second->fArc.fStartRadius, 0.5f, 1e-4f),
                        "%f", second->fArc.fStartRadius);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(start_trim(*second), 100.f, 1e-3f),
                        "%f", start_trim(*second));
    }
    check_continuity(reporter, reversal);

    // Exactly reversing: no corner, so nothing changes the length.
    precalculated =
            precalculate(polyline({{0, 0}, {100, 0}, {0, 0}}), round_join(), SkPaint::kButt_Cap);
    REPORTER_ASSERT(reporter, precalculated == 200.f, "%f", precalculated);
}

DEF_TEST(ArcLength_ZigzagBorders, reporter) {
    // Shape() tells which way a join turns, which decides whether two joins can share a border.
    std::optional<ArcLengthAccumulator::JoinShape> left =
            ArcLengthAccumulator::Shape({0, 0}, {100, 0}, {100, 100});
    std::optional<ArcLengthAccumulator::JoinShape> right =
            ArcLengthAccumulator::Shape({0, 0}, {100, 0}, {100, -100});
    REPORTER_ASSERT(reporter, left && left->fTurnsPositive);
    REPORTER_ASSERT(reporter, right && !right->fTurnsPositive);

    // The zigzags in the arclengthzigzag GM, whose opposite joins' full trims overlap on the lines
    // between them, so they share borders (see ArcLengthAccumulator::finish()). The expectations
    // come from a double-precision reference implementation of the borders, which also checks
    // them against the corners the vertex shader rebuilds.
    struct Case {
        const char* fName;
        std::vector<SkPoint> fPts;
        float fRadius;
        int fBorders;
        float fTotal;
    };
    const Case kCases[] = {
        // The middle join shares a border with each of the joins next to it.
        {"wide M", {{40, 170}, {75, 50}, {120, 175}, {165, 50}, {200, 170}}, 28, 2, 230.7042f},
        {"flatter M", {{40, 130}, {80, 75}, {120, 135}, {160, 75}, {195, 130}}, 28, 2, 191.9462f},
        // Only the first two joins' trims overlap. The third join gets the room that the second
        // one's trim leaves on the line between them.
        {"near-reversals", {{40, 50}, {200, 62}, {40, 90}, {200, 118}, {40, 146}}, 13, 1,
         254.0644f},
        {"unequal turns", {{40, 190}, {100, 40}, {120, 160}, {210, 110}}, 30, 1, 223.7658f},
        // The near-reversal's full trim doesn't fit on the line by itself, so its corner shrinks.
        {"near-reversal and a mild turn", {{40, 40}, {200, 55}, {140, 63}, {94, 174}}, 10, 1,
         239.4955f},
        // A border fixes the second join's trim (10, its full trim), so the sharp turn after it,
        // whose full trim (70) overlaps it on the 20-long line between them, gets the room that's
        // left (10) instead of a share in proportion to their full trims (17.5).
        {"zigzag, then a sharp turn the same way",
         {{0, 0}, {100, 0}, {85, 20}, {101, 32}, {41, -48}}, 10, 1, 207.0339f},
        // The second join's full trim (20) is longer than the line after it (12), which limits
        // it, since the join at that line's other end isn't known yet when the border is found.
        {"zigzag with a short last line", {{0, 0}, {100, 0}, {85, 20}, {97, 20}}, 10, 1,
         108.8302f},
    };
    for (const Case& c : kCases) {
        SkPath path = polyline(c.fPts);
        float total = precalculate(path, round_join(c.fRadius), SkPaint::kButt_Cap);
        REPORTER_ASSERT(reporter, std::abs(total - c.fTotal) <= 0.01f, "%s: total %f vs %f",
                        c.fName, total, c.fTotal);
        WriteResult result = write_patches(path, round_join(c.fRadius), SkPaint::kButt_Cap, 1.f);
        REPORTER_ASSERT(reporter, result.fTotalLength == total);
        check_continuity(reporter, result);
        // This also checks that every join sweeps between the borders on either side of it.
        check_end_joins(reporter, result);
        auto borders = std::count_if(result.fPatches.begin(), result.fPatches.end(), bordered);
        REPORTER_ASSERT(reporter, borders == c.fBorders, "%s: %d borders", c.fName, (int)borders);
    }

    // Zigzags whose middle line shrinks, past where its joins' full trims start to overlap, and
    // where they stop fitting on it by themselves: the total changes by no more than the line's
    // length does. Also with unequal turns, and with the first join limited by a short first line,
    // so its trim fits next to the other one's for longer.
    auto zigzag = [](float first, float turn1, float length, float turn2) {
        SkPoint p1 = {first, 0};
        SkPoint p2 = p1 + SkVector{std::cos(turn1), std::sin(turn1)} * length;
        SkPoint p3 = p2 + SkVector{std::cos(turn1 - turn2), std::sin(turn1 - turn2)} * 100;
        return polyline({{0, 0}, p1, p2, p3});
    };
    struct Sweep {
        float fFirst, fTurn1, fTurn2;  // The turns are in degrees.
    };
    for (Sweep s : {Sweep{100, 140, 140}, Sweep{100, 150, 110}, Sweep{100, 110, 150},
                    Sweep{15, 140, 140}}) {
        const float turn1 = SkDegreesToRadians(s.fTurn1);
        const float turn2 = SkDegreesToRadians(s.fTurn2);
        float prevTotal = 0.f;
        bool sawBorder = false, sawNoBorder = false;
        for (int i = 0; i <= 650; ++i) {
            float length = 70 - 0.1f * i;
            SkPath path = zigzag(s.fFirst, turn1, length, turn2);
            float total = precalculate(path, round_join(), SkPaint::kButt_Cap);
            if (i > 0) {
                REPORTER_ASSERT(reporter, std::abs(total - prevTotal) <= 0.105f,
                                "total %f at length %f, %f before (%g, %g, %g)", total, length,
                                prevTotal, s.fFirst, s.fTurn1, s.fTurn2);
            }
            prevTotal = total;
            WriteResult result = write_patches(path, round_join(), SkPaint::kButt_Cap, 1.f);
            check_continuity(reporter, result);
            check_end_joins(reporter, result);
            bool hasBorder = std::any_of(result.fPatches.begin(), result.fPatches.end(), bordered);
            sawBorder |= hasBorder;
            sawNoBorder |= !hasBorder;
        }
        REPORTER_ASSERT(reporter, sawBorder && sawNoBorder, "(%g, %g, %g)", s.fFirst, s.fTurn1,
                        s.fTurn2);
    }
}

DEF_TEST(ArcLength_Cusps, reporter) {
    // A flat cusp adds no length, and neither does the join after it, so joins add nothing.
    const SkPath flatCusps[] = {
        // A flat quad with a cusp
        SkPathBuilder().moveTo(1, 1).quadTo(100, 1, 25, 1).detach(),
        // A flat cubic with two cusps
        SkPathBuilder().moveTo(10, 0).cubicTo(0, 0, 20, 0, 10, 0).detach(),
    };
    for (const SkPath& path : flatCusps) {
        float roundTotal = precalculate(path, round_join(), SkPaint::kRound_Cap);
        float hairlineTotal = precalculate(path, hairline(), SkPaint::kRound_Cap);
        REPORTER_ASSERT(reporter, roundTotal == hairlineTotal,
                        "%f vs %f", roundTotal, hairlineTotal);
    }

    // The cusp of a curve that isn't flat lies inside a fold (see ArcLengthFold), which shortens
    // the total, and parameterizes the cusp's circle too.
    SkPath cusp = SkPathBuilder().moveTo(0, 0).cubicTo(128, 128, 128, 0, 0, 128).detach();
    float roundTotal = precalculate(cusp, round_join(), SkPaint::kRound_Cap);
    float hairlineTotal = precalculate(cusp, hairline(), SkPaint::kRound_Cap);
    REPORTER_ASSERT(reporter, roundTotal < hairlineTotal - 1,
                    "%f vs %f", roundTotal, hairlineTotal);
    WriteResult result = write_patches(cusp, round_join(), SkPaint::kRound_Cap, 1.f);
    int insideCircles = 0;
    for (const Patch& patch : result.fPatches) {
        if (patch.isPoint() && inside_fold(patch)) {
            ++insideCircles;
            REPORTER_ASSERT(reporter, patch.fPts[0].distanceToOrigin() > 90);  // Not a cap
        }
    }
    REPORTER_ASSERT(reporter, insideCircles == 1, "%d", insideCircles);
    check_continuity(reporter, result);
    check_end_joins(reporter, result);
}

DEF_TEST(ArcLength_Folds, reporter) {
    // The expectations come from a double-precision reference implementation of
    // FindArcLengthFolds(), which follows the same steps.
    struct Case {
        const char* fName;
        SkPathVerb fVerb;
        SkPoint fPts[4];
        float fW;
        float fRadius;
        std::optional<float> fCuspT;
        // The expected fold, and the total of a single-verb path with it.
        float fT1, fT2;
        SkPoint fCorner;
        float fAngleDegrees;
        float fFoldRadius;
        float fTotal;
        float fTotalTolerance = 0.05f;
    };
    const Case kCases[] = {
        // The swallowtail in the arclengthcusp GM, which turns 129 degrees within 6.6px.
        {"swallowtail", SkPathVerb::kCubic, {{50, 160}, {230, 40}, {70, 40}, {230, 160}}, 1,
         12.5f, std::nullopt, 0.283690f, 0.722909f, {146.0542f, 94.4985f}, 110.3141f, 12.5f,
         240.9142f},
        // Sharp apexes.
        {"quad spike", SkPathVerb::kQuad, {{60, 170}, {120, -300}, {180, 170}}, 1, 20,
         std::nullopt, 0.346040f, 0.653960f, {120, -35.0591f}, 134.9634f, 20, 481.4284f},
        {"conic spike", SkPathVerb::kConic, {{0, 100}, {60, -60}, {120, 100}}, 2, 15,
         std::nullopt, 0.384155f, 0.615845f, {60, 8.8801f}, 77.9667f, 15, 251.0228f},
        // A cusp, whose fold is the limit of near-cusps'.
        {"cusp", SkPathVerb::kCubic, {{50, 200}, {170, 320}, {170, 200}, {50, 320}}, 1, 12.5f,
         0.5f, 0.217007f, 0.782993f, {105.0124f, 260}, 120.9815f, 12.5f, 183.7474f},
        // An apex whose fold doesn't fit within the quad at the stroke radius, so it shrinks
        // until it covers all of it.
        {"short spike", SkPathVerb::kQuad, {{0, 0}, {10, -40}, {20, 0}}, 1, 20, std::nullopt,
         0, 1, {10, 2.4996f}, 151.9272f, 10.3077f, 27.3321f},
        // Just tighter than the stroke radius: the apex's curvature radius is 25.
        {"barely", SkPathVerb::kQuad, {{0, 0}, {50, 100}, {100, 0}}, 1, 26, std::nullopt,
         0.428586f, 0.571414f, {50, 23.98f}, 31.8847f, 26, 147.8883f},
        // A loop whose hole shows folds at its tip (F7). (The quadrature that measures its long,
        // strongly turning pieces isn't as exact as the reference's.)
        {"open loop", SkPathVerb::kCubic, {{50, 160}, {230, 40}, {-250, 40}, {230, 160}}, 1,
         12.5f, std::nullopt, 0.473779f, 0.755891f, {32.7206f, 82.7107f}, 154.7419f, 12.5f,
         393.5337f, 1.f},
    };
    for (const Case& c : kCases) {
        ArcLengthFold folds[kMaxArcLengthFolds];
        int numFolds = FindArcLengthFolds(c.fVerb, c.fPts, c.fW, c.fRadius, c.fCuspT, folds);
        REPORTER_ASSERT(reporter, numFolds == 1, "%s: %d folds", c.fName, numFolds);
        if (numFolds != 1) {
            continue;
        }
        const ArcLengthFold& fold = folds[0];
        // A shrunk radius comes from a bisection, which is only so precise.
        const float tolerance = c.fFoldRadius < c.fRadius ? 1e-3f : 1e-4f;
        REPORTER_ASSERT(reporter, std::abs(fold.fT1 - c.fT1) <= tolerance,
                        "%s: T1 %f vs %f", c.fName, fold.fT1, c.fT1);
        REPORTER_ASSERT(reporter, std::abs(fold.fT2 - c.fT2) <= tolerance,
                        "%s: T2 %f vs %f", c.fName, fold.fT2, c.fT2);
        REPORTER_ASSERT(reporter, SkPoint::Distance(fold.fCorner, c.fCorner) <= 1e-2f,
                        "%s: C (%f, %f) vs (%f, %f)", c.fName, fold.fCorner.fX, fold.fCorner.fY,
                        c.fCorner.fX, c.fCorner.fY);
        REPORTER_ASSERT(reporter,
                        std::abs(SkRadiansToDegrees(fold.angle()) - c.fAngleDegrees) <= 1e-2f,
                        "%s: angle %f vs %f", c.fName, SkRadiansToDegrees(fold.angle()),
                        c.fAngleDegrees);
        REPORTER_ASSERT(reporter, std::abs(fold.fRadius - c.fFoldRadius) <= tolerance * 10,
                        "%s: radius %f vs %f", c.fName, fold.fRadius, c.fFoldRadius);
        // The corner's cross-sections through C are the curve's normals at T1 and T2.
        auto eval = [&](float t) {
            switch (c.fVerb) {
                case SkPathVerb::kQuad: return SkEvalQuadAt(c.fPts, t);
                case SkPathVerb::kConic: return SkConic(c.fPts, c.fW).evalAt(t);
                default: {
                    SkPoint p;
                    SkEvalCubicAt(c.fPts, t, &p, nullptr, nullptr);
                    return p;
                }
            }
        };
        for (auto [t, tangent] : {std::make_pair(fold.fT1, fold.fA),
                                  std::make_pair(fold.fT2, fold.fB)}) {
            SkVector toCorner = fold.fCorner - eval(t);
            REPORTER_ASSERT(reporter, std::abs(toCorner.length() - fold.fRadius) <= 1e-3f,
                            "%s: %f from the curve, vs a radius of %f", c.fName,
                            toCorner.length(), fold.fRadius);
            REPORTER_ASSERT(reporter, std::abs(toCorner.dot(tangent)) <= 1e-3f,
                            "%s: %f along the tangent", c.fName, toCorner.dot(tangent));
        }

        SkPathBuilder builder;
        builder.moveTo(c.fPts[0]);
        switch (c.fVerb) {
            case SkPathVerb::kQuad: builder.quadTo(c.fPts[1], c.fPts[2]); break;
            case SkPathVerb::kConic: builder.conicTo(c.fPts[1], c.fPts[2], c.fW); break;
            default: builder.cubicTo(c.fPts[1], c.fPts[2], c.fPts[3]); break;
        }
        SkPath path = builder.detach();
        float total = precalculate(path, round_join(c.fRadius), SkPaint::kButt_Cap);
        REPORTER_ASSERT(reporter, std::abs(total - c.fTotal) <= c.fTotalTolerance,
                        "%s: total %f vs %f", c.fName, total, c.fTotal);
        WriteResult result = write_patches(path, round_join(c.fRadius), SkPaint::kButt_Cap, 1.f);
        REPORTER_ASSERT(reporter, result.fTotalLength == total);
        check_continuity(reporter, result);
        check_end_joins(reporter, result);
        // Every patch inside the fold describes it (see PatchWriter's ArcLengthData), which is
        // what the vertex shader rebuilds the fold's corner from.
        const float turn = std::atan2(fold.fA.cross(fold.fB), fold.fA.dot(fold.fB));
        const float directionA = std::atan2(fold.fA.fY, fold.fA.fX);
        int insidePatches = 0;
        for (const Patch& patch : result.fPatches) {
            if (!inside_fold(patch)) {
                continue;
            }
            ++insidePatches;
            const ArcLengthRecord& arc = patch.fArc;
            REPORTER_ASSERT(reporter, SkPoint::Distance(arc.fNextPoint, fold.fCorner) <= 1e-3f,
                            "%s: encoded C (%f, %f)", c.fName, arc.fNextPoint.fX,
                            arc.fNextPoint.fY);
            REPORTER_ASSERT(reporter, std::abs(arc.fStartRadius + fold.fRadius) <= 1e-3f,
                            "%s: encoded radius %f", c.fName, arc.fStartRadius);
            REPORTER_ASSERT(reporter, std::abs(arc.fJoinLength - fold.length()) <= 1e-3f,
                            "%s: encoded length %f", c.fName, arc.fJoinLength);
            REPORTER_ASSERT(reporter, std::abs(arc.fCurveLength - turn) <= 1e-4f,
                            "%s: encoded turn %f vs %f", c.fName, arc.fCurveLength, turn);
            REPORTER_ASSERT(reporter, std::abs(arc.fEndRadius - directionA) <= 1e-4f,
                            "%s: encoded direction %f vs %f", c.fName, arc.fEndRadius,
                            directionA);
            REPORTER_ASSERT(reporter, arc.fPartnerRadius == 0.f, "%s: partner", c.fName);
        }
        REPORTER_ASSERT(reporter, insidePatches > 0, "%s: no patches inside the fold", c.fName);
    }

    // Bends that a ball with the stroke radius can follow don't fold, and neither do circular
    // arcs (whose offsets never cross themselves), or hairlines.
    struct NoFold {
        const char* fName;
        SkPathVerb fVerb;
        SkPoint fPts[4];
        float fW;
        float fRadius;
    };
    const NoFold kNoFolds[] = {
        {"S-curve", SkPathVerb::kCubic, {{0, 0}, {100, 200}, {300, -100}, {400, 100}}, 1, 10},
        {"gentle", SkPathVerb::kQuad, {{0, 0}, {50, 100}, {100, 0}}, 1, 24},
        {"quarter circle", SkPathVerb::kConic, {{100, 0}, {100, 100}, {0, 100}},
         SK_ScalarRoot2Over2, 150},
        {"hairline", SkPathVerb::kQuad, {{60, 170}, {120, -300}, {180, 170}}, 1, 0},
    };
    for (const NoFold& c : kNoFolds) {
        ArcLengthFold folds[kMaxArcLengthFolds];
        int numFolds =
                FindArcLengthFolds(c.fVerb, c.fPts, c.fW, c.fRadius, std::nullopt, folds);
        REPORTER_ASSERT(reporter, numFolds == 0, "%s: %d folds", c.fName, numFolds);
    }

    // The swallowtail family in the arclengthfolds GM: P2.x = 50 is a cusp, smaller values make
    // loops, and the fold vanishes at ~205. The total changes continuously (about 0.1-0.5 per unit)
    // from loops that the stroke covers, which fold like cusps (F7), through the cusp, to where the
    // fold vanishes.
    auto swallowtail = [](float x) {
        return SkPathBuilder().moveTo(50, 160).cubicTo(230, 40, x, 40, 230, 160).detach();
    };
    float prevTotal = 0.f;
    for (float x = -150; x <= 220; x += 1) {
        float total = precalculate(swallowtail(x), round_join(12.5f), SkPaint::kButt_Cap);
        if (x > -150) {
            REPORTER_ASSERT(reporter, std::abs(total - prevTotal) < 0.6f,
                            "total %f at P2.x = %f, %f before", total, x, prevTotal);
        }
        prevTotal = total;
    }
    // Where the loop's hole starts to show (between P2.x = -160 and -165), the total jumps (~205 to
    // ~334) as the loop stops folding like a cusp and folds at its tip instead (see F7).

    // The near-cusps in arclengthtricky (a tiny loop, and a serpentine) fold like the cusp.
    const SkPoint kCusps[][4] = {
        {{0, 0}, {128, 128}, {128, 0}, {0, 128}},
        {{0, .01f}, {128, 127.999f}, {128, .01f}, {0, 127.99f}},
        {{0, -.01f}, {128, 128.001f}, {128, -.01f}, {0, 128.001f}},
    };
    float cuspAngle = 0.f;
    for (const auto& pts : kCusps) {
        float T[2];
        bool areCusps;
        int numChops = FindCubicConvex180Chops(pts, T, &areCusps);
        std::optional<float> cuspT;
        if (numChops == 1 && areCusps) {
            cuspT = T[0];
        }
        ArcLengthFold folds[kMaxArcLengthFolds];
        int numFolds = FindArcLengthFolds(SkPathVerb::kCubic, pts, 1, 15, cuspT, folds);
        REPORTER_ASSERT(reporter, numFolds == 1, "%d folds", numFolds);
        if (numFolds == 1) {
            if (cuspAngle == 0.f) {
                cuspAngle = folds[0].angle();
            }
            REPORTER_ASSERT(reporter, std::abs(folds[0].angle() - cuspAngle) < 1e-3f,
                            "%f vs %f", folds[0].angle(), cuspAngle);
        }
    }
}

DEF_TEST(ArcLength_FoldPairs, reporter) {
    // The S-shaped step in the arclengthfolds GM: its two bends' folds overlap and turn opposite
    // ways, so they share a border, the line through their corners, which the curve crosses
    // halfway (by symmetry). The expectations come from a double-precision reference of the
    // pairing, built on the reference implementation of FindArcLengthFolds().
    const SkPoint kStep[] = {{0, 0}, {100, 0}, {0, 20}, {100, 20}};
    constexpr SkPoint kCorner1 = {38.3107f, 14.1274f};
    constexpr SkPoint kCorner2 = {61.6893f, 5.8726f};
    constexpr float kSweepDegrees = 58.7947f;
    ArcLengthFold folds[kMaxArcLengthFolds];
    int numFolds = FindArcLengthFolds(SkPathVerb::kCubic, kStep, 1, 12, std::nullopt, folds);
    REPORTER_ASSERT(reporter, numFolds == 2, "%d folds", numFolds);
    if (numFolds == 2) {
        const ArcLengthFold& first = folds[0];
        const ArcLengthFold& second = folds[1];
        REPORTER_ASSERT(reporter, std::abs(first.fT1 - 0.215160f) <= 1e-4f, "%f", first.fT1);
        REPORTER_ASSERT(reporter, std::abs(first.fT2 - 0.5f) <= 1e-4f, "%f", first.fT2);
        REPORTER_ASSERT(reporter, second.fT1 == first.fT2);
        REPORTER_ASSERT(reporter, std::abs(second.fT2 - 0.784840f) <= 1e-4f, "%f", second.fT2);
        REPORTER_ASSERT(reporter, SkPoint::Distance(first.fCorner, kCorner1) <= 1e-2f);
        REPORTER_ASSERT(reporter, SkPoint::Distance(second.fCorner, kCorner2) <= 1e-2f);
        // The border's normal, perpendicular to the line through the corners and pointing along
        // the curve, replaces the first fold's fB and the second one's fA.
        REPORTER_ASSERT(reporter, first.fB == second.fA);
        REPORTER_ASSERT(reporter, SkPoint::Distance(first.fB, {0.332951f, 0.942944f}) <= 1e-4f);
        REPORTER_ASSERT(reporter,
                        std::abs(first.fB.dot(second.fCorner - first.fCorner)) <= 1e-3f);
        for (const ArcLengthFold& fold : folds) {
            REPORTER_ASSERT(reporter, fold.fRadius == 12);
            REPORTER_ASSERT(reporter,
                            std::abs(SkRadiansToDegrees(fold.angle()) - kSweepDegrees) <= 1e-2f,
                            "sweep %f", SkRadiansToDegrees(fold.angle()));
        }
        // The first fold describes the second one, its partner.
        REPORTER_ASSERT(reporter, !first.fIsPartner && second.fIsPartner);
        REPORTER_ASSERT(reporter, first.fPartnerCorner == second.fCorner &&
                                  first.fPartnerB == second.fB &&
                                  first.fPartnerRadius == second.fRadius);
        REPORTER_ASSERT(reporter, second.fPartnerRadius == 0);
    }

    SkPath step = SkPathBuilder().moveTo(kStep[0]).cubicTo(kStep[1], kStep[2], kStep[3]).detach();
    float total = precalculate(step, round_join(12), SkPaint::kButt_Cap);
    REPORTER_ASSERT(reporter, std::abs(total - 106.3882f) <= 0.05f, "total %f", total);
    WriteResult result = write_patches(step, round_join(12), SkPaint::kButt_Cap, 1.f);
    REPORTER_ASSERT(reporter, result.fTotalLength == total);
    check_continuity(reporter, result);
    check_end_joins(reporter, result);
    // Every patch inside either fold describes both: the first one as its fold, whose fB is the
    // border's normal, and the second one as its partner, whose sweep starts where the first one's
    // ends (see PatchWriter's ArcLengthData).
    int insidePatches = 0;
    for (const Patch& patch : result.fPatches) {
        if (!inside_fold(patch)) {
            continue;
        }
        ++insidePatches;
        const ArcLengthRecord& arc = patch.fArc;
        REPORTER_ASSERT(reporter, SkPoint::Distance(arc.fNextPoint, kCorner1) <= 1e-2f);
        REPORTER_ASSERT(reporter, SkPoint::Distance(arc.fPartnerCorner, kCorner2) <= 1e-2f);
        REPORTER_ASSERT(reporter, arc.fStartRadius == -12 && arc.fPartnerRadius == 12);
        float sweep = SkRadiansToDegrees(std::abs(arc.fCurveLength));
        float partnerSweep = SkRadiansToDegrees(partner_sweep(patch) / 12);
        REPORTER_ASSERT(reporter, std::abs(sweep - kSweepDegrees) <= 1e-2f, "sweep %f", sweep);
        REPORTER_ASSERT(reporter, std::abs(partnerSweep - kSweepDegrees) <= 1e-2f,
                        "partner's sweep %f", partnerSweep);
    }
    REPORTER_ASSERT(reporter, insidePatches >= 2, "%d patches inside the folds", insidePatches);

    // Steps of other heights: up to ~31, their folds overlap, and share a border. The total grows
    // steadily with the height, including where the folds stop overlapping, and where the second
    // bend's tightest point leaves the first one's fold (which used to skip it).
    bool sawPair = false, sawSeparate = false;
    float prevTotal = 0.f;
    for (int i = 0; i <= 136; ++i) {
        float height = 6 + 0.25f * i;
        SkPath path = SkPathBuilder().moveTo(0, 0).cubicTo(100, 0, 0, height, 100, height)
                                     .detach();
        float familyTotal = precalculate(path, round_join(12), SkPaint::kButt_Cap);
        if (i > 0) {
            REPORTER_ASSERT(reporter, familyTotal > prevTotal && familyTotal - prevTotal <= 0.3f,
                            "total %f at height %f, %f before", familyTotal, height, prevTotal);
        }
        prevTotal = familyTotal;
        WriteResult familyResult = write_patches(path, round_join(12), SkPaint::kButt_Cap, 1.f);
        check_continuity(reporter, familyResult);
        check_end_joins(reporter, familyResult);
        std::vector<SkPoint> corners;
        bool paired = false;
        for (const Patch& patch : familyResult.fPatches) {
            if (inside_fold(patch)) {
                paired |= patch.fArc.fPartnerRadius > 0.f;
                if (std::find(corners.begin(), corners.end(), patch.fArc.fNextPoint) ==
                    corners.end()) {
                    corners.push_back(patch.fArc.fNextPoint);
                }
            }
        }
        sawPair |= paired;
        sawSeparate |= !paired && corners.size() == 2;
    }
    REPORTER_ASSERT(reporter, sawPair && sawSeparate);
}

DEF_TEST(ArcLength_SpanningFolds, reporter) {
    // Bends made of several verbs that are tighter than the stroke fold too: one fold spans them,
    // from a line, over one or two verbs, to a line (see F2b in WriteStrokedPathVerbs()). The
    // expectations come from a double-precision reference of SpanningFoldWriter, FindSpanShape(),
    // and FitSpan(), which measures circular conics exactly.
    constexpr float w = SK_ScalarRoot2Over2;
    struct Fold {
        SkPoint fCorner;
        float fRadius;
        float fAngleDegrees;
    };
    struct Case {
        const char* fName;
        SkPath fPath;
        float fRadius;
        float fTotal;  // Or -1 to skip
        std::vector<Fold> fFolds;
    };
    const Case kCases[] = {
        // The rounded corner in the arclengthfolds GM, whose radius (8) is less than the stroke's.
        // The fold's corner is the one a join between the lines would get.
        {"rounded corner",
         SkPathBuilder().moveTo(48, 40).lineTo(182, 40).conicTo(190, 40, 190, 48, w)
                        .lineTo(190, 162).detach(),
         18, 256.2743f, {{{172, 58}, 18, 90}}},
        // The whole rounded rect, except the corner where it starts and ends (see F9).
        {"rounded rect",
         SkPathBuilder().moveTo(48, 40).lineTo(182, 40).conicTo(190, 40, 190, 48, w)
                        .lineTo(190, 162).conicTo(190, 170, 182, 170, w)
                        .lineTo(48, 170).conicTo(40, 170, 40, 162, w)
                        .lineTo(40, 48).conicTo(40, 40, 48, 40, w).close().detach(),
         18, 533.3894f, {{{172, 58}, 18, 90}, {{172, 152}, 18, 90}, {{58, 152}, 18, 90}}},
        // Short lines between two sharp joins that turn the same way, whose full trims overlap on
        // them.
        {"short line", polyline({{30, 170}, {110, 30}, {120, 30}, {200, 170}}), 15, 321.6955f,
         {{{115, 51.4835f}, 15, 120.5102f}}},
        {"unequal turns", polyline({{20, 170}, {100, 40}, {108, 40}, {200, 120}}), 15, 277.4358f,
         {{{106.2934f, 58.3939f}, 15, 99.4016f}}},
        // A gentle arc between two sharp joins.
        {"gentle arc",
         SkPathBuilder().moveTo(52, 172).lineTo(100, 40)
                        .conicTo(105.21f, 39.08f, 110.42f, 40, 0.9848f).lineTo(158, 172).detach(),
         15, 265.2192f, {{{105.1709f, 69.6726f}, 15, 140.195f}}},
        // Near-reversals, whose folds shrink to fit their lines: both of them, or the shorter one.
        {"whole path", polyline({{-5.2094f, 29.5442f}, {0, 0}, {10, 0}, {15.2094f, 29.5442f}}),
         15, 28.9499f, {{{5, 31.3444f}, 10.3669f, 160}}},
        {"short first line",
         polyline({{-5.17638f, 19.31852f}, {0, 0}, {6, 0}, {26.70552f, 77.27408f}}), 15,
         82.1608f, {{{3, 21.5094f}, 8.4648f, 150}}},
        // Two rounded corners whose folds don't both fit on the short side between them at full
        // size, so they split it where their corners meet (see arclength_folds::SideRadius()).
        {"short side",
         SkPathBuilder().moveTo(20, 40).lineTo(160, 40).conicTo(164, 40, 164, 44, w)
                        .lineTo(164, 50).conicTo(164, 54, 160, 54, w).lineTo(20, 54).detach(),
         12, 295.9911f, {{{157, 47}, 7, 90}, {{157, 47}, 7, 90}}},
        // Three joins on two short lines, which one fold spans (one middle verb doesn't fit).
        {"three joins",
         polyline({{-80, 0}, {0, 0}, {7.6604f, 6.4279f}, {8.7023f, 12.3367f},
                   {-31.2977f, 81.6187f}}),
         15, 169.5245f, {{{-10.1559f, 15}, 15, 120}}},
        // A rounded corner, then a short line and a sharp join that turns the same way: one fold
        // spans the conic, the line, and the join (along the path either way).
        {"rounded corner, then a join",
         SkPathBuilder().moveTo(0, 0).lineTo(100, 0).conicTo(108, 0, 108, 8, w).lineTo(108, 14)
                        .lineTo(21.3975f, 64).detach(),
         18, 173.0188f, {{{65.0718f, 18}, 18, 150}}},
        {"a join, then a rounded corner",
         SkPathBuilder().moveTo(21.3975f, 64).lineTo(108, 14).lineTo(108, 8)
                        .conicTo(108, 0, 100, 0, w).lineTo(0, 0).detach(),
         18, 173.0188f, {{{65.0718f, 18}, 18, 150}}},
        // A duplicate point changes nothing.
        {"duplicate point", polyline({{30, 170}, {110, 30}, {110, 30}, {120, 30}, {200, 170}}),
         15, 321.6955f, {{{115, 51.4835f}, 15, 120.5102f}}},
        // A rounded corner whose fold would start a thousandth into its first line, or (the other
        // way along it) end a thousandth before the end of its last line: the fold starts at the
        // line's start (or ends at its end) instead, leaving no sliver of it (see FitSpan()).
        {"snapped start",
         SkPathBuilder().moveTo(171.999f, 40).lineTo(182, 40).conicTo(190, 40, 190, 48, w)
                        .lineTo(190, 162).detach(),
         18, 132.2743f, {{{172, 58}, 18, 90}}},
        {"snapped end",
         SkPathBuilder().moveTo(190, 162).lineTo(190, 48).conicTo(190, 40, 182, 40, w)
                        .lineTo(171.999f, 40).detach(),
         18, 132.2743f, {{{172, 58}, 18, 90}}},
        // A rounded corner, then a short line that the closing line turns sharply away from. That
        // join and the fold would have to meet beyond where the fold's corner reaches the conic.
        {"closing line",
         SkPathBuilder().moveTo(0, 0).lineTo(60, 0).conicTo(70, 0, 70, 10, w).lineTo(70, 14)
                        .close().detach(),
         15, 148.8925f, {}},
        // Bends that don't fold: the short line in the GM, whose joins' full trims fit on it, a
        // U-turn, a cubic that's nearly the gentle arc above, since folds can't span cubics, and
        // the first short line above with a slight dip the other way in it (a third of a degree),
        // since its joins don't all turn the same way (see F9).
        {"trims fit", polyline({{30, 170}, {105, 30}, {125, 30}, {200, 170}}), 15, 334.0927f, {}},
        {"U-turn", polyline({{0, 0}, {100, 0}, {100, 10}, {0, 10}}), 10, 205.708f, {}},
        {"cubic",
         SkPathBuilder().moveTo(52, 172).lineTo(100, 40)
                        .cubicTo(103.4463f, 39.3923f, 106.9737f, 39.3923f, 110.42f, 40)
                        .lineTo(158, 172).detach(),
         15, -1, {}},
        {"dip", polyline({{30, 170}, {110, 30}, {115, 30.015f}, {120, 30}, {200, 170}}), 15,
         330.6109f, {}},
    };
    // No sliver of a line is left next to a fold, whose join would sweep wherever its direction
    // happens to point (see FitSpan()). (The duplicate point's empty line is the path's own.)
    auto checkSlivers = [&](const char* name, const WriteResult& result) {
        for (const Patch& patch : result.fPatches) {
            float length = SkPoint::Distance(patch.fPts[0], patch.last());
            REPORTER_ASSERT(reporter, !is_line(patch) || length == 0 || length > 1e-2f,
                            "%s: a line from (%f, %f) to (%f, %f)", name, patch.fPts[0].fX,
                            patch.fPts[0].fY, patch.last().fX, patch.last().fY);
        }
    };
    for (const Case& c : kCases) {
        float total = precalculate(c.fPath, round_join(c.fRadius), SkPaint::kButt_Cap);
        if (c.fTotal >= 0) {
            REPORTER_ASSERT(reporter, std::abs(total - c.fTotal) <= 0.01f, "%s: total %f vs %f",
                            c.fName, total, c.fTotal);
        }
        for (float scale : {1.f, 40.f}) {
            WriteResult result =
                    write_patches(c.fPath, round_join(c.fRadius), SkPaint::kButt_Cap, scale);
            REPORTER_ASSERT(reporter, result.fTotalLength == total);
            check_continuity(reporter, result);
            check_end_joins(reporter, result);
            checkSlivers(c.fName, result);
            // The folds that the patches describe, in order.
            std::vector<const Patch*> folds;
            for (const Patch& patch : result.fPatches) {
                if (inside_fold(patch) && (folds.empty() || !same_fold(*folds.back(), patch))) {
                    folds.push_back(&patch);
                }
            }
            REPORTER_ASSERT(reporter, folds.size() == c.fFolds.size(), "%s: %zu folds", c.fName,
                            folds.size());
            for (size_t i = 0; i < std::min(folds.size(), c.fFolds.size()); ++i) {
                const ArcLengthRecord& arc = folds[i]->fArc;
                const Fold& expected = c.fFolds[i];
                REPORTER_ASSERT(reporter,
                                SkPoint::Distance(arc.fNextPoint, expected.fCorner) <= 1e-2f,
                                "%s: C (%f, %f)", c.fName, arc.fNextPoint.fX, arc.fNextPoint.fY);
                REPORTER_ASSERT(reporter, std::abs(arc.fStartRadius + expected.fRadius) <= 1e-3f,
                                "%s: radius %f", c.fName, -arc.fStartRadius);
                float angle = SkRadiansToDegrees(std::abs(arc.fCurveLength));
                REPORTER_ASSERT(reporter, std::abs(angle - expected.fAngleDegrees) <= 1e-2f,
                                "%s: angle %f", c.fName, angle);
                REPORTER_ASSERT(reporter, arc.fPartnerRadius == 0.f, "%s: partner", c.fName);
            }
        }
    }

    // A loop between two lines 90 degrees apart: its joins and curve turn 450 degrees in total, so
    // no fold spans it, unlike the bend without it (although the lines' extensions cross past the
    // end of the first one and before the start of the last one either way).
    using arclength_folds::QueuedVerb;
    const QueuedVerb loop[] = {{SkPathVerb::kLine, {{-80, 0}, {0, 0}}, 1},
                               {SkPathVerb::kQuad, {{0, 0}, {-4, 4}, {-4, -4}}, 1},
                               {SkPathVerb::kLine, {{-4, -4}, {2, 2}}, 1},
                               {SkPathVerb::kLine, {{2, 2}, {2, 82}}, 1}};
    const QueuedVerb bend[] = {loop[0], {SkPathVerb::kLine, {{0, 0}, {2, 2}}, 1}, loop[3]};
    const QueuedVerb* const loopVerbs[] = {&loop[0], &loop[1], &loop[2], &loop[3]};
    const QueuedVerb* const bendVerbs[] = {&bend[0], &bend[1], &bend[2]};
    REPORTER_ASSERT(reporter, !arclength_folds::FindSpanShape(loopVerbs, 4));
    REPORTER_ASSERT(reporter, arclength_folds::FindSpanShape(bendVerbs, 3).has_value());

    // A fold and a sharp join that don't both fit on the short line between them split it where
    // their corners meet, so the join's corner is the fold's, either way along the path. (The fold
    // can't span the join too, since they turn 190 degrees in total.)
    constexpr SkPoint kMeetEnd = {9.519225f, 4.635182f};
    SkPath meet = SkPathBuilder().moveTo(0, 0).lineTo(100, 0).conicTo(108, 0, 108, 8, w)
                                 .lineTo(108, 22).lineTo(kMeetEnd).detach();
    SkPath meetReversed = SkPathBuilder().moveTo(kMeetEnd).lineTo(108, 22).lineTo(108, 8)
                                         .conicTo(108, 0, 100, 0, w).lineTo(0, 0).detach();
    WriteResult meetResult = write_patches(meet, round_join(18), SkPaint::kButt_Cap, 1.f);
    WriteResult meetReversedResult =
            write_patches(meetReversed, round_join(18), SkPaint::kButt_Cap, 1.f);
    for (const WriteResult* result : {&meetResult, &meetReversedResult}) {
        check_continuity(reporter, *result);
        check_end_joins(reporter, *result);
    }
    // The polyline arc in the arclengthcorners GM, which is narrower than the stroke: wherever
    // folds share lines with joins or other folds, their corners meet, so like its joins without
    // folds, every corner is at the arc's center, the inradius away from its lines.
    SkPathBuilder arcBuilder;
    for (int i = 0; i <= 8; ++i) {
        float angle = SK_ScalarPI * (-0.25f + i * 1.5f / 8);
        SkPoint p = {825 + 30 * std::cos(angle), 125 + 30 * std::sin(angle)};
        if (i == 0) {
            arcBuilder.moveTo(p);
        } else {
            arcBuilder.lineTo(p);
        }
    }
    WriteResult arcResult = write_patches(arcBuilder.detach(), round_join(35), SkPaint::kButt_Cap,
                                          1.f);
    check_continuity(reporter, arcResult);
    check_end_joins(reporter, arcResult);
    struct Corners {
        WriteResult* fResult;
        SkPoint fCorner;
        float fRadius;
        int fMinFolds;
    };
    const float inradius = 30 * std::cos(SK_ScalarPI * 1.5f / 16);
    for (const Corners& c : {Corners{&meetResult, {97.9624f, 10.0376f}, 10.0376f, 1},
                             Corners{&meetReversedResult, {97.9624f, 10.0376f}, 10.0376f, 1},
                             Corners{&arcResult, {825, 125}, inradius, 0}}) {
        checkSlivers("corners", *c.fResult);
        int numFolds = 0, numJoins = 0;
        for (const Patch& patch : c.fResult->fPatches) {
            SkPoint corner;
            float radius;
            if (inside_fold(patch)) {
                ++numFolds;
                corner = patch.fArc.fNextPoint;
                radius = -patch.fArc.fStartRadius;
            } else if (patch.fArc.fStartRadius > 0) {
                ++numJoins;
                corner = join_corner(patch.fPts[0], unit(patch.fPts[0] - patch.fJoin),
                                     unit(patch.startTangentPoint() - patch.fPts[0]),
                                     patch.fArc.fStartRadius);
                radius = patch.fArc.fStartRadius;
            } else {
                continue;
            }
            REPORTER_ASSERT(reporter, SkPoint::Distance(corner, c.fCorner) <= 1e-2f &&
                                      std::abs(radius - c.fRadius) <= 1e-2f,
                            "corner (%f, %f) with radius %f", corner.fX, corner.fY, radius);
        }
        REPORTER_ASSERT(reporter, numFolds >= c.fMinFolds && numJoins > 0, "%d folds, %d joins",
                        numFolds, numJoins);
    }

    // The rounded corner's fold starts and ends where its corner projects onto the lines. Only the
    // conic and the pieces of the lines next to it describe the fold, and nothing joins to the
    // pieces of the lines before and after it.
    const SkPath& corner = kCases[0].fPath;
    WriteResult cornerResult = write_patches(corner, round_join(18), SkPaint::kButt_Cap, 1.f);
    int insidePatches = 0;
    bool sawBefore = false, sawAfter = false;
    for (const Patch& patch : cornerResult.fPatches) {
        if (inside_fold(patch)) {
            ++insidePatches;
        } else if (patch.fPts[0] == SkPoint{48, 40}) {
            sawBefore = SkPoint::Distance(patch.last(), {172, 40}) <= 1e-3f &&
                        patch.fArc.fEndRadius == 0.f;
        } else if (patch.last() == SkPoint{190, 162}) {
            sawAfter = SkPoint::Distance(patch.fPts[0], {190, 58}) <= 1e-3f &&
                       patch.fArc.fStartRadius == 0.f;
        }
    }
    REPORTER_ASSERT(reporter, insidePatches == 3 && sawBefore && sawAfter, "%d patches inside",
                    insidePatches);
    // Hairlines don't fold.
    float hairlineTotal = precalculate(corner, hairline(), SkPaint::kButt_Cap);
    float cornerLength = 134 + 114 + SK_ScalarPI / 2 * 8;
    REPORTER_ASSERT(reporter, std::abs(hairlineTotal - cornerLength) <= 1e-2f, "%f vs %f",
                    hairlineTotal, cornerLength);
    WriteResult hairlineResult = write_patches(corner, hairline(), SkPaint::kButt_Cap, 1.f);
    REPORTER_ASSERT(reporter, std::none_of(hairlineResult.fPatches.begin(),
                                           hairlineResult.fPatches.end(), inside_fold));

    // Families that cross where folds start to span verbs, stepping a parameter from 30 down to 0
    // by 0.25: the radius of a rounded corner, which ends as a sharp corner, and the length of a
    // short line between two joins, which end as one join. The total never changes by more than
    // the reference's largest step (just under the bound), which is how fast the folds' feet move,
    // so it doesn't jump where the folds start (or stop) spanning, or where they shrink.
    auto sweep = [&](const char* name, float radius, float bound, auto makePath) {
        float prevTotal = 0.f;
        bool sawFold = false, sawNoFold = false;
        for (int i = 0; i <= 120; ++i) {
            float param = 30 - 0.25f * i;
            SkPath path = makePath(param);
            float total = precalculate(path, round_join(radius), SkPaint::kButt_Cap);
            if (i > 0) {
                REPORTER_ASSERT(reporter, std::abs(total - prevTotal) <= bound,
                                "%s: total %f at %f, %f before", name, total, param, prevTotal);
            }
            prevTotal = total;
            if (i % 8 == 0 || param < 1) {
                WriteResult result =
                        write_patches(path, round_join(radius), SkPaint::kButt_Cap, 1.f);
                check_continuity(reporter, result);
                check_end_joins(reporter, result);
                bool folds =
                        std::any_of(result.fPatches.begin(), result.fPatches.end(), inside_fold);
                sawFold |= folds;
                sawNoFold |= !folds;
            }
        }
        REPORTER_ASSERT(reporter, sawFold && sawNoFold, "%s", name);
    };
    sweep("rounded corner", 18, 0.51f, [=](float rc) {
        return SkPathBuilder().moveTo(0, 0).lineTo(60, 0).conicTo(60 + rc, 0, 60 + rc, rc, w)
                              .lineTo(60 + rc, rc + 60).detach();
    });
    sweep("short line", 15, 0.54f, [](float length) {
        return polyline({{-75, 140}, {0, 0}, {length, 0}, {length + 75, 140}});
    });
    sweep("unequal turns", 15, 0.49f, [](float length) {
        return polyline({{-76.60444f, 64.27876f}, {0, 0}, {length, 0},
                         {length + 17.36482f, 98.48078f}});
    });
    // A near-reversal, whose fold shrinks to fit its short lines.
    sweep("near-reversal", 15, 1.46f, [](float length) {
        return polyline({{-5.20945f, 29.54423f}, {0, 0}, {length, 0},
                         {length + 5.20945f, 29.54423f}});
    });
}

static const SkPath* test_paths(int* count);

DEF_TEST(ArcLength_FoldsInTestPaths, reporter) {
    // Make sure the test paths with folds exercise what they're meant to (see test_paths()).
    int pathCount;
    const SkPath* paths = test_paths(&pathCount);
    auto countFolds = [](const WriteResult& result) {
        std::vector<const Patch*> folds;
        for (const Patch& patch : result.fPatches) {
            if (inside_fold(patch) && std::none_of(folds.begin(), folds.end(), [&](auto* fold) {
                    return same_fold(*fold, patch);
                })) {
                folds.push_back(&patch);
            }
        }
        return folds.size();
    };
    // The swallowtail, and the quad's and conic's apexes.
    REPORTER_ASSERT(reporter, countFolds(write_patches(paths[pathCount - 5], round_join(),
                                                       SkPaint::kButt_Cap, 1.f)) == 1);
    REPORTER_ASSERT(reporter, countFolds(write_patches(paths[pathCount - 4], round_join(),
                                                       SkPaint::kButt_Cap, 1.f)) == 2);
    // The cubic with two sharp bends, whose folds overlap and turn opposite ways, so they share a
    // border. Every patch inside either one describes both (see ArcLengthFold).
    WriteResult step = write_patches(paths[pathCount - 3], round_join(), SkPaint::kButt_Cap, 1.f);
    REPORTER_ASSERT(reporter, countFolds(step) == 1);
    REPORTER_ASSERT(reporter, std::all_of(step.fPatches.begin(), step.fPatches.end(),
                                          [](const Patch& patch) {
                                              return !inside_fold(patch) ||
                                                     patch.fArc.fPartnerRadius > 0.f;
                                          }));
    // The closed contour, whose deferred first patch (written last) is inside a fold that covers
    // all of its quad.
    WriteResult closed = write_patches(paths[pathCount - 2], round_join(), SkPaint::kButt_Cap, 1.f);
    REPORTER_ASSERT(reporter, countFolds(closed) == 1);
    REPORTER_ASSERT(reporter, !closed.fPatches.empty() && inside_fold(closed.fPatches.back()) &&
                              closed.fPatches.back().fArc.fStart == 0.f);
    // The loop that the stroke covers folds like a cusp (F7).
    REPORTER_ASSERT(reporter, countFolds(write_patches(paths[pathCount - 1], round_join(),
                                                       SkPaint::kButt_Cap, 1.f)) == 1);
    // Without arc lengths, the same chops are written (see HeldPatchesKeepTheirOrder).
}

static const SkPath* test_paths(int* count) {
    static const SkPath kPaths[] = {
        // Sharp joins
        SkPathBuilder().moveTo(0, 0).lineTo(100, 0).lineTo(50, 80).lineTo(150, 90).detach(),
        // A join whose trim doesn't fit, and one that nearly reverses
        SkPathBuilder().moveTo(0, 0).lineTo(6, 0).lineTo(6, 100).lineTo(0, 0.5f).detach(),
        // A join that nearly reverses, whose trim takes up all of the verb before it
        SkPathBuilder().moveTo(0, 0).lineTo(100, 0).lineTo(0, 1).detach(),
        // Two joins that split the short verb between them
        SkPathBuilder().moveTo(0, 0).lineTo(100, 0).lineTo(100, 8).lineTo(0, 8).detach(),
        // A join whose full trim needs most of the verb after it (arclengthcorners' left path)
        SkPathBuilder().moveTo(20, 200).lineTo(100, 150).lineTo(170, 150).lineTo(170, 70)
                       .lineTo(215, 110).lineTo(300, 190).detach(),
        // Degenerate verbs between and after other verbs. The cubic's start tangent comes from its
        // last point, and the conic after it has a different curve type, which matters for the
        // square cap at the start (see PatchWriter::writeDeferredStrokePatch()).
        SkPathBuilder().moveTo(0, 0).lineTo(50, 0).lineTo(50, 0).lineTo(50, 50).detach(),
        SkPathBuilder().moveTo(0, 0).cubicTo(0, 0, 0, 0, 50, 50).conicTo(100, 50, 100, 100, 0.5f)
                       .lineTo(100, 100).detach(),
        // A closed contour, with a curve as the deferred first patch
        SkPathBuilder().moveTo(0, 0).quadTo(100, -50, 200, 0).lineTo(150, 150).lineTo(20, 100)
                       .close().detach(),
        // Several contours, including degenerate ones that only draw caps
        SkPathBuilder().moveTo(0, 0).lineTo(0, 0)
                       .moveTo(10, 10).lineTo(100, 10).quadTo(150, 60, 100, 110).close()
                       .moveTo(200, 200).cubicTo(300, 100, 350, 300, 400, 200)
                       .moveTo(500, 500).close()
                       .detach(),
        // Single-verb contours, open and closed
        SkPathBuilder().moveTo(0, 0).lineTo(50, 50)
                       .moveTo(100, 0).cubicTo(150, 0, 150, 50, 100, 0).close()
                       .detach(),
        // Cusps
        SkPathBuilder().moveTo(0, 0).cubicTo(128, 128, 128, 0, 0, 128).lineTo(64, 200).detach(),
        SkPathBuilder().moveTo(1, 1).quadTo(100, 1, 25, 1).lineTo(25, 50).detach(),
        // Curves that get chopped to fit the tessellation limits at large scales
        SkPathBuilder().moveTo(0, 0).cubicTo(30, -300, 220, -300, 250, 0)
                       .conicTo(400, 200, 250, 400, 0.4f)
                       .quadTo(0, 600, -100, 300).detach(),
        // Folds that span verbs (see F2b in WriteStrokedPathVerbs()): a rounded rect whose corners
        // are tighter than the stroke (except where it starts and ends), two rounded corners that
        // share a short side, and a short line between two sharp joins
        SkPathBuilder().moveTo(8, 0).lineTo(42, 0).conicTo(50, 0, 50, 8, SK_ScalarRoot2Over2)
                       .lineTo(50, 22).conicTo(50, 30, 42, 30, SK_ScalarRoot2Over2)
                       .lineTo(8, 30).conicTo(0, 30, 0, 22, SK_ScalarRoot2Over2)
                       .lineTo(0, 8).conicTo(0, 0, 8, 0, SK_ScalarRoot2Over2).close().detach(),
        SkPathBuilder().moveTo(0, 0).lineTo(40, 0).conicTo(44, 0, 44, 4, SK_ScalarRoot2Over2)
                       .lineTo(44, 8).conicTo(44, 12, 40, 12, SK_ScalarRoot2Over2).lineTo(0, 12)
                       .detach(),
        SkPathBuilder().moveTo(0, 100).lineTo(40, 20).lineTo(45, 20).lineTo(85, 100).detach(),
        // Folds (bends tighter than the stroke radius), next to sharp joins: a swallowtail, a
        // quad's and a conic's sharp apexes, and a cubic with two sharp bends (whose folds
        // overlap and turn opposite ways, so they share a border)
        SkPathBuilder().moveTo(10, 50).lineTo(50, 160).cubicTo(230, 40, 70, 40, 230, 160)
                       .lineTo(260, 60).detach(),
        SkPathBuilder().moveTo(0, 100).quadTo(30, -60, 60, 100).lineTo(0, 120)
                       .conicTo(30, 0, 60, 120, 3).detach(),
        SkPathBuilder().moveTo(0, 0).cubicTo(100, 0, 0, 12, 100, 12).detach(),
        // A fold that has to shrink to cover all of its quad, first in a closed contour, so
        // the deferred patch is inside it
        SkPathBuilder().moveTo(0, 0).quadTo(5, -20, 10, 0).lineTo(10, 30).close().detach(),
        // A loop that the stroke covers, between sharp joins, which folds like a cusp (F7)
        SkPathBuilder().moveTo(10, 50).lineTo(50, 160).cubicTo(230, 40, -40, 40, 230, 160)
                       .lineTo(260, 60).detach(),
    };
    *count = std::size(kPaths);
    return kPaths;
}

DEF_TEST(ArcLength_PrecalculatorMatchesPatchWriter, reporter) {
    const StrokeParams strokes[] = {round_join(), bevel_join(), miter_join(), hairline()};
    const SkPaint::Cap caps[] = {SkPaint::kButt_Cap, SkPaint::kRound_Cap, SkPaint::kSquare_Cap};

    int pathCount;
    const SkPath* paths = test_paths(&pathCount);
    bool sawChoppedCurves = false;
    for (int i = 0; i < pathCount; ++i) {
        for (StrokeParams stroke : strokes) {
            for (SkPaint::Cap cap : caps) {
                float precalculated = precalculate(paths[i], stroke, cap);
                size_t unscaledPatchCount = 0;
                for (float scale : {1.f, 40.f}) {
                    WriteResult result =
                            write_patches(paths[i], stroke, cap, scale, precalculated);
                    REPORTER_ASSERT(reporter, result.fTotalLength == precalculated,
                                    "written %f vs precalculated %f (path %d, scale %f)",
                                    result.fTotalLength, precalculated, i, scale);
                    check_continuity(reporter, result);
                    check_end_joins(reporter, result);
                    for (const Patch& patch : result.fPatches) {
                        REPORTER_ASSERT(reporter, patch.fArc.fTotalLength == precalculated);
                    }
                    if (scale == 1.f) {
                        unscaledPatchCount = result.fPatches.size();
                    } else if (result.fPatches.size() > unscaledPatchCount) {
                        sawChoppedCurves = true;
                    }
                }
            }
        }
    }
    // Make sure the large scale actually exercised chopping.
    REPORTER_ASSERT(reporter, sawChoppedCurves);
}

DEF_TEST(ArcLength_HeldPatchesKeepTheirOrder, reporter) {
    // PatchWriter holds each patch back until it knows the patch's end join, but writes them out in
    // the same order (and with the same geometry) as without arc lengths.
    int pathCount;
    const SkPath* paths = test_paths(&pathCount);
    for (int i = 0; i < pathCount; ++i) {
        for (SkPaint::Cap cap : {SkPaint::kButt_Cap, SkPaint::kRound_Cap, SkPaint::kSquare_Cap}) {
            for (float scale : {1.f, 40.f}) {
                std::vector<std::vector<char>> withArcLength =
                        collect_patches<ArcLengthPatchWriter>(paths[i], kAttribs, round_join(), cap,
                                                              scale);
                std::vector<std::vector<char>> without = collect_patches<PlainPatchWriter>(
                        paths[i], kPlainAttribs, round_join(), cap, scale);
                REPORTER_ASSERT(reporter, withArcLength.size() == without.size(),
                                "%zu vs %zu patches (path %d)",
                                withArcLength.size(), without.size(), i);
                size_t count = std::min(withArcLength.size(), without.size());
                for (size_t p = 0; p < count; ++p) {
                    // Everything but kArcLength, which comes last.
                    REPORTER_ASSERT(reporter, memcmp(withArcLength[p].data(), without[p].data(),
                                                     PatchStride(kPlainAttribs)) == 0,
                                    "patch %zu differs (path %d)", p, i);
                }
            }
        }
    }
}
