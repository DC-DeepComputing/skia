/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathEffectBase.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace skiagm {


class ArcLengthDummyPathEffect : public SkPathEffectBase {
public:
    static sk_sp<SkPathEffect> Make() {
        return sk_sp<SkPathEffect>(new ArcLengthDummyPathEffect());
    }

    bool isArcLengthParameterized() const override { return true; }

    bool computeFastBounds(SkRect* bounds) const override { return true; }

protected:
    bool onFilterPath(SkPathBuilder*, const SkPath&, SkStrokeRec*, const SkRect*,
                      const SkMatrix&) const override {
        return false;
    }

    Factory getFactory() const override { return nullptr; }
    const char* getTypeName() const override { return "ArcLengthDummyPathEffect"; }
    Type getFlattenableType() const override { return kSkPathEffect_Type; }

private:
    ArcLengthDummyPathEffect() = default;
};

class ArcLengthStrokeGM : public GM {
protected:
    SkString getName() const override { return SkString("arclengthstroke"); }

    SkISize getISize() override { return SkISize::Make(500, 500); }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(30.f);
        paint.setStrokeCap(SkPaint::kRound_Cap);
        paint.setStrokeJoin(SkPaint::kRound_Join);
        paint.setPathEffect(ArcLengthDummyPathEffect::Make());
        paint.setColor(SK_ColorWHITE);

        // 1. A straight line
        {
            SkPath path = SkPathBuilder()
                .moveTo(50.f, 50.f)
                .lineTo(450.f, 50.f)
                .detach();
            canvas->drawPath(path, paint);
        }

        // 2. A quadratic curve
        {
            SkPath path = SkPathBuilder()
                .moveTo(50.f, 150.f)
                .quadTo(250.f, 300.f, 450.f, 150.f)
                .detach();
            canvas->drawPath(path, paint);
        }

        // 3. A cubic curve
        {
            SkPath path = SkPathBuilder()
                .moveTo(50.f, 350.f)
                .cubicTo(150.f, 200.f, 350.f, 500.f, 450.f, 350.f)
                .detach();
            canvas->drawPath(path, paint);
        }
    }
};

DEF_GM(return new ArcLengthStrokeGM;)

class ArcLengthSkiaGM : public GM {
protected:
    SkString getName() const override { return SkString("arclengthskia"); }

    SkISize getISize() override { return SkISize::Make(500, 220); }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(20.f);
        paint.setStrokeCap(SkPaint::kRound_Cap);
        paint.setStrokeJoin(SkPaint::kRound_Join);
        paint.setPathEffect(ArcLengthDummyPathEffect::Make());
        paint.setColor(SK_ColorWHITE);

        SkPath path = SkPathBuilder()
            // S
            .moveTo(140.f, 60.f)
            .cubicTo(110.f, 40.f, 60.f, 60.f, 80.f, 95.f)
            .cubicTo(95.f, 120.f, 135.f, 110.f, 135.f, 135.f)
            .cubicTo(135.f, 155.f, 90.f, 165.f, 60.f, 145.f)

            // k
            .moveTo(180.f, 40.f)
            .lineTo(180.f, 160.f)
            .moveTo(240.f, 70.f)
            .lineTo(180.f, 110.f)
            .lineTo(240.f, 160.f)

            // i
            .moveTo(280.f, 85.f)
            .lineTo(280.f, 160.f)
            .moveTo(280.f, 55.f)
            .lineTo(280.f, 56.f)

            // a
            .moveTo(380.f, 110.f)
            .cubicTo(380.f, 80.f, 320.f, 80.f, 320.f, 120.f)
            .cubicTo(320.f, 160.f, 380.f, 160.f, 380.f, 120.f)
            .lineTo(380.f, 160.f)
            .detach();

        canvas->drawPath(path, paint);
    }
};

DEF_GM(return new ArcLengthSkiaGM;)

class ArcLengthStressGM : public GM {
protected:
    SkString getName() const override { return SkString("arclengthstress"); }

    SkISize getISize() override { return SkISize::Make(800, 800); }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(2.f);
        paint.setStrokeCap(SkPaint::kRound_Cap);
        paint.setStrokeJoin(SkPaint::kRound_Join);
        paint.setPathEffect(ArcLengthDummyPathEffect::Make());
        paint.setColor(SK_ColorWHITE);

