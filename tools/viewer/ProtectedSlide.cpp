/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"

#if 1
#include "include/effects/SkImageFilters.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tools/ToolUtils.h"
#include "tools/gpu/ProtectedUtils.h"
#include "tools/viewer/Slide.h"

#include "include/core/SkBitmap.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkCanvasPriv.h"


#include "include/android/SkImageAndroid.h"
#include "include/android/SkSurfaceAndroid.h"
#include <android/hardware_buffer.h>

#if 1
namespace {

static void cleanup_resources(AHardwareBuffer* buffer) {
    if (buffer) {
        AHardwareBuffer_release(buffer);
    }
}

sk_sp<SkSurface> create_protected_AHB_surface(GrDirectContext* dContext, int width, int height) {

    AHardwareBuffer* buffer = nullptr;

    AHardwareBuffer_Desc hwbDesc;
    hwbDesc.width = width;
    hwbDesc.height = height;
    hwbDesc.layers = 1;
    hwbDesc.usage = AHARDWAREBUFFER_USAGE_CPU_READ_NEVER |
                    AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER |
                    AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE |
                    AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT;

    hwbDesc.usage |= AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT;

    hwbDesc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    // The following three are not used in the allocate
    hwbDesc.stride = 0;
    hwbDesc.rfu0= 0;
    hwbDesc.rfu1= 0;

    if (int error = AHardwareBuffer_allocate(&hwbDesc, &buffer)) {
        SkDebugf("------- Failed to allocated hardware buffer, error: %d\n", error);
        cleanup_resources(buffer);
        return nullptr;
    }

    sk_sp<SkSurface> surface =
            SkSurfaces::WrapAndroidHardwareBuffer(dContext, buffer, kTopLeft_GrSurfaceOrigin,
                                                  nullptr, nullptr);
    if (!surface) {
        SkDebugf("------- Failed to make SkSurface.\n");
        cleanup_resources(buffer);
        return nullptr;
    }

    return surface;
}

sk_sp<SkImage> create_protected_AHB_image(GrDirectContext* dContext,
                                          SkColor color, int width, int height) {
    sk_sp<SkSurface> surf = create_protected_AHB_surface(dContext, width, height);

    SkDebugf("------- create_protected_AHB_image - begin image creation %dx%d\n",
             width, height);
    ToolUtils::draw_checkerboard(surf->getCanvas(), color, SK_ColorTRANSPARENT, 32);

    sk_sp<SkImage> tmp = surf->makeImageSnapshot();
    SkDebugf("------- create_protected_AHB_image - end image creation\n");

    return tmp;
}

sk_sp<SkImage> create_unprotected_AHB_image(SkColor color, int width, int height) {

    const SkBitmap srcBitmap = ToolUtils::create_checkerboard_bitmap(width, height, color,
                                                                     SK_ColorTRANSPARENT, 32);

    AHardwareBuffer* buffer = nullptr;

    AHardwareBuffer_Desc hwbDesc;
    hwbDesc.width = width;
    hwbDesc.height = height;
    hwbDesc.layers = 1;
    hwbDesc.usage = AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN |
                    AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;

    hwbDesc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    // The following three are not used in the allocate
    hwbDesc.stride = 0;
    hwbDesc.rfu0= 0;
    hwbDesc.rfu1= 0;

    if (int error = AHardwareBuffer_allocate(&hwbDesc, &buffer)) {
        SkDebugf("$$$$$$$ Failed to allocated hardware buffer, error: %d", error);
        cleanup_resources(buffer);
        return nullptr;
    }

    // Get actual desc for allocated buffer so we know the stride for uploading cpu data.
    AHardwareBuffer_describe(buffer, &hwbDesc);

    uint32_t* bufferAddr;
    if (AHardwareBuffer_lock(buffer, AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN, -1, nullptr,
                             reinterpret_cast<void**>(&bufferAddr))) {
        SkDebugf("$$$$$$$ Failed to lock hardware buffer");
        cleanup_resources(buffer);
        return nullptr;
    }

    int bbp = srcBitmap.bytesPerPixel();
    uint32_t* src = (uint32_t*)srcBitmap.getPixels();

    uint32_t* dst = bufferAddr;
    for (int y = 0; y < height; ++y) {
        memcpy(dst, src, width * bbp);
        src += width;
        dst += hwbDesc.stride;
    }
    AHardwareBuffer_unlock(buffer, nullptr);

    return SkImages::DeferredFromAHardwareBuffer(buffer, kPremul_SkAlphaType,
                                                 nullptr, kTopLeft_GrSurfaceOrigin);
}

sk_sp<SkImage> create_skia_image(GrDirectContext* dContext, int width, int height,
                                 SkColor color, bool isProtected) {
    SkImageInfo ii = SkImageInfo::Make(width, height, kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);

    sk_sp<SkSurface> tmpS = SkSurfaces::RenderTarget(dContext,
                                                     skgpu::Budgeted::kYes,
                                                     ii,
                                                     /* sampleCount= */ 1,
                                                     kTopLeft_GrSurfaceOrigin,
                                                     /* surfaceProps= */ nullptr,
                                                     /* shouldCreateWithMips= */ false,
                                                     /* isProtected= */ true);

    ToolUtils::draw_checkerboard(tmpS->getCanvas(), color, SK_ColorTRANSPARENT, 32);

    return tmpS->makeImageSnapshot();
}

} // anonymous namespace

