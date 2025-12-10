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
    ZoomInSlide(size_t scale, size_t width, size_t height, SkString name)
            : fScale(scale), fWidth(width), fHeight(height) {
        fName = name;
    }

    void draw(SkCanvas* canvas) override {
        canvas->scale(fScale, fScale);
        canvas->clear(SK_ColorWHITE);

        this->drawUnderGrid(canvas);
        this->drawGrid(canvas);
        this->drawOverGrid(canvas);
    }

    virtual void drawUnderGrid(SkCanvas* canvas) = 0;

    // Draw path at normal size (typically very small) and then draw it into the scaled canvas.
    void drawScaledPath(SkCanvas* canvas, const SkPath& path, const SkPaint& pathPaint) {
        constexpr size_t kBufferSize = 10024;
        SkASSERT_RELEASE(fWidth * fHeight * 4 <= kBufferSize);
        uint8_t buffer[kBufferSize];
        auto ii = SkImageInfo::MakeN32Premul(fWidth, fHeight);
        SkPixmap pm1(ii, buffer, ii.minRowBytes());
        auto surface = SkSurfaces::WrapPixels(pm1);
        surface->getCanvas()->clear(SK_ColorTRANSPARENT);
        surface->getCanvas()->drawPath(path, pathPaint);
        auto pathImg = surface->makeImageSnapshot();

        // Remember fScale applies to this canvas.
        canvas->drawImage(pathImg, 0, 0);
    }

    void drawGrid(SkCanvas* canvas) {
        SkPaint gridPaint;
        gridPaint.setColor(SK_ColorDKGRAY);
        gridPaint.setStyle(SkPaint::Style::kStroke_Style);
        gridPaint.setStrokeWidth(0);
        for (int y = 0; y <= (int)fHeight; y++) {
            canvas->drawLine(0, y, fWidth, y, gridPaint);
        }
        for (int x = 0; x <= (int)fWidth; x++) {
            canvas->drawLine(x, 0, x, fHeight, gridPaint);
        }
    }

    virtual void drawOverGrid(SkCanvas* canvas) = 0;

protected:
    const size_t fScale;
    const size_t fWidth;
    const size_t fHeight;

    class ScaledClick : public ClickHandlerSlide::Click {
    public:
        ScaledClick(size_t scale) : fScale(scale) {}

        SkPoint currScaled() const { return {fCurr.fX / fScale, fCurr.fY / fScale}; }

    private:
        const size_t fScale;
    };

    virtual void handleClick(const ScaledClick*) {}

private:
    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modifiers) override {
        if (modifiers != skui::ModifierKey::kControl) {
            return nullptr;
        }
        return new ScaledClick(fScale);
    }

    bool onClick(ClickHandlerSlide::Click* click) override {
        auto myClick = static_cast<ScaledClick*>(click);
        this->handleClick(myClick);
        return true;
    }
};
