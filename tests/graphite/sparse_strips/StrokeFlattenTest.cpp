/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkStrokeRec.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/StrokeFlatten.h"
#include "tests/Test.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace skgpu::graphite {

namespace {

// StrokeFlatten's vertices lie on the exact offsets with chords within ~0.3px of them, while
// SkStroke approximates the offsets within 0.25px. Wherever the two disagree on coverage, the
// sample must therefore be close to our outline.
constexpr float kMaxMismatchDistance = 0.75f;
constexpr float kSampleStep = 0.41f;

// Winding number of 'p' with respect to all of the polyline's edges.
int winding_at(const Polyline& polyline, SkPoint p) {
    int winding = 0;
    for (auto item : polyline) {
        const SkPoint a = item.first.p0;
        const SkPoint b = item.first.p1;
        if ((a.fY <= p.fY) != (b.fY <= p.fY)) {
            const float t = (p.fY - a.fY) / (b.fY - a.fY);
            if (a.fX + t * (b.fX - a.fX) > p.fX) {
                winding += b.fY > a.fY ? 1 : -1;
            }
        }
    }
    return winding;
}

float distance_to_outline(const Polyline& polyline, SkPoint p) {
    float best = SK_ScalarInfinity;
    for (auto item : polyline) {
        const SkVector ab = item.first.p1 - item.first.p0;
        const SkVector ap = p - item.first.p0;
        const float lenSq = ab.fX * ab.fX + ab.fY * ab.fY;
        const float t =
                lenSq > 0 ? std::clamp((ap.fX * ab.fX + ap.fY * ab.fY) / lenSq, 0.0f, 1.0f) : 0.0f;
        const float dx = ap.fX - t * ab.fX;
        const float dy = ap.fY - t * ab.fY;
        best = std::min(best, std::sqrt(dx * dx + dy * dy));
    }
    return best;
}

// Every sub-path of a stroke outline must be closed for its winding to be well defined.
bool all_chains_closed(const Polyline& polyline) {
    const auto& pts = polyline.points();
    int start = -1;
    for (int i = 0; i <= pts.size(); ++i) {
        const bool chainEnd = i == pts.size() || std::isnan(pts[i].fX);
        if (chainEnd) {
            if (start >= 0 && i - start >= 2 && pts[start] != pts[i - 1]) {
                return false;
            }
            start = -1;
        } else if (start < 0) {
            start = i;
        }
    }
    return true;
}

struct StrokeCase {
    const char* fName;
    SkPath fPath;
    float fWidth;
    SkPaint::Join fJoin;
    SkPaint::Cap fCap;
};

// Compares StrokeFlatten's coverage (nonzero winding) against SkStroke's outline on a grid.
void check_case(skiatest::Reporter* reporter,
                const StrokeCase& sc,
                const SkMatrix& ctm,
                bool simd) {
    const char* mode = simd ? "simd" : "scalar";
    SkStrokeRec rec(SkStrokeRec::kFill_InitStyle);
    rec.setStrokeStyle(sc.fWidth);
    rec.setStrokeParams(sc.fCap, sc.fJoin, /*miterLimit=*/4.0f);
    rec.setResScale(ctm.getMaxScale());

    SkPathBuilder builder;
    if (!rec.applyToPath(&builder, sc.fPath)) {
        ERRORF(reporter, "[%s] SkStroke failed", sc.fName);
        return;
    }
    const SkPath reference = builder.detach(&ctm);

    const SkIRect clip = SkIRect::MakeWH(1024, 1024);
    Polyline polyline;
    StrokeFlatten stroker;
    const bool accepted =
            simd ? stroker.processStroke<FlattenMode::kSimd>(sc.fPath, ctm, rec, clip, &polyline)
                 : stroker.processStroke<FlattenMode::kScalar>(sc.fPath, ctm, rec, clip, &polyline);
    if (!accepted) {
        ERRORF(reporter, "[%s %s] StrokeFlatten rejected the stroke", sc.fName, mode);
        return;
    }
    REPORTER_ASSERT(reporter, !polyline.empty(), "[%s %s] empty outline", sc.fName, mode);
    REPORTER_ASSERT(reporter, all_chains_closed(polyline), "[%s %s] open sub-path", sc.fName, mode);

    const SkRect bounds = reference.getBounds().makeOutset(2.0f, 2.0f);
    const int cols = static_cast<int>(bounds.width() / kSampleStep);
    const int rows = static_cast<int>(bounds.height() / kSampleStep);
    int mismatches = 0;
    float worst = 0.0f;
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            const SkPoint p = {bounds.fLeft + (static_cast<float>(col) + 0.31f) * kSampleStep,
                               bounds.fTop + (static_cast<float>(row) + 0.73f) * kSampleStep};
            const bool expected = reference.contains(p.fX, p.fY);
            const bool actual = winding_at(polyline, p) != 0;
            if (expected != actual) {
                ++mismatches;
                worst = std::max(worst, distance_to_outline(polyline, p));
            }
        }
    }
    REPORTER_ASSERT(reporter,
                    worst <= kMaxMismatchDistance,
                    "[%s %s] coverage differs %.3fpx from the outline (%d of %d samples differ)",
                    sc.fName,
                    mode,
                    worst,
                    mismatches,
                    rows * cols);

    if (reporter->verbose()) {
        Polyline baseline;
        Flatten flatten;
        flatten.processPaths<FlattenMode::kScalar>(reference, SkMatrix::I(), clip, &baseline);
        SkDebugf(
                "[%s %s] points: StrokeFlatten %d vs SkStroke+Flatten %d; worst %.3fpx, %d/%d "
                "samples differ\n",
                sc.fName,
                mode,
                static_cast<int>(polyline.count()),
                static_cast<int>(baseline.count()),
                worst,
                mismatches,
                rows * cols);
    }
}