#endif

class ProtectedSlide : public Slide {
public:
    ProtectedSlide() { fName = "Protected"; }

    SkISize getDimensions() const override { return {kSize, 2*kSize}; }

    void draw(SkCanvas* origCanvas) override {
        SkDebugf("------- Entering slide\n");

        origCanvas->clear(SK_ColorDKGRAY);

#if defined(SK_GANESH)
        GrDirectContext* dContext = GrAsDirectContext(origCanvas->recordingContext());
        if (!dContext) {
            origCanvas->clear(SK_ColorGREEN);
            return;
        }

        if (fCachedContext != dContext) {
            fCachedContext = dContext;
            fProtectedAHBImage = create_protected_AHB_image(dContext, SK_ColorRED, kSize, kSize);
            fUnprotectedAHBImage = create_unprotected_AHB_image(SK_ColorGREEN, kSize, kSize);
            fProtectedSkImage = create_skia_image(dContext, kSize, kSize, SK_ColorBLUE,
                                                  /* isProtected= */ true);
            fUnprotectedSkImage = create_skia_image(dContext, kSize, kSize, SK_ColorCYAN,
                                                    /* isProtected= */ false);
        }

        // Pick one of the four combinations to draw
        SkImage* imgToUse = fProtectedAHBImage.get();
//        SkImage* imgToUse = fUnprotectedAHBImage.get();
//        SkImage* imgToUse = fProtectedSkImage.get();
//        SkImage* imgToUse = fUnprotectedSkImage.get();

        SkDebugf("------- Begin creating Indirect Image\n");

        sk_sp<SkImage> indirectI;

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

            SkDebugf("------- End creating Indirect Image\n");

            tmpS->getCanvas()->clear(SK_ColorMAGENTA);
//            tmpS->getCanvas()->drawCircle(64, 64, 32, SkPaint());

            // For protected AHB-backed images this draw seems to poison all above the draws too
            tmpS->getCanvas()->drawImage(imgToUse, 0, 0);
            indirectI = tmpS->makeImageSnapshot();
        }

        origCanvas->drawImage(imgToUse, 0, 0);
        origCanvas->drawImage(indirectI, 0, kSize);

#endif // SK_GANESH

        dContext->flushAndSubmit(true);
        SkDebugf("------- Exiting slide\n");
    }

private:
    static const int kSize = 128;

    GrDirectContext* fCachedContext = nullptr;
    sk_sp<SkImage> fProtectedAHBImage;
    sk_sp<SkImage> fUnprotectedAHBImage;
    sk_sp<SkImage> fProtectedSkImage;
    sk_sp<SkImage> fUnprotectedSkImage;
};

DEF_SLIDE( return new ProtectedSlide(); )
#endif