        SkPathBuilder builder;
        builder.moveTo(400.f, 400.f);
        for (int i = 0; i < 20000; ++i) {
            float t = i * 0.05f;
            float x = 400.f + cos(t) * (t * 0.3f);
            float y = 400.f + sin(t) * (t * 0.3f);
            float next_t = (i + 1) * 0.05f;
            float next_x = 400.f + cos(next_t) * (next_t * 0.3f);
            float next_y = 400.f + sin(next_t) * (next_t * 0.3f);

            float cp1_x = x + -sin(t) * 2.f;
            float cp1_y = y + cos(t) * 2.f;
            float cp2_x = next_x - -sin(next_t) * 2.f;
            float cp2_y = next_y - cos(next_t) * 2.f;

            builder.cubicTo(cp1_x, cp1_y, cp2_x, cp2_y, next_x, next_y);
        }
        SkPath path = builder.detach();
        canvas->drawPath(path, paint);
    }
};

DEF_GM(return new ArcLengthStressGM;)

namespace {

static constexpr float kStrokeWidth = 30;
static constexpr int kCellSize = 200;
static constexpr int kNumCols = 5;
static constexpr int kNumRows = 5;
static constexpr int kTestWidth = kNumCols * kCellSize;
static constexpr int kTestHeight = kNumRows * kCellSize;

enum class CellFillMode {
    kStretch,
    kCenter
};

struct TrickyCubic {
    SkPoint fPoints[4];
    int fNumPts;
    CellFillMode fFillMode;
    float fScale = 1;
};

static const TrickyCubic kTrickyCubics[] = {
    {{{122, 737}, {348, 553}, {403, 761}, {400, 760}}, 4, CellFillMode::kStretch},
    {{{244, 520}, {244, 518}, {1141, 634}, {394, 688}}, 4, CellFillMode::kStretch},
    {{{550, 194}, {138, 130}, {1035, 246}, {288, 300}}, 4, CellFillMode::kStretch},
    {{{226, 733}, {556, 779}, {-43, 471}, {348, 683}}, 4, CellFillMode::kStretch},
    {{{268, 204}, {492, 304}, {352, 23}, {433, 412}}, 4, CellFillMode::kStretch},

    {{{172, 480}, {396, 580}, {256, 299}, {338, 677}}, 4, CellFillMode::kStretch},

    {{{731, 340}, {318, 252}, {1026, -64}, {367, 265}}, 4, CellFillMode::kStretch},
    {{{475, 708}, {62, 620}, {770, 304}, {220, 659}}, 4, CellFillMode::kStretch},
    {{{0, 0}, {128, 128}, {128, 0}, {0, 128}}, 4, CellFillMode::kCenter},  // Perfect cusp
    {{{0,.01f}, {128,127.999f}, {128,.01f}, {0,127.99f}}, 4, CellFillMode::kCenter},  // Near-cusp
    {{{0,-.01f}, {128,128.001f}, {128,-.01f}, {0,128.001f}}, 4, CellFillMode::kCenter}, // Near-cusp
    {{{0,0}, {0,-10}, {0,-10}, {0,10}}, 4, CellFillMode::kCenter, 1.098283f},  // Flat line with 180
    {{{10,0}, {0,0}, {20,0}, {10,0}}, 4, CellFillMode::kStretch},  // Flat line with 2 180s
    {{{39,-39}, {40,-40}, {40,-40}, {0,0}}, 4, CellFillMode::kStretch},  // Flat diagonal with 180
    {{{39,-39}, {40,-40}, {37,-39}, {0,0}}, 4, CellFillMode::kStretch},  // Near-flat diagonal
    {{{40, 40}, {0, 0}, {200, 200}, {0, 0}}, 4, CellFillMode::kStretch},  // Diag w/ an internal 180
    {{{0,0}, {1e-2f,0}, {-1e-2f,0}, {0,0}}, 4, CellFillMode::kCenter},  // Circle
    {{{400.75f,100.05f}, {400.75f,100.05f}, {100.05f,300.95f}, {100.05f,300.95f}}, 4,
     CellFillMode::kStretch},  // Flat line with no turns
    {{{0.5f,0}, {0,0}, {20,0}, {10,0}}, 4, CellFillMode::kStretch},  // Flat line with 2 180s
    {{{10,0}, {0,0}, {10,0}, {10,0}}, 4, CellFillMode::kStretch},  // Flat line with a 180
    {{{1,1}, {2,1}, {1,1}, {1, std::numeric_limits<float>::quiet_NaN()}}, 3,
     CellFillMode::kStretch},  // Flat QUAD with a cusp
    {{{1,1}, {100,1}, {25,1}, {.3f, std::numeric_limits<float>::quiet_NaN()}}, 3,
     CellFillMode::kStretch},  // Flat CONIC with a cusp
    {{{1,1}, {100,1}, {25,1}, {1.5f, std::numeric_limits<float>::quiet_NaN()}}, 3,
     CellFillMode::kStretch},  // Flat CONIC with a cusp
};

static SkRect calc_tight_cubic_bounds(const SkPoint P[4], int depth=5) {
    if (0 == depth) {
        SkRect bounds;
        bounds.fLeft = std::min(std::min(P[0].x(), P[1].x()), std::min(P[2].x(), P[3].x()));
        bounds.fTop = std::min(std::min(P[0].y(), P[1].y()), std::min(P[2].y(), P[3].y()));
        bounds.fRight = std::max(std::max(P[0].x(), P[1].x()), std::max(P[2].x(), P[3].x()));
        bounds.fBottom = std::max(std::max(P[0].y(), P[1].y()), std::max(P[2].y(), P[3].y()));
        return bounds;
    }

    SkPoint chopped[7];
    SkChopCubicAt(P, chopped, .5f);
    SkRect bounds = calc_tight_cubic_bounds(chopped, depth - 1);
    bounds.join(calc_tight_cubic_bounds(chopped+3, depth - 1));
    return bounds;
}

static SkPoint lerp(const SkPoint& a, const SkPoint& b, float T) {
    SkASSERT(1 != T);  // The below does not guarantee lerp(a, b, 1) === b.
    return (b - a) * T + a;
}

}  // namespace

