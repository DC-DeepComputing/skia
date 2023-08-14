/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_GANESH) && defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

#include "include/android/SkImageAndroid.h"
#include "include/android/SkSurfaceAndroid.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tools/ToolUtils.h"
#include "tools/gpu/android/AHBUtils_android.h"
#include "tools/viewer/Slide.h"

namespace {

sk_sp<SkImage> create_skia_image(GrDirectContext* dContext, int width, int height,
                                 SkColor color, bool isProtected) {
    SkImageInfo ii = SkImageInfo::Make(width, height, kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);

    sk_sp<SkSurface> tmpSurface = SkSurfaces::RenderTarget(dContext,
                                                           skgpu::Budgeted::kYes,
                                                           ii,
                                                           /* sampleCount= */ 1,
                                                           kTopLeft_GrSurfaceOrigin,
                                                           /* surfaceProps= */ nullptr,
                                                           /* shouldCreateWithMips= */ false,
                                                           /* isProtected= */ true);
    if (!tmpSurface) {
        return nullptr;
    }

    ToolUtils::draw_checkerboard(tmpSurface->getCanvas(), color, SK_ColorTRANSPARENT, 32);

    return tmpSurface->makeImageSnapshot();
}

} // anonymous namespace

class ProtectedSlide : public Slide {
public:
    ProtectedSlide() {
        fName = "Protected";
        fRedBitmap = ToolUtils::create_checkerboard_bitmap(kSize, kSize,
                                                           SK_ColorRED, SK_ColorTRANSPARENT,
                                                           /* checkSize= */ 32);
        fGreenBitmap = ToolUtils::create_checkerboard_bitmap(kSize, kSize,
                                                             SK_ColorGREEN, SK_ColorTRANSPARENT,
                                                             /* checkSize= */ 32);
    }

    SkISize getDimensions() const override { return {kSize, 2*kSize}; }

    void draw(SkCanvas* origCanvas) override {
        origCanvas->clear(SK_ColorDKGRAY);

        GrDirectContext* dContext = GrAsDirectContext(origCanvas->recordingContext());
        if (!dContext) {
            origCanvas->clear(SK_ColorGREEN);
            return;
        }

        if (fCachedContext != dContext) {
            fProtectedAHBImage = nullptr;
            if (fProtectedAHB) {
                fProtectedAHB->release();
                fProtectedAHB = nullptr;
            }
            fUnprotectedAHBImage = nullptr;
            if (fUnprotectedAHB) {
                fUnprotectedAHB->release();
                fUnprotectedAHB = nullptr;
            }

            fCachedContext = dContext;
            fProtectedAHBImage = AHBUtils::CreateAHBBackedImage(dContext, fRedBitmap,
                                                                kTopLeft_GrSurfaceOrigin,
                                                                /* isProtected= */ true,
                                                                &fProtectedAHB);
            fUnprotectedAHBImage = AHBUtils::CreateAHBBackedImage(dContext, fGreenBitmap,
                                                                  kTopLeft_GrSurfaceOrigin,
                                                                  /* isProtected= */ false,
                                                                  &fUnprotectedAHB);
            fProtectedSkImage = create_skia_image(dContext, kSize, kSize, SK_ColorBLUE,
                                                  /* isProtected= */ true);
            fUnprotectedSkImage = create_skia_image(dContext, kSize, kSize, SK_ColorCYAN,
                                                    /* isProtected= */ false);
        }

        // Pick one of the four combinations to draw. Only the protected AHB-backed image will
        // reproduce the bug (b/242266174).
        SkImage* imgToUse = fProtectedAHBImage.get();
//        SkImage* imgToUse = fUnprotectedAHBImage.get();
//        SkImage* imgToUse = fProtectedSkImage.get();
//        SkImage* imgToUse = fUnprotectedSkImage.get();

        sk_sp<SkImage> indirectImg;

        {
            SkImageInfo ii = SkImageInfo::Make(kSize, kSize, kRGBA_8888_SkColorType,
                                               kPremul_SkAlphaType);
            sk_sp<SkSurface> tmpS = SkSurfaces::RenderTarget(dContext,
                                                             skgpu::Budgeted::kYes,
                                                             ii,
                                                             /* sampleCount= */ 1,
                                                             kTopLeft_GrSurfaceOrigin,
                                                             /* surfaceProps= */ nullptr,
                                                             /* shouldCreateWithMips= */ false,
                                                             /* isProtected= */ true);

            tmpS->getCanvas()->clear(SK_ColorMAGENTA);
            tmpS->getCanvas()->drawCircle(64, 64, 32, SkPaint());

            // For protected AHB-backed images this draw seems to poison all above the draws too
            tmpS->getCanvas()->drawImage(imgToUse, 0, 0);
            indirectImg = tmpS->makeImageSnapshot();
        }

        origCanvas->drawImage(imgToUse, 0, 0);
        origCanvas->drawImage(indirectImg, 0, kSize);
    }

private:
    static const int kSize = 128;

    SkBitmap fRedBitmap;
    SkBitmap fGreenBitmap;

    GrDirectContext* fCachedContext = nullptr;
    sk_sp<SkImage> fProtectedAHBImage;
    AHardwareBuffer* fProtectedAHB = nullptr;
    sk_sp<SkImage> fUnprotectedAHBImage;
    AHardwareBuffer* fUnprotectedAHB = nullptr;
    sk_sp<SkImage> fProtectedSkImage;
    sk_sp<SkImage> fUnprotectedSkImage;
};

DEF_SLIDE( return new ProtectedSlide(); )

#endif