SkPath path_zigzag() {
    return SkPathBuilder()
            .moveTo(10, 40)
            .lineTo(30, 10)
            .lineTo(45, 60)
            .lineTo(60, 15)
            .lineTo(70, 50)
            .detach();
}

SkPath path_polygon() {
    return SkPathBuilder()
            .moveTo(10, 10)
            .lineTo(70, 15)
            .lineTo(60, 60)
            .lineTo(15, 50)
            .close()
            .detach();
}

SkPath path_s_curve() {
    return SkPathBuilder().moveTo(10, 60).cubicTo(10, 10, 70, 70, 70, 10).detach();
}

SkPath path_blob() {
    return SkPathBuilder()
            .moveTo(40, 10)
            .cubicTo(70, 10, 75, 40, 60, 60)
            .cubicTo(45, 75, 15, 70, 12, 45)
            .cubicTo(10, 25, 20, 10, 40, 10)
            .close()
            .detach();
}

SkPath path_quads() {
    return SkPathBuilder().moveTo(10, 50).quadTo(25, 10, 40, 40).quadTo(55, 70, 70, 20).detach();
}

SkPath path_circle() { return SkPathBuilder().addCircle({40, 40}, 25).detach(); }

SkPath path_loop() { return SkPathBuilder().moveTo(10, 60).cubicTo(80, 0, 0, 0, 70, 60).detach(); }

SkPath path_cusp() {
    return SkPathBuilder().moveTo(10, 10).cubicTo(70, 70, 10, 70, 70, 10).detach();
}

SkPath path_flat_cubic() {
    return SkPathBuilder().moveTo(10, 40).cubicTo(60, 40, 0, 40, 50, 40).detach();
}

SkPath path_mixed() {
    return SkPathBuilder()
            .moveTo(10, 10)
            .lineTo(40, 12)
            .quadTo(70, 15, 65, 45)
            .conicTo(60, 70, 30, 65, 0.7f)
            .cubicTo(10, 60, 5, 30, 10, 10)
            .close()
            .detach();
}