class ArcLengthTrickyGM : public GM {
protected:
    SkString getName() const override { return SkString("arclengthtricky"); }

    SkISize getISize() override { return SkISize::Make(kTestWidth, kTestHeight); }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        SkPaint strokePaint;
        strokePaint.setAntiAlias(true);
        strokePaint.setStrokeWidth(kStrokeWidth);
        strokePaint.setStyle(SkPaint::kStroke_Style);
        strokePaint.setStrokeCap(SkPaint::kRound_Cap);
        strokePaint.setStrokeJoin(SkPaint::kRound_Join);
        strokePaint.setPathEffect(ArcLengthDummyPathEffect::Make());
        strokePaint.setColor(SK_ColorWHITE);

        for (size_t i = 0; i < std::size(kTrickyCubics); ++i) {
            auto [originalPts, numPts, fillMode, scale] = kTrickyCubics[i];

            SkASSERT(numPts <= 4);
            SkPoint p[4];
            memcpy(p, originalPts, sizeof(SkPoint) * numPts);
            for (int j = 0; j < numPts; ++j) {
                p[j] *= scale;
            }
            float w = originalPts[3].fX;

            auto cellRect = SkRect::MakeXYWH((i % kNumCols) * kCellSize, (i / kNumCols) * kCellSize,
                                             kCellSize, kCellSize);

            SkRect strokeBounds;
            if (numPts == 4) {
                strokeBounds = calc_tight_cubic_bounds(p);
            } else {
                SkASSERT(numPts == 3);
                SkPoint asCubic[4] = {p[0], lerp(p[0], p[1], 2/3.f), lerp(p[1], p[2], 1/3.f), p[2]};
                strokeBounds = calc_tight_cubic_bounds(asCubic);
            }
            strokeBounds.outset(kStrokeWidth, kStrokeWidth);

            SkMatrix matrix = (fillMode == CellFillMode::kStretch) ?
                SkMatrix::RectToRectOrIdentity(strokeBounds, cellRect, SkMatrix::kCenter_ScaleToFit) :
                SkMatrix::Translate(cellRect.x() + kStrokeWidth +
                                    (cellRect.width() - strokeBounds.width()) / 2,
                                    cellRect.y() + kStrokeWidth +
                                    (cellRect.height() - strokeBounds.height()) / 2);

            SkAutoCanvasRestore acr(canvas, true);
            canvas->concat(matrix);
            strokePaint.setStrokeWidth(kStrokeWidth / matrix.getMaxScale());
            SkPathBuilder builder;
            builder.moveTo(p[0]);
            if (numPts == 4) {
                builder.cubicTo(p[1], p[2], p[3]);
            } else if (w == 1) {
                SkASSERT(numPts == 3);
                builder.quadTo(p[1], p[2]);
            } else {
                SkASSERT(numPts == 3);
                builder.conicTo(p[1], p[2], w);
            }
            canvas->drawPath(builder.detach(), strokePaint);
        }
    }
};

class ArcLengthCuspGM : public GM {
protected:
    SkString getName() const override { return SkString("arclengthcusp"); }

