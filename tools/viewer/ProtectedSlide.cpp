/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/effects/SkImageFilters.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tools/gpu/ProtectedUtils.h"
#include "tools/viewer/Slide.h"


#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkCanvasPriv.h"

static sk_sp<SkShader> make_shader(SkScalar cx, SkScalar cy, SkScalar rad) {
    const SkColor colors[] = {
            SK_ColorRED, SK_ColorRED, SK_ColorBLUE, SK_ColorBLUE, SK_ColorGREEN, SK_ColorGREEN,
            SK_ColorRED, SK_ColorRED, SK_ColorBLUE, SK_ColorBLUE, SK_ColorGREEN, SK_ColorGREEN,
    };
    constexpr int count = std::size(colors);
    SkScalar pos[count] = { 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6 };
    for (int i = 0; i < count; ++i) {
        pos[i] *= 1.0f/6;
    }
    return SkGradientShader::MakeSweep(cx, cy, colors, pos, count);
}

static void do_draw(SkCanvas* canvas, SkScalar scaleFactor, SkImage* img) {
    SkAutoCanvasRestore acr(canvas, true);

    const SkScalar cx = 128, cy = 128, rad = 100;
    SkPaint p;
    p.setShader(make_shader(cx, cy, rad));
    p.setAntiAlias(true);
    canvas->drawCircle(cx, cy, rad, p);

    canvas->drawImage(img, cx-32, cy-32);

    // now setup a saveLayer that will pull in the backdrop and blur it
    const SkScalar sigma = 10;

    // Using kClamp because kDecal, the default, produces transparency near the edge of the canvas's
    // device.
    auto blur = SkImageFilters::Blur(sigma, sigma, SkTileMode::kClamp, nullptr);
    auto rec = SkCanvasPriv::ScaledBackdropLayer(nullptr, nullptr, blur.get(), scaleFactor, 0);
    canvas->saveLayer(rec);
    // draw something inside, just to demonstrate that we don't blur the new contents,
    // just the backdrop.
    p.setColor(SK_ColorYELLOW);
    p.setShader(nullptr);
    canvas->drawCircle(cx, cy, 16, p);
    canvas->restore();
}

class ProtectedSlide : public Slide {
public:
    ProtectedSlide() { fName = "Protected"; }

    SkISize getDimensions() const override { return {kSize, 2*kSize}; }

    void draw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorDKGRAY);

#if defined(SK_GANESH)
        GrDirectContext* dContext = GrAsDirectContext(canvas->recordingContext());
        if (!dContext) {
            canvas->clear(SK_ColorGREEN);
            return;
        }

        if (fCachedContext != dContext) {
            fCachedContext = dContext;

            // Intentionally leak these. The issue is that, on Android, Viewer keeps recreating
            // the context w/o signaling the slides.
            (void) fProtectedImage.release();
            (void) fUnProtectedImage.release();

            if (ProtectedUtils::ContextSupportsProtected(dContext)) {
                fProtectedImage = ProtectedUtils::CreateProtectedSkImage(dContext, { 64, 64 },
                                                                         SkColors::kCyan,
                                                                         /* isProtected= */ true);
            }

            fUnProtectedImage = ProtectedUtils::CreateProtectedSkImage(dContext, { 64, 64 },
                                                                       SkColors::kCyan,
                                                                       /* isProtected= */ false);
        }

#if 0
        SkPaint stroke;
        stroke.setStyle(SkPaint::kStroke_Style);
        stroke.setStrokeWidth(2);

        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        paint.setShader(fProtectedImage ? fProtectedImage->makeShader({}) : nullptr);
        paint.setImageFilter(SkImageFilters::Blur(10, 10, nullptr));

        canvas->drawRect(SkRect::MakeWH(kSize, kSize), paint);
        canvas->drawRect(SkRect::MakeWH(kSize, kSize), stroke);

        paint.setShader(fUnProtectedImage->makeShader({}));
        canvas->drawRect(SkRect::MakeXYWH(0, kSize, kSize, kSize), paint);
        canvas->drawRect(SkRect::MakeXYWH(0, kSize, kSize, kSize), stroke);
#endif

        do_draw(canvas, 1.0f, fProtectedImage.get());

#endif // SK_GANESH
    }

private:
    static const int kSize = 256;

    GrDirectContext* fCachedContext = nullptr;
    sk_sp<SkImage> fProtectedImage;
    sk_sp<SkImage> fUnProtectedImage;
};

DEF_SLIDE( return new ProtectedSlide(); )
