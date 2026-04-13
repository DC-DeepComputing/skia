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

class GraphiteClipFringeGM : public skiagm::GM {
public:
    GraphiteClipFringeGM() {
        this->setBGColor(0xFFFFFFFF); // White background to make the fringe visible
    }

protected:
    SkString getName() const override { return SkString("graphite_clip_fringe"); }

    SkISize getISize() override { return SkISize::Make(1024, 500); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        // 1. Create the shallow diagonal clip path using SkPathBuilder
        SkPathBuilder builder;
        builder.moveTo(0, 10)
               .lineTo(1024, 60)  // Shallow diagonal edge
               .lineTo(1024, 100)
               .lineTo(0, 50)
               .close();

        // Finalize the builder to get our immutable SkPath
        SkPath clipPath = builder.detach();

        SkPaint layer1Paint;
        layer1Paint.setColor(SK_ColorBLUE);
        layer1Paint.setAntiAlias(true);

        SkPaint layer2Paint;
        layer2Paint.setColor(SK_ColorRED);
        layer2Paint.setAntiAlias(true);

        // 2. Bottom Layer (Simulating first absolutely positioned element)
        canvas->save();
        // Simulate CSS transform: translateX(10px)
        canvas->translate(10, 0);
        // Simulate CSS clip-path with Anti-Aliasing enabled (the `true` flag)
        canvas->clipPath(clipPath, SkClipOp::kIntersect, true);
        canvas->drawRect(SkRect::MakeWH(1024, 500), layer1Paint);
        canvas->restore();

        // 3. Top Layer (Simulating the overlapping reveal element)
        canvas->save();
        // Simulate CSS transform: translateX(20px)
        // This overlap forces the AA edges to blend incorrectly in Graphite
        canvas->translate(20, 0);
        canvas->clipPath(clipPath, SkClipOp::kIntersect, true);
        canvas->drawRect(SkRect::MakeWH(1024, 500), layer2Paint);
        canvas->restore();

        return DrawResult::kOk;
    }
};

DEF_GM(return new GraphiteClipFringeGM;)