    SkISize getISize() override { return SkISize::Make(750, 520); }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        static const SkPaint::Join kJoins[] = {
            SkPaint::kMiter_Join,
            SkPaint::kRound_Join,
            SkPaint::kBevel_Join,
        };

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(25.f);
        paint.setStrokeCap(SkPaint::kRound_Cap);
        paint.setPathEffect(ArcLengthDummyPathEffect::Make());
        paint.setColor(SK_ColorWHITE);

        // 1. Swallowtail cubics (self-intersecting loops)
        for (int i = 0; i < 3; ++i) {
            float offsetX = 50.f + i * 230.f;
            float offsetY = 40.f;

            paint.setStrokeJoin(kJoins[i]);

            SkPath swallowtail = SkPathBuilder()
                .moveTo(offsetX, offsetY + 120.f)
                .cubicTo(offsetX + 180.f, offsetY, offsetX + 20.f, offsetY, offsetX + 180.f, offsetY + 120.f)
                .detach();

            canvas->drawPath(swallowtail, paint);
        }

        // 2. Perfect cusp cubics
        for (int i = 0; i < 3; ++i) {
            float offsetX = 50.f + i * 230.f;
            float offsetY = 200.f;

            paint.setStrokeJoin(kJoins[i]);

            // Perfect cusp cubic: P0(0,0), P1(120,120), P2(120,0), P3(0,120)
            SkPath cuspCubic = SkPathBuilder()
                .moveTo(offsetX, offsetY)
                .cubicTo(offsetX + 120.f, offsetY + 120.f, offsetX + 120.f, offsetY, offsetX, offsetY + 120.f)
                .detach();

            canvas->drawPath(cuspCubic, paint);
        }

        // 3. Sharp line join cusps
        for (int i = 0; i < 3; ++i) {
            float offsetX = 50.f + i * 230.f;
            float offsetY = 360.f;

            paint.setStrokeJoin(kJoins[i]);

            SkPath sharpCusp = SkPathBuilder()
                .moveTo(offsetX, offsetY + 40.f)
                .lineTo(offsetX + 150.f, offsetY + 10.f)
                .lineTo(offsetX + 10.f, offsetY + 50.f)
                .detach();

            canvas->drawPath(sharpCusp, paint);
        }
    }
};

DEF_GM(return new ArcLengthCuspGM;)
DEF_GM(return new ArcLengthTrickyGM;)

// Exercises how joins, caps, and contours are parameterized by arc length: joins of every type
// sweep around their inner corners (see skgpu::tess::ArcLengthAccumulator), while closing joins,
// hairline joins, caps, and cusps add no length. The parameter continues from one contour to the
// next.
class ArcLengthJoinsGM : public GM {
protected:
    SkString getName() const override { return SkString("arclengthjoins"); }

    SkISize getISize() override { return SkISize::Make(800, 600); }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        auto makePaint = [](float width, SkPaint::Join join, SkPaint::Cap cap) {
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(width);
            paint.setStrokeJoin(join);
            paint.setStrokeCap(cap);
            paint.setPathEffect(ArcLengthDummyPathEffect::Make());
            paint.setColor(SK_ColorWHITE);
            return paint;
        };
        // Calls 'draw' with the canvas translated to the cell at (col, row).
        auto cell = [canvas](int col, int row, auto&& draw) {
            SkAutoCanvasRestore acr(canvas, true);
            canvas->translate(col * 200.f, row * 200.f);
            draw();
        };

        const SkPath zigzag = SkPathBuilder()
                .moveTo(25, 150).lineTo(65, 45).lineTo(105, 155).lineTo(145, 45).lineTo(180, 150)
                .detach();
        const SkPath smallZigzag = SkPathBuilder()
                .moveTo(25, 80).lineTo(65, 25).lineTo(105, 85).lineTo(145, 25).lineTo(180, 80)
                .detach();

