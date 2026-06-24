/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_GANESH)

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/core/SkRandom.h"

#include "include/core/SkPathBuilder.h"

#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"

namespace {

SkPath make_star() {
    SkPathBuilder starPath;
    starPath.moveTo(0.0f, -33.3333f);
    starPath.lineTo(9.62f, -16.6667f);
    starPath.lineTo(28.867f, -16.6667f);
    starPath.lineTo(19.24f, 0.0f);
    starPath.lineTo(28.867f, 16.6667f);
    starPath.lineTo(9.62f, 16.6667f);
    starPath.lineTo(0.0f, 33.3333f);
    starPath.lineTo(-9.62f, 16.6667f);
    starPath.lineTo(-28.867f, 16.6667f);
    starPath.lineTo(-19.24f, 0.0f);
    starPath.lineTo(-28.867f, -16.6667f);
    starPath.lineTo(-9.62f, -16.6667f);
    starPath.close();

    return starPath.detach();
}

sk_sp<SkSurface> gpu_surface(GrDirectContext* dContext) {
    SkSurfaceProps props(SkSurfaceProps::kDynamicMSAA_Flag, kUnknown_SkPixelGeometry);

    SkImageInfo ii = SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    return SkSurfaces::RenderTarget(dContext,
                                    skgpu::Budgeted::kYes,
                                    ii,
                                    /* sampleCount= */ 4,
                                    kTopLeft_GrSurfaceOrigin,
                                    &props,
                                    /*shouldCreateWithMips=*/true);
}

} // anonymous namespace

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(SK_ColorMAGENTA);
    }

protected:
    SkString getName() const override { return SkString("arcofzorro"); }

    SkISize getISize() override { return SkISize::Make(512, 256); }

#if defined(SK_GANESH)
    void modifyGrContextOptions(GrContextOptions* ctxOptions) override {
        ctxOptions->fGpuPathRenderers &= ~GpuPathRenderers::kAtlas;
    }
#endif

    void onDraw(SkCanvas* origCanvas) override {
#if defined(SK_GANESH)
        GrDirectContext* dContext = GrAsDirectContext(origCanvas->recordingContext());
        if (!dContext) {
            return;
        }
#endif

        SkPath star = make_star();

        // This first draw clears the stencil buffer and fills it with the first star
        sk_sp<SkSurface> s1 = gpu_surface(dContext);
        {
            SkCanvas* canvas = s1->getCanvas();

            SkPaint paint;
            paint.setColor(SK_ColorBLUE);
            paint.setAntiAlias(true);

            canvas->concat(SkMatrix::ScaleTranslate(4, 4, 128, 128));
            canvas->drawPath(star, paint);
        }

        // This starts a new Task that doesn't need stencil
        sk_sp<SkSurface> s2 = gpu_surface(dContext);
        {
            SkCanvas* canvas = s2->getCanvas();

            SkPaint paint;
            paint.setColor(SK_ColorRED);
            canvas->drawRect(SkRect::MakeWH(256, 256), paint);
        }

        // This block just serves to close the active Task from s2
        {
            sk_sp<SkSurface> s3 = gpu_surface(dContext);
            SkCanvas* canvas = s3->getCanvas();

            SkPaint paint;
            paint.setColor(SK_ColorGREEN);
            canvas->drawRect(SkRect::MakeWH(256, 256), paint);
        }

        // This adds a new Task on s2 that does require stencil
        {
            SkCanvas* canvas = s2->getCanvas();

            SkPaint paint;
            paint.setColor(SK_ColorYELLOW);
            paint.setAntiAlias(true);

            canvas->concat(SkMatrix::ScaleTranslate(4, 4, 128, 128));
            canvas->drawPath(star, paint);
        }

        dContext->flush();
        dContext->submit(GrSyncCpu::kYes);

        sk_sp<SkImage> i1 = s1->makeImageSnapshot();
        origCanvas->drawImage(i1, 0, 0);

        sk_sp<SkImage> i2 = s2->makeImageSnapshot();
        origCanvas->drawImage(i2, 256, 0);
    }

};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm

#endif
