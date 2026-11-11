/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/core/SkStrokeRec.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/StrokeFlatten.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

// Compares the fused StrokeFlatten against the SkStroke + Flatten pipeline it replaces:
//   skstroke_flatten: SkStrokeRec::applyToPath() followed by Flatten (the existing pipeline)
//   flatten_stroked:  Flatten of the pre-stroked path only (the Flatten share of the above)
//   strokeflatten:    StrokeFlatten (fused)
//   fill_flatten:     Flatten of the un-stroked path, i.e. the pure flattening (fill) cost
// Each bench prints the resulting Polyline point count once during setup.

namespace skgpu::graphite {
namespace {

constexpr int kCanvasSize = 1024;
constexpr float kTwoPi = 6.28318531f;

// Deterministic xorshift32, so every run strokes identical geometry.
class Rand {
public:
    explicit Rand(uint32_t seed) : fState(seed) {}

    float next(float lo, float hi) {
        fState ^= fState << 13;
        fState ^= fState >> 17;
        fState ^= fState << 5;
        return lo + (hi - lo) * static_cast<float>(fState >> 8) * (1.0f / 16777216.0f);
    }

private:
    uint32_t fState;
};

// A 1000 vertex chart-like polyline.
SkPath make_polyline() {
    Rand rand(1);
    SkPathBuilder builder;
    float y = 512.0f;
    builder.moveTo(10.0f, y);
    for (int i = 1; i < 1000; ++i) {
        y = std::clamp(y + rand.next(-12.0f, 12.0f), 20.0f, 1000.0f);
        builder.lineTo(10.0f + static_cast<float>(i), y);
    }
    return builder.detach();
}

// 500 short open line contours, as produced by dashing.
SkPath make_dashes() {
    Rand rand(2);
    SkPathBuilder builder;
    for (int i = 0; i < 500; ++i) {
        const float x = rand.next(20.0f, 1000.0f);
        const float y = rand.next(20.0f, 1000.0f);
        const float angle = rand.next(0.0f, kTwoPi);
        const float length = rand.next(4.0f, 12.0f);
        builder.moveTo(x, y);
        builder.lineTo(x + length * std::cos(angle), y + length * std::sin(angle));
    }
    return builder.detach();
}

// 64 closed, icon-sized blobs made of cubics.
SkPath make_icons() {
    Rand rand(3);
    SkPathBuilder builder;
    constexpr int kSegments = 6;
    for (int i = 0; i < 64; ++i) {
        const float cx = 60.0f + static_cast<float>(i % 8) * 120.0f;
        const float cy = 60.0f + static_cast<float>(i / 8) * 120.0f;
        auto polar = [cx, cy](float radius, float theta) {
            return SkPoint{cx + radius * std::cos(theta), cy + radius * std::sin(theta)};
        };
        const SkPoint start = polar(rand.next(20.0f, 40.0f), 0.0f);
        builder.moveTo(start);
        for (int k = 1; k <= kSegments; ++k) {
            const float a0 = kTwoPi * static_cast<float>(k - 1) / kSegments;
            const float a1 = kTwoPi * static_cast<float>(k) / kSegments;
            const SkPoint c1 = polar(rand.next(10.0f, 50.0f), a0 + (a1 - a0) / 3.0f);
            const SkPoint c2 = polar(rand.next(10.0f, 50.0f), a0 + 2.0f * (a1 - a0) / 3.0f);
            const SkPoint end = k == kSegments ? start : polar(rand.next(20.0f, 40.0f), a1);
            builder.cubicTo(c1, c2, end);
        }
        builder.close();
    }
    return builder.detach();
}

// 100 open, randomly shaped cubics spanning up to ~300px.
SkPath make_curves() {
    Rand rand(4);
    SkPathBuilder builder;
    for (int i = 0; i < 100; ++i) {
        const SkPoint p0 = {rand.next(100.0f, 900.0f), rand.next(100.0f, 900.0f)};
        SkPoint p[3];
        for (SkPoint& pt : p) {
            pt = {p0.fX + rand.next(-150.0f, 150.0f), p0.fY + rand.next(-150.0f, 150.0f)};
        }
        builder.moveTo(p0);
        builder.cubicTo(p[0], p[1], p[2]);
    }
    return builder.detach();
}

// 300 short chains of quadratics, similar to TrueType glyph outlines.
SkPath make_quads() {
    Rand rand(5);
    SkPathBuilder builder;
    for (int i = 0; i < 300; ++i) {
        SkPoint pt = {rand.next(20.0f, 1000.0f), rand.next(20.0f, 1000.0f)};
        builder.moveTo(pt);
        for (int k = 0; k < 4; ++k) {
            const SkPoint ctrl = {pt.fX + rand.next(-12.0f, 12.0f),
                                  pt.fY + rand.next(-12.0f, 12.0f)};
            pt = {ctrl.fX + rand.next(-12.0f, 12.0f), ctrl.fY + rand.next(-12.0f, 12.0f)};
            builder.quadTo(ctrl, pt);
        }
    }
    return builder.detach();
}

// 100 circles (conics).
SkPath make_circles() {
    Rand rand(6);
    SkPathBuilder builder;
    for (int i = 0; i < 100; ++i) {
        const SkPoint center = {rand.next(60.0f, 960.0f), rand.next(60.0f, 960.0f)};
        builder.addCircle(center, rand.next(4.0f, 40.0f));
    }
    return builder.detach();
}

// Magnified 4x about the center of the canvas, as when zoomed in: most of the geometry is
// outside the clip.
SkPath zoom(const SkPath& path) {
    SkMatrix matrix;
    matrix.setScale(4.0f, 4.0f, 0.5f * kCanvasSize, 0.5f * kCanvasSize);
    return path.makeTransform(matrix);
}

SkPath make_curves_zoomed() { return zoom(make_curves()); }
SkPath make_quads_zoomed() { return zoom(make_quads()); }

enum class Method {
    kSkStrokeThenFlatten,
    kFlattenStrokedPath,
    kStrokeFlatten,
    kStrokeFlattenScalar,
    kFillFlatten,
};

const char* method_name(Method method) {
    switch (method) {
        case Method::kSkStrokeThenFlatten:
            return "skstroke_flatten";
        case Method::kFlattenStrokedPath:
            return "flatten_stroked";
        case Method::kStrokeFlatten:
            return "strokeflatten";
        case Method::kStrokeFlattenScalar:
            return "strokeflatten_scalar";
        case Method::kFillFlatten:
            return "fill_flatten";
    }
    return "";
}

class StrokeFlattenBench final : public Benchmark {
public:
    StrokeFlattenBench(const char* shape,
                       SkPath (*makePath)(),
                       float width,
                       SkPaint::Join join,
                       SkPaint::Cap cap,
                       Method method)
            : fMakePath(makePath), fMethod(method), fStroke(SkStrokeRec::kFill_InitStyle) {
        fStroke.setStrokeStyle(width);
        fStroke.setStrokeParams(cap, join, /*miterLimit=*/4.0f);
        fName.printf("sparse_strips_stroke_%s_%s", shape, method_name(method));
    }

    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDelayedSetup() override {
        fPath = fMakePath();
        SkPathBuilder stroked;
        if (fStroke.applyToPath(&stroked, fPath)) {
            fStrokedPath = stroked.detach();
        }
        this->run();
        SkDebugf("%s: %d polyline points\n", fName.c_str(), static_cast<int>(fPolyline.count()));
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            this->run();
        }
    }

private:
    void run() {
        const SkIRect clip = SkIRect::MakeWH(kCanvasSize, kCanvasSize);
        fPolyline.reset();
        switch (fMethod) {
            case Method::kSkStrokeThenFlatten: {
                SkPathBuilder stroked;
                if (fStroke.applyToPath(&stroked, fPath)) {
                    fFlatten.processPaths<FlattenMode::kSimd>(
                            stroked.detach(), SkMatrix::I(), clip, &fPolyline);
                }
                break;
            }
            case Method::kFlattenStrokedPath:
                fFlatten.processPaths<FlattenMode::kSimd>(
                        fStrokedPath, SkMatrix::I(), clip, &fPolyline);
                break;
            case Method::kStrokeFlatten:
                fStrokeFlatten.processStroke<FlattenMode::kSimd>(
                        fPath, SkMatrix::I(), fStroke, clip, &fPolyline);
                break;
            case Method::kStrokeFlattenScalar:
                fStrokeFlatten.processStroke<FlattenMode::kScalar>(
                        fPath, SkMatrix::I(), fStroke, clip, &fPolyline);
                break;
            case Method::kFillFlatten:
                fFlatten.processPaths<FlattenMode::kSimd>(fPath, SkMatrix::I(), clip, &fPolyline);
                break;
        }
    }

