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
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"

class CrBug495458817GM : public skiagm::GM {
public:
    CrBug495458817GM() {
        this->setBGColor(0xFFFFFFFF); // White background to make the fringe visible
    }

protected:
    SkString getName() const override { return SkString("crbug_495458817"); }

    // Increased the canvas height slightly to 600 to comfortably fit the third row
    SkISize getISize() override { return SkISize::Make(1024, 600); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        // 1. Create a razor-thin, extremely skewed parallelogram.
        SkPathBuilder builder;
        builder.moveTo(0, 400)       // Top-Left
               .lineTo(800, 100)     // Top-Right
               .lineTo(855, 110)     // Bottom-Right (Shifted +55x, +10y to keep it parallel)
               .lineTo(55, 410)      // Bottom-Left (Shifted +55x, +10y)
               .close();

        // Finalize the builder to get our immutable SkPath
        SkPath clipPath = builder.detach();

        // Array of colors to demonstrate multiple nested overlapping layers
        const SkColor layerColors[] = {
            SK_ColorBLUE,
            SK_ColorGREEN,
            SK_ColorYELLOW,
            SK_ColorCYAN,
            SK_ColorRED,
            SK_ColorMAGENTA
        };
        const int numLayers = 6;

        // 2. FIRST SET: Just translated on the X-axis (Creates horizontal striations)
        for (int i = 0; i < numLayers; ++i) {
            SkPaint paint;
            paint.setColor(layerColors[i]);
            paint.setAntiAlias(true);

            canvas->save();
            canvas->translate(10.0f * (i + 1), 0);

            canvas->clipPath(clipPath, SkClipOp::kDifference, true);
            canvas->drawRect(SkRect::MakeWH(1024, 500), paint);

            canvas->restore();
        }

        // 3. SECOND SET: Perfectly nested/layered along the left edge.
        // Shift the entire second set down by 80px so it sits visually beneath the first set
        canvas->save();
        canvas->translate(0, 80);

        for (int i = 0; i < numLayers; ++i) {
            SkPaint paint;
            paint.setColor(layerColors[i]);
            paint.setAntiAlias(true);

            canvas->save();

            // Translate exactly parallel to the left edge's slope.
            // The edge goes +55 X and +10 Y.
            // 55 / 5 = 11. 10 / 5 = 2.
            // Translating by (11, 2) steps ensures perfect edge-to-edge nesting.
            canvas->translate(11.0f * (i + 1), 2.0f * (i + 1));

            canvas->clipPath(clipPath, SkClipOp::kDifference, true);
            canvas->drawRect(SkRect::MakeWH(1024, 500), paint);

            canvas->restore();
        }

        canvas->restore(); // Restore the global Y offset for the second set

        // 4. THIRD SET: Perfectly nested/layered "Up and Right" along the top edge.
        // Shift this set down by 160px so it sits visually beneath the second set.
        canvas->save();
        canvas->translate(0, 190);

        for (int i = 0; i < numLayers; ++i) {
            SkPaint paint;
            paint.setColor(layerColors[i]);
            paint.setAntiAlias(true);

            canvas->save();

            // Translate exactly parallel to the top edge's slope (Up and Right).
            // Top edge goes +800 X and -300 Y (ratio of 8 to -3).
            // Translating by (16, -6) steps ensures flawless nesting.
            canvas->translate(16.0f * (i + 1), -6.0f * (i + 1));

            canvas->clipPath(clipPath, SkClipOp::kDifference, true);
            canvas->drawRect(SkRect::MakeWH(1024, 500), paint);

            canvas->restore();
        }

        canvas->restore(); // Restore the global Y offset for the third set

        return DrawResult::kOk;
    }
};

DEF_GM(return new CrBug495458817GM;)