        // Row 0: round joins.
        cell(0, 0, [&] {  // Sharp joins on an open polyline
            canvas->drawPath(zigzag, makePaint(30, SkPaint::kRound_Join, SkPaint::kRound_Cap));
        });
        cell(1, 0, [&] {  // A closed star: many sharp joins, then a zero-length closing join
            SkPathBuilder star;
            for (int i = 0; i < 10; ++i) {
                float r = (i % 2) ? 32.f : 82.f;
                float a = SK_ScalarPI * (i / 5.f - 0.5f);
                SkPoint p = {100 + r * std::cos(a), 105 + r * std::sin(a)};
                if (i == 0) {
                    star.moveTo(p);
                } else {
                    star.lineTo(p);
                }
            }
            star.close();
            canvas->drawPath(star.detach(),
                             makePaint(16, SkPaint::kRound_Join, SkPaint::kButt_Cap));
        });
        cell(2, 0, [&] {  // A closed triangle with a wide stroke
            SkPath triangle = SkPathBuilder()
                    .moveTo(35, 165).lineTo(100, 40).lineTo(165, 165).close()
                    .detach();
            canvas->drawPath(triangle, makePaint(40, SkPaint::kRound_Join, SkPaint::kButt_Cap));
        });
        cell(3, 0, [&] {  // Two contours: the parameter continues from the first to the second
            SkPath path = SkPathBuilder()
                    .moveTo(25, 70).lineTo(100, 30).lineTo(175, 70)
                    .moveTo(25, 170).lineTo(100, 120).lineTo(175, 170)
                    .detach();
            canvas->drawPath(path, makePaint(24, SkPaint::kRound_Join, SkPaint::kRound_Cap));
        });

        // Row 1: caps, other join types, and hairlines (whose joins add no length).
        cell(0, 1, [&] {  // Butt caps
            canvas->drawPath(zigzag, makePaint(26, SkPaint::kRound_Join, SkPaint::kButt_Cap));
        });
        cell(1, 1, [&] {  // Square caps
            canvas->drawPath(zigzag, makePaint(26, SkPaint::kRound_Join, SkPaint::kSquare_Cap));
        });
        cell(2, 1, [&] {  // Miter (top) and bevel (bottom) joins
            canvas->drawPath(smallZigzag, makePaint(18, SkPaint::kMiter_Join, SkPaint::kButt_Cap));
            canvas->translate(0, 95);
            canvas->drawPath(smallZigzag, makePaint(18, SkPaint::kBevel_Join, SkPaint::kButt_Cap));
        });
        cell(3, 1, [&] {  // A hairline (top) and a thin stroke (bottom)
            canvas->drawPath(smallZigzag, makePaint(0, SkPaint::kRound_Join, SkPaint::kButt_Cap));
            canvas->translate(0, 95);
            canvas->drawPath(smallZigzag, makePaint(3, SkPaint::kRound_Join, SkPaint::kRound_Cap));
        });

        // Row 2: transforms, reversals, cusps, and paths without length.
        cell(0, 2, [&] {  // Non-uniform scale (joins are measured in local coordinates)
            canvas->scale(1.8f, 0.6f);
            SkPath path = SkPathBuilder()
                    .moveTo(15, 290).quadTo(55, 20, 95, 290).lineTo(25, 190)
                    .detach();
            canvas->drawPath(path, makePaint(16, SkPaint::kRound_Join, SkPaint::kRound_Cap));
        });
        cell(1, 2, [&] {  // Joins that turn nearly 180 degrees
            SkPath path = SkPathBuilder()
                    .moveTo(25, 40).lineTo(175, 60).lineTo(25, 90).lineTo(175, 130).lineTo(30, 165)
                    .detach();
            canvas->drawPath(path, makePaint(22, SkPaint::kRound_Join, SkPaint::kRound_Cap));
        });
        cell(2, 2, [&] {  // A cusp adds no length, and neither does the join after it
            SkPath path = SkPathBuilder()
                    .moveTo(40, 40).cubicTo(170, 170, 170, 40, 40, 170)
                    .detach();
            canvas->drawPath(path, makePaint(24, SkPaint::kRound_Join, SkPaint::kRound_Cap));
        });
        cell(3, 2, [&] {  // Zero-length contours only draw caps
            SkPaint paint = makePaint(30, SkPaint::kRound_Join, SkPaint::kRound_Cap);
            // A path that is only a dot has no length at all.
            canvas->drawPath(SkPathBuilder().moveTo(50, 50).lineTo(50, 50).detach(), paint);
            // Dots before and after a line sit at the start and end of the parameter.
            canvas->drawPath(SkPathBuilder()
                                     .moveTo(40, 150).lineTo(40, 150)
                                     .moveTo(80, 150).lineTo(130, 110)
                                     .moveTo(170, 150).lineTo(170, 150)
                                     .detach(),
                             paint);
        });
    }
};

// Curves big enough on screen that PatchWriter chops each one into two patches to fit the
// tessellation limits. The parameter must stay continuous across the chop (at each apex).
class ArcLengthChoppedGM : public GM {
protected:
    SkString getName() const override { return SkString("arclengthchopped"); }

