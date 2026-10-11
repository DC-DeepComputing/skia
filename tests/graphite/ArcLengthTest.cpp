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

// Where the arc length along a patch's curve would reach the end of the curve, if no join followed.
// For a patch inside a fold, that's where the fold's sweep ends.
float linear_end(const Patch& patch) {
    if (inside_fold(patch)) {
        return patch.fArc.fStart + patch.fArc.fJoinLength;
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
    // Order the spans by where the span before each one ends: its start plus its join's trim.
    // (Sorting by start alone fails where a join takes up all of the span before it, so both
    // start at the same arc length, up to rounding.)
    std::sort(spans.begin(), spans.end(), [](const Patch* a, const Patch* b) {
        return a->fArc.fStart + start_trim(*a) < b->fArc.fStart + start_trim(*b);
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

// Checks that every patch with a start join was told about it as the end join of the patch before
// it: the same corner radius, and the direction it leaves in. Patches inside folds have no joins.
void check_end_joins(skiatest::Reporter* reporter, const WriteResult& result) {
    int startJoins = 0;
    int endJoins = 0;
    for (const Patch& patch : result.fPatches) {
        if (inside_fold(patch)) {
            continue;
        }
        // Outside folds, the end radius is a corner radius, or 0 where nothing joins to the end
        // of the patch (like before a fold), never the negative radius that marks a fold.
        REPORTER_ASSERT(reporter, patch.fArc.fEndRadius >= 0.f, "end radius %f",
                        patch.fArc.fEndRadius);
        if (patch.fArc.fEndRadius > 0.f) {
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
            REPORTER_ASSERT(reporter, prev->fArc.fEndRadius == patch.fArc.fStartRadius,
                            "end radius %f vs start radius %f",
                            prev->fArc.fEndRadius, patch.fArc.fStartRadius);
            REPORTER_ASSERT(reporter, prev->fArc.fNextPoint == patch.startTangentPoint());
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
        REPORTER_ASSERT(reporter, std::abs(total - c.fTotal) <= 0.05f,
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
    // A loop whose hole shows doesn't fold.
    float loopTotal = precalculate(swallowtail(-250), round_join(12.5f), SkPaint::kButt_Cap);
    float loopLength = precalculate(swallowtail(-250), hairline(), SkPaint::kButt_Cap);
    REPORTER_ASSERT(reporter, loopTotal == loopLength, "%f vs %f", loopTotal, loopLength);

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
    // The cubic with two sharp bends: the first one's fold reaches past the second one's
    // tightest point, so the second one doesn't fold.
    REPORTER_ASSERT(reporter, countFolds(write_patches(paths[pathCount - 3], round_join(),
                                                       SkPaint::kButt_Cap, 1.f)) == 1);
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
        // Folds (bends tighter than the stroke radius), next to sharp joins: a swallowtail, a
        // quad's and a conic's sharp apexes, and a cubic with two sharp bends (whose folds would
        // overlap, so only one of them folds)
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
