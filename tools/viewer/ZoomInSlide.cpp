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

/*
*/
class ZoomInSlide : public ClickHandlerSlide {
public:
    ZoomInSlide() { fName = "ZoomIn"; }

    void draw(SkCanvas* canvas) override {
        canvas->scale(kScale, kScale);
        canvas->clear(SK_ColorWHITE);

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeCap(SkPaint::kSquare_Cap);
        paint.setStrokeJoin(SkPaint::kMiter_Join);
        paint.setAntiAlias(true);

        SkPath path = SkPathBuilder()
            .moveTo(1.5, 1.5)
            .lineTo(10.5, 1.5)
            .lineTo(10.5, 10.5)
            .lineTo(1.5, 10.5)
            .close()
            .detach();

        drawScaledPath(canvas, path, paint);
        drawGrid(canvas);
        drawHighRezOverlay(canvas, path);
    }

    // Draw path at normal size (typically very small) and then draw it into the scaled canvas.
    void drawScaledPath(SkCanvas* canvas, const SkPath& path, const SkPaint& pathPaint) {
        uint8_t buffer[kMaskWidth * kMaskHeight * 4];
        auto ii = SkImageInfo::MakeN32Premul(kMaskWidth, kMaskHeight);
        SkPixmap pm1(ii, buffer, ii.minRowBytes());
        auto surface = SkSurfaces::WrapPixels(pm1);
        surface->getCanvas()->clear(SK_ColorTRANSPARENT);
        surface->getCanvas()->drawPath(path, pathPaint);
        auto pathImg = surface->makeImageSnapshot();

        // Remember kScale applies to this canvas.
        canvas->drawImage(pathImg, 0, 0);
    }

    void drawGrid(SkCanvas* canvas) {
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

    void drawHighRezOverlay(SkCanvas* canvas, const SkPath& path) {
        SkPaint truthPaint;
        truthPaint.setStyle(SkPaint::Style::kStroke_Style);
        truthPaint.setColor(SK_ColorRED);
        truthPaint.setAntiAlias(true);
        truthPaint.setStrokeWidth(2.f / kScale);

        canvas->drawPath(path, truthPaint);
    }

private:
    static constexpr size_t kScale = 32;
    // Defines the region of the screen that will be zoomed in on.
    // The path under scrutiny will be drawn in this region.
    static constexpr int kMaskWidth = 20;
    static constexpr int kMaskHeight = 30;

    class Click : public ClickHandlerSlide::Click {
    public:
        Click(float* x, float* y) : fX(x), fY(y) {}

        void doClick(ZoomInSlide* that) {
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
        return new Click(&fUnusedX, &fUnusedY);
    }

    bool onClick(ClickHandlerSlide::Click* click) override {
        Click* myClick = (Click*)click;
        myClick->doClick(this);
        return true;
    }
    float fUnusedX = 2.f, fUnusedY = 13.f;
};
DEF_SLIDE(return new ZoomInSlide();)