    SkPath (*fMakePath)();
    Method fMethod;
    SkStrokeRec fStroke;
    SkString fName;
    SkPath fPath;
    SkPath fStrokedPath;
    Polyline fPolyline;
    Flatten fFlatten;
    StrokeFlatten fStrokeFlatten;
};

}  // namespace

#define DEF_STROKE_FLATTEN_BENCHES(SHAPE, MAKER, WIDTH, JOIN, CAP)                                 \
    DEF_BENCH(return new StrokeFlattenBench(                                                       \
            SHAPE, MAKER, WIDTH, JOIN, CAP, Method::kSkStrokeThenFlatten))                         \
    DEF_BENCH(return new StrokeFlattenBench(                                                       \
            SHAPE, MAKER, WIDTH, JOIN, CAP, Method::kFlattenStrokedPath))                          \
    DEF_BENCH(                                                                                     \
            return new StrokeFlattenBench(SHAPE, MAKER, WIDTH, JOIN, CAP, Method::kStrokeFlatten)) \
    DEF_BENCH(return new StrokeFlattenBench(                                                       \
            SHAPE, MAKER, WIDTH, JOIN, CAP, Method::kStrokeFlattenScalar))                         \
    DEF_BENCH(return new StrokeFlattenBench(SHAPE, MAKER, WIDTH, JOIN, CAP, Method::kFillFlatten))