SkPath path_dot() { return SkPathBuilder().moveTo(40, 40).lineTo(40, 40).detach(); }

}  // namespace

DEF_TEST(SparseStrips_StrokeFlatten_MatchesSkStroke, reporter) {
    const StrokeCase cases[] = {
            {"zigzag_miter_butt", path_zigzag(), 6, SkPaint::kMiter_Join, SkPaint::kButt_Cap},
            {"zigzag_bevel_square", path_zigzag(), 6, SkPaint::kBevel_Join, SkPaint::kSquare_Cap},
            {"zigzag_round_round", path_zigzag(), 6, SkPaint::kRound_Join, SkPaint::kRound_Cap},
            {"polygon_miter", path_polygon(), 5, SkPaint::kMiter_Join, SkPaint::kButt_Cap},
            {"polygon_round", path_polygon(), 9, SkPaint::kRound_Join, SkPaint::kButt_Cap},
            {"s_curve_round", path_s_curve(), 8, SkPaint::kRound_Join, SkPaint::kRound_Cap},
            {"s_curve_thin", path_s_curve(), 1, SkPaint::kMiter_Join, SkPaint::kButt_Cap},
            {"blob_miter", path_blob(), 4, SkPaint::kMiter_Join, SkPaint::kButt_Cap},
            {"quads_bevel_square", path_quads(), 3, SkPaint::kBevel_Join, SkPaint::kSquare_Cap},
            {"circle", path_circle(), 6, SkPaint::kMiter_Join, SkPaint::kButt_Cap},
            {"loop", path_loop(), 4, SkPaint::kRound_Join, SkPaint::kButt_Cap},
            {"cusp", path_cusp(), 6, SkPaint::kMiter_Join, SkPaint::kRound_Cap},
            {"flat_cubic", path_flat_cubic(), 6, SkPaint::kMiter_Join, SkPaint::kButt_Cap},
            {"mixed", path_mixed(), 5, SkPaint::kMiter_Join, SkPaint::kSquare_Cap},
            {"dot_round", path_dot(), 8, SkPaint::kMiter_Join, SkPaint::kRound_Cap},
            {"dot_square", path_dot(), 8, SkPaint::kMiter_Join, SkPaint::kSquare_Cap},
            // Thick strokes, where the radial term dominates the sample distribution and the
            // rotation check subdivides intervals.
            {"blob_thick", path_blob(), 16, SkPaint::kMiter_Join, SkPaint::kButt_Cap},
            {"s_curve_thick", path_s_curve(), 20, SkPaint::kRound_Join, SkPaint::kButt_Cap},
            {"quads_thick", path_quads(), 14, SkPaint::kMiter_Join, SkPaint::kRound_Cap},
    };
    for (const StrokeCase& sc : cases) {
        for (bool simd : {false, true}) {
            check_case(reporter, sc, SkMatrix::I(), simd);
        }
    }
}

DEF_TEST(SparseStrips_StrokeFlatten_Transforms, reporter) {
    SkMatrix similarity = SkMatrix::Translate(100, 50);
    similarity.preRotate(30);
    similarity.preScale(1.5f, 1.5f);
    SkMatrix affine = SkMatrix::Translate(40, 300);
    affine.preSkew(0.3f, 0.0f);
    affine.preScale(2.0f, 0.75f);
    const StrokeCase cases[] = {
            {"zigzag_miter_butt", path_zigzag(), 6, SkPaint::kMiter_Join, SkPaint::kButt_Cap},
            {"blob_round", path_blob(), 4, SkPaint::kRound_Join, SkPaint::kButt_Cap},
            {"mixed_square", path_mixed(), 5, SkPaint::kMiter_Join, SkPaint::kSquare_Cap},
    };
    for (const StrokeCase& sc : cases) {
        for (bool simd : {false, true}) {
            check_case(reporter, sc, similarity, simd);
            check_case(reporter, sc, affine, simd);
        }
    }
}

