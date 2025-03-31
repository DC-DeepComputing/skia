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
#include "include/core/SkPixmap.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkDebugUtils.h"
#include "src/core/SkEdge.h"
#include "src/core/SkEdgeBuilder.h"
#include "tools/viewer/Slide.h"

#include <vector>

// static bool compare_edges(const SkEdge* a, const SkEdge* b) {
//     if (a->fFirstY != b->fFirstY) {
//         return a->fFirstY < b->fFirstY;
//     }

//     return a->fX < b->fX;
// }

// static SkEdge* sort_edges(SkEdge* list[], int count, SkEdge** last) {
//     SkTQSort(list, list + count, compare_edges);

//     // now make the edges linked in sorted order
//     for (int i = 1; i < count; i++) {
//         list[i - 1]->fNext = list[i];
//         list[i]->fPrev = list[i - 1];
//     }

//     *last = list[count - 1];
//     return list[0];
// }

class EdgeBuilderSlide : public Slide  {
public:
    EdgeBuilderSlide() { fName = "EdgeBuilderViz"; }
    void draw(SkCanvas* canvas) override {
        constexpr size_t kScale = 64;
        canvas->scale(kScale, kScale);
        canvas->clear(SK_ColorWHITE);

        constexpr int kMaskWidth = 20;
        constexpr int kMaskHeight = 10;
        SkIRect bounds{0, 0, 200, 200};
        SkRRect rrect = SkRRect::MakeRectXY({0, 0, kMaskWidth, kMaskHeight}, 5, 3);
        SkPath path = SkPath::RRect(rrect);
        path.dump();

        {
            uint8_t buffer[kMaskWidth*kMaskHeight*4];
            auto ii = SkImageInfo::MakeN32Premul(kMaskWidth, kMaskHeight);
            SkPixmap pm1(ii, buffer, ii.minRowBytes());
            auto surface = SkSurfaces::WrapPixels(pm1);
            SkPaint pathPaint;
            pathPaint.setStyle(SkPaint::Style::kFill_Style);
            pathPaint.setColor(SK_ColorBLACK);
            surface->getCanvas()->clear(SK_ColorTRANSPARENT);
            surface->getCanvas()->drawPath(path, pathPaint);
            auto pathImg = surface->makeImageSnapshot();

            canvas->drawImage(pathImg, 0, 0);
        }

        {
            SkPaint gridPaint;
            gridPaint.setColor(SK_ColorDKGRAY);
            gridPaint.setStyle(SkPaint::Style::kStroke_Style);
            gridPaint.setStrokeWidth(0);
            for (int y = 0; y <= kMaskHeight; y++) {
                canvas->drawLine(0, y, kMaskWidth, y, gridPaint);
            }
            for (int x = 0; x <= kMaskWidth; x++) {
                canvas->drawLine(x, 0, x, kMaskHeight, gridPaint);
            }
        }


        {
            SkPaint truthPaint;
            truthPaint.setStyle(SkPaint::Style::kStroke_Style);
            truthPaint.setColor(SK_ColorRED);
            truthPaint.setAntiAlias(true);
            truthPaint.setStrokeWidth(1.f/kScale);

            canvas->drawPath(path, truthPaint);
        }

        // SkPaint segmentPaint;
        // segmentPaint.setStyle(SkPaint::Style::kStroke_Style);
        // segmentPaint.setStrokeWidth(1);
        // int color = -1;
        // const SkColor kSegmentColors[] = {SK_ColorRED, SK_ColorBLUE};

        // SkPath::Iter iter(path, false);
        // SkPoint points[4];
        // for (SkPath::Verb verb = iter.next(points); verb != SkPath::kDone_Verb; verb = iter.next(points)) {
        //     segmentPaint.setColor(kSegmentColors[++color % std::size(kSegmentColors)]);
        //     switch (verb) {
        //         case SkPath::kMove_Verb:
        //             color--;
        //             break;
        //         case SkPath::kLine_Verb: {
        //             SkPathBuilder p;
        //             p.moveTo(points[0]).lineTo(points[1]);
        //             canvas->drawPath(p.detach(), segmentPaint);
        //             break;
        //         }
        //         case SkPath::kQuad_Verb: {
        //             SkPathBuilder p;
        //             p.moveTo(points[0]).quadTo(points[1], points[2]);
        //             canvas->drawPath(p.detach(), segmentPaint);
        //             break;
        //         }
        //         case SkPath::kConic_Verb: {
        //             SkPathBuilder p;
        //             p.moveTo(points[0]).conicTo(points[1], points[2], iter.conicWeight());
        //             canvas->drawPath(p.detach(), segmentPaint);
        //             break;
        //         }
        //         case SkPath::kClose_Verb:
        //             break;
        //         default:
        //             SK_ABORT("FIXME KJLUBICK %d", (int)verb);
        //     }
        // }

        {
            SkBasicEdgeBuilder builder;
            int num = builder.buildEdges(path, nullptr);
            SkEdge** edgeList = builder.edgeList();

            SkPaint edgePaint;
            edgePaint.setColor(SkColorSetRGB(0xFF, 0x8C, 0x00));
            edgePaint.setStyle(SkPaint::Style::kFill_Style);

            SkDebugf("\n");
            for (int i = 0; i < num; ++i) {
                SkEdge* e = edgeList[i];
                e->dump();
                float x1 = SkFixedToFloat(e->fX);
                float y1 = e->fFirstY;
                float y2 = e->fLastY;
                float x2 = x1 + SkFixedToFloat(e->fDX) * (y2 - y1);
                SkDebugf("%f,%f -> %f,%f\n", x1, y1, x2, y2);
                canvas->drawCircle(x1, y1, 4.0/kScale, edgePaint);
            }
        }
    }
private:

};
DEF_SLIDE( return new EdgeBuilderSlide(); )
