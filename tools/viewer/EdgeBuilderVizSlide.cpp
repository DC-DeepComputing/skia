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
#include "tools/viewer/ClickHandlerSlide.h"
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

class EdgeBuilderSlide : public ClickHandlerSlide  {
public:
    EdgeBuilderSlide() { fName = "EdgeBuilderViz"; }
    void draw(SkCanvas* canvas) override {
        canvas->scale(kScale, kScale);
        canvas->clear(SK_ColorWHITE);

        constexpr int kMaskWidth = 15;
        constexpr int kMaskHeight = 25;
        // SkIRect bounds{0, 0, 200, 200};
        // SkRRect rrect = SkRRect::MakeRectXY({0, 0, kMaskWidth, kMaskHeight}, 5, 3);
        // SkPath path = SkPath::RRect(rrect);

        SkPathBuilder pb;
        SkPath path = pb.moveTo(0, 0)
                        .quadTo({fControlX, fControlY}, {10, 20})
                        .lineTo({10, 10})
                        .lineTo({12, 0})
                        .close().detach();

        // path.dump();

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
            truthPaint.setStrokeWidth(2.f/kScale);

            canvas->drawPath(path, truthPaint);
        }

        {
            SkBasicEdgeBuilder builder;
            int num = builder.buildEdges(path, nullptr);
            SkEdge** edgeList = builder.edgeList();

            SkPaint edgePaint;
            edgePaint.setColor(SkColorSetRGB(0xFF, 0x8C, 0x00));
            edgePaint.setStyle(SkPaint::Style::kStroke_Style);
            edgePaint.setStrokeWidth(4.f/kScale);

            SkDebugf("\n");
            for (int i = 0; i < num; ++i) {
                SkEdge* e = edgeList[i];
                while (true) {
                    e->dump();
                    float x1 = SkFixedToFloat(e->fX);
                    float y1 = e->fFirstY;
                    float y2 = e->fLastY;
                    float x2 = x1 + SkFixedToFloat(e->fDxDy) * (y2 - y1);
                    if (x1 == x2 && y1 == y2) {
                        x2 += 0.2f; // Make "zero height" edges visible.
                    }
                    // The y coordinates are implied to be at the half pixel values for y
                    canvas->drawLine(x1, y1 + 0.5, x2, y2 + 0.5, edgePaint);
                    if (!e->hasNextSegment()) {
                        break;
                    }
                    e->fX = x2;
                    if (!e->nextSegment()) {
                        break;
                    }
                }
            }
        }

        {
            SkPaint controlPaint;
            controlPaint.setStyle(SkPaint::Style::kStroke_Style);
            controlPaint.setColor(SK_ColorBLUE);
            controlPaint.setAntiAlias(true);
            controlPaint.setStrokeWidth(1.f/kScale);

            canvas->drawCircle(fControlX, fControlY, 4.f/kScale, controlPaint);
        }
    }
private:
    static constexpr size_t kScale = 32;
    class Click : public ClickHandlerSlide::Click {
    public:
        Click(float* x, float* y) : fX(x), fY(y) {}

        void doClick(EdgeBuilderSlide* that) {
            *fX = fCurr.fX / kScale;
            *fY = fCurr.fY / kScale;
        }

    private:
        float* fX;
        float* fY;
    };

    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modifiers) override {
        if (modifiers != skui::ModifierKey::kControl) {
            return nullptr;
        }
        return new Click(&fControlX, &fControlY);
    }

    bool onClick(ClickHandlerSlide::Click* click) override {
        Click* myClick = (Click*)click;
        myClick->doClick(this);
        return true;
    }
    float fControlX = 2.5f, fControlY = 15.f;

};
DEF_SLIDE( return new EdgeBuilderSlide(); )
