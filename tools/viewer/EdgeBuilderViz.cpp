/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkEdge.h"
#include "src/core/SkEdgeBuilder.h"
#include "tools/viewer/Slide.h"

#include <vector>

class EdgeBuilderSlide : public Slide  {
public:
    EdgeBuilderSlide() { fName = "EdgeBuilderViz"; }
    void draw(SkCanvas* canvas) override {
        canvas->scale(4, 4);
        canvas->clear(SK_ColorWHITE);

        SkPaint pathPaint;
        pathPaint.setStyle(SkPaint::Style::kFill_Style);

        SkIRect bounds{0, 0, 200, 200};
        SkRRect rrect = SkRRect::MakeRectXY({40, 20, 100, 50}, 5, 5);
        SkPath path = SkPath::RRect(rrect);

        canvas->drawPath(path, pathPaint);

        SkPaint segmentPaint;
        segmentPaint.setStyle(SkPaint::Style::kStroke_Style);
        segmentPaint.setStrokeWidth(3);
        int color = -1;
        const SkColor kSegmentColors[] = {SK_ColorRED, SK_ColorBLUE};

        SkPath::Iter iter(path, false);
        SkPoint points[4];
        for (SkPath::Verb verb = iter.next(points); verb != SkPath::kDone_Verb; verb = iter.next(points)) {
            segmentPaint.setColor(kSegmentColors[++color % std::size(kSegmentColors)]);
            switch (verb) {
                case SkPath::kMove_Verb:
                    color--;
                    break;
                case SkPath::kLine_Verb: {
                    SkPathBuilder p;
                    p.moveTo(points[0]).lineTo(points[1]);
                    canvas->drawPath(p.detach(), segmentPaint);
                    break;
                }
                case SkPath::kQuad_Verb: {
                    SkPathBuilder p;
                    p.moveTo(points[0]).quadTo(points[1], points[2]);
                    canvas->drawPath(p.detach(), segmentPaint);
                    break;
                }
                case SkPath::kConic_Verb: {
                    SkPathBuilder p;
                    p.moveTo(points[0]).conicTo(points[1], points[2], iter.conicWeight());
                    canvas->drawPath(p.detach(), segmentPaint);
                    break;
                }
            }
        }

        SkBasicEdgeBuilder builder;
        int num = builder.buildEdges(path, &bounds);
        SkEdge** edgeList = builder.edgeList();

        SkPaint edgePaint;
        edgePaint.setColor(SK_ColorDKGRAY);
        edgePaint.setStyle(SkPaint::Style::kStroke_Style);
        edgePaint.setStrokeWidth(1);

        for (int i = 0; i < num; ++i) {
            edgeList[i]->dump();
        }




    }
private:

};
DEF_SLIDE( return new EdgeBuilderSlide(); )