    SkISize getISize() override { return SkISize::Make(1000, 500); }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(40.f);
        paint.setStrokeCap(SkPaint::kRound_Cap);
        paint.setStrokeJoin(SkPaint::kRound_Join);
        paint.setPathEffect(ArcLengthDummyPathEffect::Make());
        paint.setColor(SK_ColorWHITE);

        canvas->drawPath(SkPathBuilder().moveTo(30, 470).cubicTo(60, -130, 440, -130, 470, 470)
                                        .detach(),
                         paint);
        canvas->drawPath(SkPathBuilder().moveTo(530, 470).quadTo(750, -420, 970, 470).detach(),
                         paint);
    }
};

// Joins sweep around their inner corners (see skgpu::tess::ArcLengthAccumulator). The first two
// paths match the CPU mock-up of that parameterization, for comparison:
//   - Left: joins of ~32, 90, ~132 (whose full trim needs most of the short segment after it), and
//     ~2 degrees.
//   - Middle: joins of ~161 (whose full trim also fits), ~120, and ~43 degrees.
//   - Right: a polyline arc that's narrower than the stroke, so every join's trim is limited by the
//     segments on either side, and the inner side of the stroke overlaps itself.
class ArcLengthCornersGM : public GM {
protected:
    SkString getName() const override { return SkString("arclengthcorners"); }

    SkISize getISize() override { return SkISize::Make(990, 250); }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        auto makePaint = [](float width) {
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(width);
            paint.setStrokeJoin(SkPaint::kRound_Join);
            paint.setStrokeCap(SkPaint::kButt_Cap);
            paint.setPathEffect(ArcLengthDummyPathEffect::Make());
            paint.setColor(SK_ColorWHITE);
            return paint;
        };

        canvas->drawPath(SkPathBuilder().moveTo(20, 200).lineTo(100, 150).lineTo(170, 150)
                                        .lineTo(170, 70).lineTo(215, 110).lineTo(300, 190)
                                        .detach(),
                         makePaint(32));

        canvas->save();
        canvas->translate(330, 10);
        canvas->drawPath(SkPathBuilder().moveTo(20, 180).lineTo(120, 40).lineTo(75, 190)
                                        .lineTo(240, 150).lineTo(300, 60)
                                        .detach(),
                         makePaint(28));
        canvas->restore();

        SkPathBuilder arc;
        for (int i = 0; i <= 8; ++i) {
            float angle = SK_ScalarPI * (-0.25f + i * 1.5f / 8);
            SkPoint p = {825 + 30 * std::cos(angle), 125 + 30 * std::sin(angle)};
            if (i == 0) {
                arc.moveTo(p);
            } else {
                arc.lineTo(p);
            }
        }
        canvas->drawPath(arc.detach(), makePaint(70));
    }
};

DEF_GM(return new ArcLengthJoinsGM;)
DEF_GM(return new ArcLengthChoppedGM;)
DEF_GM(return new ArcLengthCornersGM;)

// Bends tighter than the stroke radius, where the stroke's inner side overlaps itself, fold (see
// skgpu::tess::ArcLengthFold): the arc length sweeps around the inner corner where the stroke's
// inner edge crosses itself, like it does at a join, so the overlapping strokes agree.
//   - Row 0: the swallowtail from arclengthcusp as its second-to-last control point moves right:
//     from a loop that the stroke covers (P2.x = -100), which folds like a cusp, through a cusp
//     (P2.x = 50), to where the fold vanishes (~205). The fold turns ~85-110 degrees until the
//     bend opens up.
//   - Row 1: a quad's and a conic's sharp apexes, a cusp, a fold that shrinks to fit within its
//     short quad, and a swallowtail between sharp joins.
//   - Row 2: the limits. A cubic with two sharp bends close together, whose folds overlap and turn
//     opposite ways, so they share a border (the line through their corners). A loop whose hole
//     shows folds at its tip, rather than like a cusp. A rounded rect whose corners are tighter
//     than the stroke, which folds that span verbs cover (except where it starts and ends; see
//     F9). A short line between two sharp joins whose trims still fit on it, so it doesn't fold.
//   - Row 3: folds that span verbs (F2b). Short lines between two sharp joins whose trims overlap
//     on them, with equal and unequal turns. A short side between two rounded corners, whose folds
//     share it. A gentle arc between two sharp joins. A near-reversal made of two joins (see E1).
class ArcLengthFoldsGM : public GM {
protected:
    SkString getName() const override { return SkString("arclengthfolds"); }

