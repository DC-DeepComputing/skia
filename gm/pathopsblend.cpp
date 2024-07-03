/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/pathops/SkPathOps.h"
#include "src/core/SkStroke.h"
#include "tools/ToolUtils.h"

static SkPath cross() {
    SkPath path;
    path.addRect(15, 0, 35, 50);
    path.addRect(0, 15, 50, 35);
    return path;
}

static SkPath circle() { return SkPath::Circle(25, 25, 20); }

// We implement every op except ReverseDifference: That one can be handled by swapping the paths
// and using the Difference logic.
static constexpr SkPathOp kOps[] = {
        kDifference_SkPathOp,
        kIntersect_SkPathOp,
        kUnion_SkPathOp,
        kXOR_SkPathOp,
};

struct OpAsBlend {
    SkBlendMode fMode;
    bool fAsLayer = false;
};

static OpAsBlend op_blend_mode(SkPathOp op) {
    switch (op) {
        case kDifference_SkPathOp:
            return {SkBlendMode::kClear};
        case kIntersect_SkPathOp:
            return {SkBlendMode::kModulate, true};
        case kUnion_SkPathOp:
            return {SkBlendMode::kPlus};
        case kXOR_SkPathOp:
            return {SkBlendMode::kXor, true};
        default:
            // Other modes not implemented
            return {SkBlendMode::kSrcOver};
    }
}

DEF_SIMPLE_GM(pathops_blend, canvas, 130, 60 * std::size(kOps) + 60 + 10) {
    // Checkerboard background to demonstrate that we're only covering the pixels we want:
    ToolUtils::draw_checkerboard(canvas);

    // Two paths that overlap in interesting ways:
    SkPath p1 = cross();
    SkPath p2 = circle();
    SkPaint paint;
    paint.setAntiAlias(true);

    canvas->translate(10, 10);

    // Draw the two paths by themselves:
    {
        canvas->save();
        canvas->drawPath(p1, paint);
        canvas->translate(60, 0);
        canvas->drawPath(p2, paint);
        canvas->restore();
        canvas->translate(0, 60);
    }

    for (SkPathOp op : kOps) {
        canvas->save();

        // Use PathOps to compute new path, then draw it:
        {
            SkPath opPath;
            Op(p1, p2, op, &opPath);
            canvas->drawPath(opPath, paint);
        }

        canvas->translate(60, 0);

        // Do raster version of op
        {
            auto blend = op_blend_mode(op);
            // Create a first layer. We will use blending to build a mask of the shape we want here
            canvas->saveLayer(SkRect::MakeWH(50, 50), nullptr);

            // We reuse this paint to apply various blend modes:
            SkPaint p;
            p.setAntiAlias(true);

            // We always draw the first shape to the first layer, using src mode. This fills the
            // layer with a mask of that path:
            p.setBlendMode(SkBlendMode::kSrc);
            canvas->drawPath(p1, p);

            // We use a specific blend mode for each op, applied to the second path
            p.setBlendMode(blend.fMode);

            if (blend.fAsLayer) {
                // Some ops need a second layer. For those, we apply the blend mode to the restore
                canvas->saveLayer(SkRect::MakeWH(50, 50), &p);

                // And just draw the path into the second layer in src mode
                p.setBlendMode(SkBlendMode::kSrc);
                canvas->drawPath(p2, p);

                // When we restore, the blend mode passed to saveLayer does the operation we want
                canvas->restore();
            } else {
                // Other ops need to be done in the first layer. Apply the blend mode to the path:
                canvas->drawPath(p2, p);
            }

            // In either case, the layer's alpha channel now contains a mask of the desired shape.
            // Cover the entire rectangle with whatever paint we ACTUALLY want to draw (eg, blue),
            // using kSrcIn. This will only draw where the mask was present:
            p.setBlendMode(SkBlendMode::kSrcIn);
            p.setColor(SK_ColorBLUE);
            canvas->drawPaint(p);
            canvas->restore();
        }

        canvas->restore();
        canvas->translate(0, 60);
    }
}
