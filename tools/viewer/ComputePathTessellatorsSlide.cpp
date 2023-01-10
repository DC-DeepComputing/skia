/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if SK_SUPPORT_GPU && defined(SK_GRAPHITE_ENABLED)

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/graphite/Device.h"
#include "tools/viewer/ClickHandlerSlide.h"

namespace {

constexpr size_t kNumPaths = 9;

const char* kPathNames[kNumPaths] = {
    "1 quadratic Bezier",
    "1 cubic Bezier",
    "4 lines",
    "2 quads + 1 cubic",
    "800 cubics",
    "20,000 cubics",
    "80,000 cubics",
    "204,000 cubics",
    "500,000 cubics",
};

static void build_long_path(float scale, SkPath* p) {
    float offsetX = 50;
    float offsetY = 60;
    int rows = 40 * scale;
    int columns = 10 * scale;
    p->moveTo(offsetX, offsetY);
    for (int c = 0; c < columns; c++) {
        for (int i = 0; i < rows; i++) {
            offsetY += 20 / scale;
            p->cubicTo(offsetX - 40 / scale, offsetY + 30 / scale,
                       offsetX - 40 / scale, offsetY - 50 / scale,
                       offsetX, offsetY);
        }
        p->quadTo(offsetX + 10 / scale, offsetY + 20 / scale, offsetX + 20 / scale, offsetY);
        offsetX += 60 / scale;
        for (int i = 0; i < rows; i++) {
            offsetY -= 20 / scale;
            p->cubicTo(offsetX, offsetY - 30 / scale,
                       offsetX, offsetY + 50 / scale,
                       offsetX - 40 / scale, offsetY);
        }
        offsetX += 50 / scale;
        p->lineTo(offsetX, offsetY);
    }
}

class ComputePathTessellatorsSlide final : public ClickHandlerSlide {
public:
    ComputePathTessellatorsSlide() {
        fPaths[0].moveTo(100, 100).quadTo(500, 500, 1000, 100);
        fPaths[1].moveTo(100, 100).cubicTo(100, 300, 1000, 300, 1000, 100);
        fPaths[2].moveTo(100, 500).lineTo(500, 900).lineTo(900, 500).lineTo(500, 100);
        fPaths[3]
                .moveTo(500, 100)
                .quadTo(100, 600, 490, 500)
                .cubicTo(100, 800, 900, 800, 510, 500)
                .quadTo(900, 600, 500, 100);

        // A large path (800 verbs) that should not use wedges.
        build_long_path(/*scale=*/1, &fPaths[4]);  //     800 verbs
        build_long_path(/*scale=*/5, &fPaths[5]);  //  20,000 verbs
        build_long_path(/*scale=*/10, &fPaths[6]); //  80,000 verbs
        build_long_path(/*scale=*/16, &fPaths[7]); // 204,000 verbs
        build_long_path(/*scale=*/25, &fPaths[8]); // 500,000 verbs

        this->selectPath(0);

        fName = "PathTessellators_Compute";
    }

private:
    class Click : public ClickHandlerSlide::Click {
    public:
        Click(int ptIdx) : fPtIdx(ptIdx) {}

        void doClick(SkPath* path) {
            SkPoint pt = path->getPoint(fPtIdx);
            SkPathPriv::UpdatePathPoint(path, fPtIdx, pt + fCurr - fPrev);
        }

    private:
        int fPtIdx;
    };

    void draw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        skgpu::graphite::Device* device = SkCanvasPriv::TopDevice(canvas)->asGraphiteDevice();
        if (!device) {
            canvas->clear(SK_ColorRED);
            SkFont font(nullptr, 20);
            SkPaint captionPaint;
            captionPaint.setColor(SK_ColorWHITE);
            canvas->drawString("Graphite only", 10, 30, font, captionPaint);
            return;
        }

        canvas->clear(SK_ColorWHITE);

        // Draw the path.
        SkPaint paint;
        paint.setStrokeWidth(20);
        paint.setStroke(fStroke);
        paint.setColor(fUseCpu ? SK_ColorRED : SK_ColorGREEN);
        if (fUseCpu) {
            device->drawPath(*fPath, paint);
        } else {
            device->drawPathUsingGpuComputeTessellation(*fPath, paint);
        }

        if (fDrawControlPoints) {
            // Draw the path points.
            SkPaint pointsPaint;
            pointsPaint.setColor(SK_ColorBLUE);
            pointsPaint.setStrokeWidth(8);
            canvas->drawPoints(SkCanvas::kPoints_PointMode,
                               fPath->countPoints(),
                               SkPathPriv::PointData(*fPath),
                               pointsPaint);
        }

        // Draw caption
        SkString caption = SkStringPrintf(
                "Current path: %s        "
                "t: toggle GPU Compute vs CPU tessellation   "
                "q: toggle fill/stroke   "
                "c: show/hide control points   "
                "1-9: change path",
                kPathNames[fPathIdx]);

        SkFont font(nullptr, 16);
        SkPaint captionPaint;
        captionPaint.setColor(SK_ColorBLACK);
        canvas->drawString(caption, 10, 20, font, captionPaint);
    }

    bool onChar(SkUnichar c) override {
        switch (c) {
            case 't':
                fUseCpu = !fUseCpu;
                return true;
            case 'q':
                fStroke = !fStroke;
                return true;
            case 'c':
                fDrawControlPoints = !fDrawControlPoints;
                return true;
            case '1':
            default: {
                size_t idx = static_cast<size_t>(c - '1');
                if (idx >= kNumPaths) {
                    return false;
                }
                this->selectPath(idx);
                return true;
            }
        }
        return false;
    }

    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) override {
        const SkPoint* pts = SkPathPriv::PointData(*fPath);
        float fuzz = 30;
        for (int i = 0; i < fPath->countPoints(); ++i) {
            if (fabs(x - pts[i].x()) < fuzz && fabsf(y - pts[i].y()) < fuzz) {
                return new Click(i);
            }
        }
        return nullptr;
    }

    bool onClick(ClickHandlerSlide::Click* clickBase) override {
        Click* click = static_cast<Click*>(clickBase);
        click->doClick(fPath);
        return true;
    }

    void selectPath(int index) {
        fPathIdx = index;
        fPath = &fPaths[index];
    }

    bool fUseCpu = false;
    bool fStroke = false;
    bool fDrawControlPoints = false;

    SkPath* fPath;
    int fPathIdx;
    SkPath fPaths[kNumPaths];
};

}  // namespace

DEF_SLIDE( return new ComputePathTessellatorsSlide; )

#endif  // SK_SUPPORT_GPU && defined(SK_GRAPHITE_ENABLED)