DEF_STROKE_FLATTEN_BENCHES(
        "polyline", make_polyline, 2.0f, SkPaint::kMiter_Join, SkPaint::kButt_Cap)
DEF_STROKE_FLATTEN_BENCHES("dashes", make_dashes, 2.0f, SkPaint::kMiter_Join, SkPaint::kButt_Cap)
DEF_STROKE_FLATTEN_BENCHES(
        "dashes_round", make_dashes, 2.0f, SkPaint::kRound_Join, SkPaint::kRound_Cap)
DEF_STROKE_FLATTEN_BENCHES("icons", make_icons, 2.0f, SkPaint::kRound_Join, SkPaint::kRound_Cap)
DEF_STROKE_FLATTEN_BENCHES("curves", make_curves, 4.0f, SkPaint::kRound_Join, SkPaint::kButt_Cap)
DEF_STROKE_FLATTEN_BENCHES(
        "curves_thick", make_curves, 24.0f, SkPaint::kMiter_Join, SkPaint::kButt_Cap)
DEF_STROKE_FLATTEN_BENCHES("quads", make_quads, 1.5f, SkPaint::kMiter_Join, SkPaint::kButt_Cap)
DEF_STROKE_FLATTEN_BENCHES("circles", make_circles, 3.0f, SkPaint::kMiter_Join, SkPaint::kButt_Cap)
DEF_STROKE_FLATTEN_BENCHES(
        "curves_zoomed", make_curves_zoomed, 16.0f, SkPaint::kRound_Join, SkPaint::kButt_Cap)
DEF_STROKE_FLATTEN_BENCHES(
        "quads_zoomed", make_quads_zoomed, 6.0f, SkPaint::kMiter_Join, SkPaint::kButt_Cap)

}  // namespace skgpu::graphite
