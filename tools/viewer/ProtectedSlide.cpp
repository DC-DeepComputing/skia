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

namespace {

#if 0
static sk_sp<SkImage> create_image(GrDirectContext* dContext) {
    SkImageInfo ii = SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> srcSurface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo,
                                                           ii, 1, nullptr, /* isProtected=*/ false);
//    sk_sp<SkSurface> srcSurface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(256, 256));
    SkCanvas* srcCanvas = srcSurface->getCanvas();

    srcCanvas->clear(SK_ColorRED);

    return srcSurface->makeImageSnapshot();
}
#endif

} // anonymous namespace

class ProtectedSlide : public Slide {
public:
    ProtectedSlide() { fName = "Protected";
    }

    SkISize getDimensions() const override { return {256, 512}; }

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
                fProtectedImage = ProtectedUtils::CreateProtectedSkImage(dContext, { 256, 256 },
                                                                         SkColors::kRed,
                                                                         /* isProtected= */ true);
            }

            fUnProtectedImage = ProtectedUtils::CreateProtectedSkImage(dContext, { 256, 256 },
                                                                       SkColors::kRed,
                                                                       /* isProtected= */ false);
        }

        SkPaint stroke;
        stroke.setStyle(SkPaint::kStroke_Style);
        stroke.setStrokeWidth(2);

        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        paint.setShader(fProtectedImage ? fProtectedImage->makeShader({}) : nullptr);
        paint.setImageFilter(SkImageFilters::Blur(10, 10, nullptr));

        canvas->drawRect(SkRect::MakeWH(256, 256), paint);
        canvas->drawRect(SkRect::MakeWH(256, 256), stroke);

        paint.setShader(fUnProtectedImage->makeShader({}));
        canvas->drawRect(SkRect::MakeXYWH(0, 256, 256, 256), paint);
        canvas->drawRect(SkRect::MakeXYWH(0, 256, 256, 256), stroke);
#endif // SK_GANESH
    }

private:
    GrDirectContext* fCachedContext = nullptr;
    sk_sp<SkImage> fProtectedImage;
    sk_sp<SkImage> fUnProtectedImage;
};

DEF_SLIDE( return new ProtectedSlide(); )