// Curves whose strokes lie entirely beyond one edge of the clip are stroked as their chords. That
// must not change the winding number anywhere inside the clip.
DEF_TEST(SparseStrips_StrokeFlatten_Culling, reporter) {
    const SkPath paths[] = {path_blob(),
                            path_mixed(),
                            path_loop(),
                            path_quads(),
                            path_circle(),
                            path_s_curve(),
                            path_zigzag()};
    const SkIRect clips[] = {SkIRect::MakeLTRB(0, 0, 32, 32),
                             SkIRect::MakeLTRB(48, 48, 120, 120),
                             SkIRect::MakeLTRB(0, 40, 120, 120),
                             SkIRect::MakeLTRB(30, 0, 50, 120)};
    // Large enough to contain every stroke: nothing is culled.
    const SkIRect everything = SkIRect::MakeWH(1024, 1024);
    SkMatrix affine = SkMatrix::Scale(1.5f, 0.75f);
    affine.postSkew(0.3f, 0.0f);
    int fullPoints = 0;
    int culledPoints = 0;
    for (const SkPath& path : paths) {
        for (float width : {2.0f, 10.0f}) {
            SkStrokeRec rec(SkStrokeRec::kFill_InitStyle);
            rec.setStrokeStyle(width);
            rec.setStrokeParams(SkPaint::kSquare_Cap, SkPaint::kMiter_Join, /*miterLimit=*/4.0f);
            for (const SkMatrix& ctm : {SkMatrix::I(), affine}) {
                for (bool simd : {false, true}) {
                    auto stroke = [&](const SkIRect& clip, Polyline* polyline) {
                        StrokeFlatten stroker;
                        return simd ? stroker.processStroke<FlattenMode::kSimd>(
                                              path, ctm, rec, clip, polyline)
                                    : stroker.processStroke<FlattenMode::kScalar>(
                                              path, ctm, rec, clip, polyline);
                    };
                    Polyline full;
                    REPORTER_ASSERT(reporter, stroke(everything, &full));
                    for (const SkIRect& clip : clips) {
                        Polyline culled;
                        REPORTER_ASSERT(reporter, stroke(clip, &culled));
                        fullPoints += static_cast<int>(full.count());
                        culledPoints += static_cast<int>(culled.count());
                        int mismatches = 0;
                        const SkRect bounds = SkRect::Make(clip);
                        for (float y = bounds.fTop + 0.27f; y < bounds.fBottom; y += 0.53f) {
                            for (float x = bounds.fLeft + 0.31f; x < bounds.fRight; x += 0.53f) {
                                if (winding_at(culled, {x, y}) != winding_at(full, {x, y})) {
                                    ++mismatches;
                                }
                            }
                        }
                        REPORTER_ASSERT(reporter,
                                        mismatches == 0,
                                        "%d samples changed winding (width %g, clip %d,%d,%d,%d)",
                                        mismatches,
                                        width,
                                        clip.fLeft,
                                        clip.fTop,
                                        clip.fRight,
                                        clip.fBottom);
                    }
                }
            }
        }
    }
    // The clips cut through the strokes, so some curves were culled.
    REPORTER_ASSERT(reporter, culledPoints < fullPoints, "%d vs %d", culledPoints, fullPoints);
}

