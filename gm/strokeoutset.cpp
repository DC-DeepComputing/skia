/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "src/core/SkStroke.h"

namespace skiagm {

struct TrickyCurve {
    SkPoint fPts[4];
    int fNumPts;
    float fWeight = 1.0f;
};

static const TrickyCurve kTrickyCurves[] = {
    {{{122, 737}, {348, 553}, {403, 761}, {400, 760}}, 4},
    {{{244, 520}, {244, 518}, {1141, 634}, {394, 688}}, 4},
    {{{550, 194}, {138, 130}, {1035, 246}, {288, 300}}, 4},
    {{{226, 733}, {556, 779}, {-43, 471}, {348, 683}}, 4},
    {{{268, 204}, {492, 304}, {352, 23}, {433, 412}}, 4},
    {{{172, 480}, {396, 580}, {256, 299}, {338, 677}}, 4},
    {{{731, 340}, {318, 252}, {1026, -64}, {367, 265}}, 4},
    {{{475, 708}, {62, 620}, {770, 304}, {220, 659}}, 4},
    {{{0, 0}, {128, 128}, {128, 0}, {0, 128}}, 4},
    {{{0, 0}, {0, -10}, {0, -10}, {0, 10}}, 4, 1.098283f},
    {{{10, 0}, {0, 0}, {20, 0}, {10, 0}}, 4},
};

static SkPath make_star() {
    SkPathBuilder star;
    star.moveTo(50, 0);
    for (int i = 1; i < 5; ++i) {
        SkScalar theta = i * 4 * SK_ScalarPI / 5;
        star.lineTo(50 + 50 * SkScalarSin(theta), 50 - 50 * SkScalarCos(theta));
    }
    star.close();

    SkRect bounds = star.computeBounds();
    SkMatrix matrix;
    matrix.setRectToRect(bounds, {85, 85, 115, 115}, SkMatrix::kFill_ScaleToFit);
    return star.detach().makeTransform(matrix);
}

static SkPath make_circle() {
    return SkPath::Circle(100, 100, 15);
}

static SkPath make_rect() {
    return SkPath::Rect({85, 85, 115, 115});
}

static SkPath make_rrect() {
    return SkPath::RRect(SkRect::MakeLTRB(85, 85, 115, 115), 6, 6);
}

static SkPath make_ellipse() {
    return SkPath::Oval(SkRect::MakeLTRB(80, 90, 120, 110));
}

static SkPath make_tricky_curve(const TrickyCurve& tc) {
    SkPathBuilder builder;
    builder.moveTo(tc.fPts[0]);
    if (tc.fNumPts == 4) {
        builder.cubicTo(tc.fPts[1], tc.fPts[2], tc.fPts[3]);
    } else if (tc.fNumPts == 3) {
        builder.conicTo(tc.fPts[1], tc.fPts[2], tc.fWeight);
    }
    SkPath path = builder.detach();

    SkRect bounds = path.getBounds();
    if (bounds.isEmpty()) {
        bounds.setLTRB(0, 0, 128, 128);
    }
    SkMatrix matrix;
    matrix.setRectToRect(bounds, {85, 85, 115, 115}, SkMatrix::kFill_ScaleToFit);
    return path.makeTransform(matrix);
}

class StrokeOutsetGM : public GM {
public:
    StrokeOutsetGM() {}

protected:
    SkString getName() const override { return SkString("strokeoutset"); }

    SkISize getISize() override { return SkISize::Make(1240, 1240); }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE); // Single flat white background

        // Prepare paths to draw
        struct NamedPath {
            SkPath fPath;
            const char* fName;
        };

        std::vector<NamedPath> shapes;

        // 1. Standard Closed Geometries
        shapes.push_back({make_star(), "Star"});
        shapes.push_back({make_circle(), "Circle"});
        shapes.push_back({make_rect(), "Rect"});
        shapes.push_back({make_rrect(), "RRect"});
        shapes.push_back({make_ellipse(), "Ellipse"});

        // 2. Tricky Curves
        for (size_t i = 0; i < std::size(kTrickyCurves); ++i) {
            shapes.push_back({make_tricky_curve(kTrickyCurves[i]), "Tricky Curve"});
        }

        // 3. Open Paths & self-intersecting
        shapes.push_back({SkPathBuilder().moveTo(85, 100).cubicTo(90, 80, 110, 120, 115, 100).detach(), "Open Cubic"});

        shapes.push_back({SkPathBuilder()
                          .moveTo(85, 85)
                          .lineTo(115, 115)
                          .lineTo(115, 85)
                          .lineTo(85, 115)
                          .close()
                          .detach(), "Self-Intersecting"});