    SkISize getISize() override { return SkISize::Make(1150, 800); }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        auto makePaint = [](float width, SkPaint::Join join = SkPaint::kRound_Join) {
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(width);
            paint.setStrokeJoin(join);
            paint.setStrokeCap(SkPaint::kButt_Cap);
            paint.setPathEffect(ArcLengthDummyPathEffect::Make());
            paint.setColor(SK_ColorWHITE);
            return paint;
        };
        // Calls 'draw' with the canvas translated to the cell at (col, row).
        auto cell = [canvas](int col, int row, auto&& draw) {
            SkAutoCanvasRestore acr(canvas, true);
            canvas->translate(col * 230.f, row * 200.f);
            draw();
        };
        auto swallowtail = [](float p2x) {
            return SkPathBuilder().moveTo(50, 160).cubicTo(230, 40, p2x, 40, 230, 160).detach();
        };

        // Row 0: the swallowtail family.
        const float kP2X[] = {-100, 50, 70, 130, 200};
        for (int i = 0; i < 5; ++i) {
            cell(i, 0, [&] {
                canvas->translate(-25, 10);
                canvas->drawPath(swallowtail(kP2X[i]), makePaint(25));
            });
        }

        // Row 1: sharp apexes, a cusp, a shrunk fold, and a fold between joins.
        cell(0, 1, [&] {  // A quad's apex
            canvas->drawPath(SkPathBuilder().moveTo(30, 180).quadTo(115, -110, 200, 180).detach(),
                             makePaint(30));
        });
        cell(1, 1, [&] {  // A conic's apex
            canvas->drawPath(
                    SkPathBuilder().moveTo(30, 180).conicTo(115, -40, 200, 180, 3).detach(),
                    makePaint(30));
        });
        cell(2, 1, [&] {  // A cusp
            canvas->drawPath(SkPathBuilder().moveTo(40, 35).cubicTo(180, 175, 180, 35, 40, 175)
                                            .detach(),
                             makePaint(25));
        });
        cell(3, 1, [&] {  // A short apex, whose fold shrinks until it covers the whole quad
            canvas->drawPath(SkPathBuilder().moveTo(85, 175).quadTo(115, 15, 145, 175).detach(),
                             makePaint(40));
        });
        cell(4, 1, [&] {  // A swallowtail between sharp (miter) joins
            canvas->scale(0.8f, 0.8f);
            canvas->drawPath(SkPathBuilder().moveTo(20, 60).lineTo(50, 220)
                                            .cubicTo(230, 100, 70, 100, 230, 220).lineTo(260, 70)
                                            .detach(),
                             makePaint(25, SkPaint::kMiter_Join));
        });

        // Row 2: the limits.
        cell(0, 2, [&] {  // Two sharp bends close together
            canvas->translate(20, 70);
            canvas->scale(1.9f, 1.9f);
            canvas->drawPath(SkPathBuilder().moveTo(0, 0).cubicTo(100, 0, 0, 20, 100, 20)
                                            .detach(),
                             makePaint(24));
        });
        cell(1, 2, [&] {  // A loop whose hole shows
            canvas->translate(-15, 0);
            canvas->drawPath(swallowtail(-250), makePaint(25));
        });
        cell(2, 2, [&] {  // Rounded corners tighter than the stroke (folds span verbs)
            constexpr float w = SK_ScalarRoot2Over2;
            canvas->drawPath(SkPathBuilder().moveTo(48, 40).lineTo(182, 40)
                                            .conicTo(190, 40, 190, 48, w).lineTo(190, 162)
                                            .conicTo(190, 170, 182, 170, w).lineTo(48, 170)
                                            .conicTo(40, 170, 40, 162, w).lineTo(40, 48)
                                            .conicTo(40, 40, 48, 40, w).close()
                                            .detach(),
                             makePaint(36));
        });
        cell(3, 2, [&] {  // A short line between two sharp joins, whose trims fit
            canvas->drawPath(SkPathBuilder().moveTo(30, 170).lineTo(105, 30).lineTo(125, 30)
                                            .lineTo(200, 170).detach(),
                             makePaint(30));
        });
        cell(4, 2, [&] {  // A hairline doesn't fold
            canvas->translate(-25, 10);
            canvas->drawPath(swallowtail(70), makePaint(0));
        });