// Inputs at the edges of the arithmetic: a line that turns back onto itself (a join without a
// miter direction), and radii so large or small that the sampling math degenerates.
DEF_TEST(SparseStrips_StrokeFlatten_ExtremeInputs, reporter) {
    const SkIRect clip = SkIRect::MakeWH(256, 256);
    auto stroke = [&](const SkPath& path,
                      const SkMatrix& ctm,
                      float width,
                      SkPaint::Join join,
                      SkPaint::Cap cap,
                      bool simd,
                      Polyline* polyline) {
        SkStrokeRec rec(SkStrokeRec::kFill_InitStyle);
        rec.setStrokeStyle(width);
        rec.setStrokeParams(cap, join, /*miterLimit=*/4.0f);
        StrokeFlatten stroker;
        return simd ? stroker.processStroke<FlattenMode::kSimd>(path, ctm, rec, clip, polyline)
                    : stroker.processStroke<FlattenMode::kScalar>(path, ctm, rec, clip, polyline);
    };
    // Finite points (or sentinels) forming closed chains.
    auto valid = [](const Polyline& polyline) {
        for (SkPoint p : polyline.points()) {
            const bool finite = std::isfinite(p.fX) && std::isfinite(p.fY);
            if (!finite && !(std::isnan(p.fX) && std::isnan(p.fY))) {
                return false;
            }
        }
        return all_chains_closed(polyline);
    };
    const SkPath reversal = SkPathBuilder().moveTo(10, 10).lineTo(60, 10).lineTo(20, 10).detach();
    for (bool simd : {false, true}) {
        for (SkPaint::Join join :
             {SkPaint::kMiter_Join, SkPaint::kRound_Join, SkPaint::kBevel_Join}) {
            Polyline polyline;
            REPORTER_ASSERT(
                    reporter,
                    stroke(reversal, SkMatrix::I(), 6, join, SkPaint::kButt_Cap, simd, &polyline));
            REPORTER_ASSERT(reporter, !polyline.empty() && valid(polyline));
        }
        // So large that 1 - tolerance / radius rounds to 1.
        Polyline huge;
        REPORTER_ASSERT(reporter,
                        stroke(path_s_curve(),
                               SkMatrix::I(),
                               1e8f,
                               SkPaint::kRound_Join,
                               SkPaint::kRound_Cap,
                               simd,
                               &huge));
        REPORTER_ASSERT(reporter, !huge.empty() && valid(huge));
        // So small that the radius underflows to 0: left to the caller.
        Polyline tiny;
        REPORTER_ASSERT(reporter,
                        !stroke(path_s_curve(),
                                SkMatrix::I(),
                                std::numeric_limits<float>::denorm_min(),
                                SkPaint::kRound_Join,
                                SkPaint::kRound_Cap,
                                simd,
                                &tiny));
        REPORTER_ASSERT(reporter, tiny.empty());
    }
}

DEF_TEST(SparseStrips_StrokeFlatten_Unsupported, reporter) {
    const SkPath path = path_zigzag();
    const SkIRect clip = SkIRect::MakeWH(256, 256);
    StrokeFlatten stroker;
    Polyline polyline;

    const SkStrokeRec fill(SkStrokeRec::kFill_InitStyle);
    REPORTER_ASSERT(reporter, !stroker.processStroke(path, SkMatrix::I(), fill, clip, &polyline));

    const SkStrokeRec hairline(SkStrokeRec::kHairline_InitStyle);
    REPORTER_ASSERT(reporter,
                    !stroker.processStroke(path, SkMatrix::I(), hairline, clip, &polyline));

    SkStrokeRec strokeAndFill(SkStrokeRec::kFill_InitStyle);
    strokeAndFill.setStrokeStyle(4, /*strokeAndFill=*/true);
    REPORTER_ASSERT(reporter,
                    !stroker.processStroke(path, SkMatrix::I(), strokeAndFill, clip, &polyline));

    SkStrokeRec stroke(SkStrokeRec::kFill_InitStyle);
    stroke.setStrokeStyle(4);
    SkMatrix perspective = SkMatrix::I();
    perspective.setPerspX(0.001f);
    REPORTER_ASSERT(reporter, !stroker.processStroke(path, perspective, stroke, clip, &polyline));

    REPORTER_ASSERT(reporter, polyline.empty());
}

}  // namespace skgpu::graphite