        const SkScalar kCellSize = 200.0f;
        const SkScalar kPadding = 20.0f;

        // Outset widths: pathologically large (100), medium (40), tiny (6)
        struct OutsetStyle {
            float fWidth;
            SkColor fStrokeColor;
            SkColor fFillColor;
        };
        const OutsetStyle kOutsets[] = {
            {100.0f, 0xFFDD6B20, 0x22DD6B20}, // Outset = 50. Translucent Orange
            {40.0f,  0xFF38A169, 0x4438A169}, // Outset = 20. Translucent Emerald Green
            {6.0f,   0xFF3182CE, 0x663182CE}  // Outset = 3. Translucent Blue
        };

        SkPaint paint;
        paint.setAntiAlias(true);

        for (size_t i = 0; i < shapes.size(); ++i) {
            const SkPath& path = shapes[i].fPath;

            for (int drawFilled = 0; drawFilled <= 1; ++drawFilled) {
                int shapeIndexInRow = i % 3;
                int row = i / 3;
                int col = shapeIndexInRow * 2 + drawFilled;

                SkAutoCanvasRestore acr(canvas, true);
                canvas->translate(kPadding + col * kCellSize, kPadding + row * kCellSize);

                if (drawFilled) {
                    // --- DRAW FILLED LAYER BY LAYER (Largest to Smallest) ---

                    // 1. Largest Outset (width = 100)
                    {
                        SkStroke stroker;
                        stroker.setWidth(kOutsets[0].fWidth);
                        stroker.setJoin(SkPaint::kRound_Join);
                        stroker.setCap(SkPaint::kRound_Cap);

                        SkPathBuilder outsetBuilder;
                        stroker.outsetPath(path, &outsetBuilder);
                        SkPath outsetPath = outsetBuilder.detach();

                        paint.setStyle(SkPaint::kFill_Style);
                        paint.setColor(kOutsets[0].fFillColor);
                        canvas->drawPath(outsetPath, paint);
                    }

                    // 2. Medium Outset (width = 40)
                    {
                        SkStroke stroker;
                        stroker.setWidth(kOutsets[1].fWidth);
                        stroker.setJoin(SkPaint::kRound_Join);
                        stroker.setCap(SkPaint::kRound_Cap);

                        SkPathBuilder outsetBuilder;
                        stroker.outsetPath(path, &outsetBuilder);
                        SkPath outsetPath = outsetBuilder.detach();

                        paint.setStyle(SkPaint::kFill_Style);
                        paint.setColor(kOutsets[1].fFillColor);
                        canvas->drawPath(outsetPath, paint);
                    }

                    // 3. Tiny Outset (width = 6)
                    {
                        SkStroke stroker;
                        stroker.setWidth(kOutsets[2].fWidth);
                        stroker.setJoin(SkPaint::kRound_Join);
                        stroker.setCap(SkPaint::kRound_Cap);

                        SkPathBuilder outsetBuilder;
                        stroker.outsetPath(path, &outsetBuilder);
                        SkPath outsetPath = outsetBuilder.detach();

                        paint.setStyle(SkPaint::kFill_Style);
                        paint.setColor(kOutsets[2].fFillColor);
                        canvas->drawPath(outsetPath, paint);
                    }

                    // 4. Original Path
                    paint.setStyle(SkPaint::kFill_Style);
                    paint.setColor(0xAA4A5568); // Translucent grey fill for original
                    canvas->drawPath(path, paint);

                } else {
                    // --- DRAW STROKED LAYERS ---

                    // 1. Outset layers
                    for (const auto& outset : kOutsets) {
                        SkStroke stroker;
                        stroker.setWidth(outset.fWidth);
                        stroker.setJoin(SkPaint::kRound_Join);
                        stroker.setCap(SkPaint::kRound_Cap);

                        SkPathBuilder outsetBuilder;
                        stroker.outsetPath(path, &outsetBuilder);
                        SkPath outsetPath = outsetBuilder.detach();

                        paint.setStyle(SkPaint::kStroke_Style);
                        paint.setStrokeWidth(1.5f);
                        paint.setColor(outset.fStrokeColor);
                        canvas->drawPath(outsetPath, paint);
                    }

                    // 2. Original Path (Bright crimson stroke)
                    paint.setStyle(SkPaint::kStroke_Style);
                    paint.setStrokeWidth(1.5f);
                    paint.setColor(0xFFE53E3E); // Bright crimson red
                    canvas->drawPath(path, paint);
                }
            }
        }
    }

private:
    using INHERITED = GM;
};

DEF_GM(return new StrokeOutsetGM;)

} // namespace skiagm