        // Row 3: folds that span verbs.
        cell(0, 3, [&] {  // A short line between two sharp joins, whose trims overlap on it
            canvas->drawPath(SkPathBuilder().moveTo(30, 170).lineTo(110, 30).lineTo(120, 30)
                                            .lineTo(200, 170).detach(),
                             makePaint(30));
        });
        cell(1, 3, [&] {  // The same, with unequal turns
            canvas->drawPath(SkPathBuilder().moveTo(20, 170).lineTo(100, 40).lineTo(108, 40)
                                            .lineTo(200, 120).detach(),
                             makePaint(30));
        });
        cell(2, 3, [&] {  // A short side between two rounded corners, whose folds share it
            constexpr float w = SK_ScalarRoot2Over2;
            canvas->drawPath(SkPathBuilder().moveTo(40, 70).lineTo(182, 70)
                                            .conicTo(190, 70, 190, 78, w).lineTo(190, 92)
                                            .conicTo(190, 100, 182, 100, w).lineTo(40, 100)
                                            .detach(),
                             makePaint(36));
        });
        cell(3, 3, [&] {  // A gentle arc between two sharp joins
            canvas->drawPath(SkPathBuilder().moveTo(52, 172).lineTo(100, 40)
                                            .conicTo(105.21f, 39.08f, 110.42f, 40, 0.9848f)
                                            .lineTo(158, 172).detach(),
                             makePaint(30));
        });
        cell(4, 3, [&] {  // A near-reversal made of two joins
            canvas->drawPath(SkPathBuilder().moveTo(99.58f, 109.09f).lineTo(110, 50)
                                            .lineTo(122, 50).lineTo(132.42f, 109.09f).detach(),
                             makePaint(30));
        });
    }
};

DEF_GM(return new ArcLengthFoldsGM;)

// Zigzags: consecutive joins that turn opposite ways, whose full trims (see
// skgpu::tess::ArcLengthAccumulator) overlap on the segment between them, so they share a border
// between their corners instead of shrinking them (see ArcLengthAccumulator::finish()):
//   - A wide M, with joins of ~144, ~140, and ~144 degrees. The middle join has a border on each
//     side.
//   - A wide, flatter M, with joins of ~110-114 degrees.
//   - Near-reversals (~166 and ~160 degrees). Only the first two joins' trims overlap.
//   - Unequal turns (~149 and ~110 degrees).
//   - A near-reversal (~167 degrees) next to a mild turn (~60 degrees), on a segment shorter than
//     the near-reversal's full trim alone. Its corner still shrinks, and the border reaches the
//     mild turn's outer side, which leaves a cliff there (E5).
class ArcLengthZigzagGM : public GM {
protected:
    SkString getName() const override { return SkString("arclengthzigzag"); }

    SkISize getISize() override { return SkISize::Make(1200, 220); }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        auto makePaint = [](float width) {
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(width);
            paint.setStrokeJoin(SkPaint::kRound_Join);
            paint.setStrokeCap(SkPaint::kButt_Cap);
            paint.setPathEffect(ArcLengthDummyPathEffect::Make());
            paint.setColor(SK_ColorWHITE);
            return paint;
        };
        // Calls 'draw' with the canvas translated to the cell at 'col'.
        auto cell = [canvas](int col, auto&& draw) {
            SkAutoCanvasRestore acr(canvas, true);
            canvas->translate(col * 240.f, 0);
            draw();
        };

        cell(0, [&] {  // A wide M
            canvas->drawPath(SkPathBuilder().moveTo(40, 170).lineTo(75, 50).lineTo(120, 175)
                                            .lineTo(165, 50).lineTo(200, 170).detach(),
                             makePaint(56));
        });
        cell(1, [&] {  // A wide, flatter M
            canvas->drawPath(SkPathBuilder().moveTo(40, 130).lineTo(80, 75).lineTo(120, 135)
                                            .lineTo(160, 75).lineTo(195, 130).detach(),
                             makePaint(56));
        });
        cell(2, [&] {  // Near-reversals
            canvas->drawPath(SkPathBuilder().moveTo(40, 50).lineTo(200, 62).lineTo(40, 90)
                                            .lineTo(200, 118).lineTo(40, 146).detach(),
                             makePaint(26));
        });
        cell(3, [&] {  // Unequal turns
            canvas->drawPath(SkPathBuilder().moveTo(40, 190).lineTo(100, 40).lineTo(120, 160)
                                            .lineTo(210, 110).detach(),
                             makePaint(60));
        });
        cell(4, [&] {  // A near-reversal next to a mild turn
            canvas->drawPath(SkPathBuilder().moveTo(40, 40).lineTo(200, 55).lineTo(140, 63)
                                            .lineTo(94, 174).detach(),
                             makePaint(20));
        });
    }
};

DEF_GM(return new ArcLengthZigzagGM;)

}  // namespace skiagm


