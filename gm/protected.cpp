/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkImageFilters.h"
#include "include/gpu/GrDirectContext.h"

#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "tools/gpu/BackendSurfaceFactory.h"
#include "tools/gpu/BackendTextureImageFactory.h"

#ifdef SK_GL
#include "src/gpu/ganesh/gl/GrGLCaps.h"
#endif
#ifdef SK_VULKAN
#include "src/gpu/ganesh/vk/GrVkCaps.h"
#endif

namespace {

#if 0
static sk_sp<SkImage> create_image() {
    sk_sp<SkSurface> srcSurface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(256, 256));
    SkCanvas* srcCanvas = srcSurface->getCanvas();

    srcCanvas->clear(SK_ColorRED);

    return srcSurface->makeImageSnapshot();
}
#endif

bool context_supports_protected(GrDirectContext* dContext) {
    [[maybe_unused]] const GrCaps* caps = dContext->priv().caps();

#ifdef SK_GL
    if (dContext->backend() == GrBackendApi::kOpenGL) {
        const GrGLCaps* glCaps = static_cast<const GrGLCaps*>(caps);
        return glCaps->supportsProtected();
    }
#endif
#ifdef SK_VULKAN
    if (dContext->backend() == GrBackendApi::kVulkan) {
        const GrVkCaps* vkCaps = static_cast<const GrVkCaps*>(caps);
        return vkCaps->supportsProtectedMemory();
    }
#endif
    if (dContext->backend() == GrBackendApi::kMock) {
        return true;
    }

    // Metal, Dawn and D3D don't support protected textures
    return false;
}

#if 0
sk_sp<SkSurface> create_protected_sksurface(GrDirectContext* dContext,
                                            bool textureable,
                                            bool isProtected) {
    const int kW = 8;
    const int kH = 8;
    SkSurfaceProps surfaceProps = SkSurfaceProps(0, kRGB_H_SkPixelGeometry);
    sk_sp<SkSurface> surface;
    if (textureable) {
        surface = sk_gpu_test::MakeBackendTextureSurface(dContext,
                                                         {kW, kH},
                                                         kTopLeft_GrSurfaceOrigin,
                                                         1,
                                                         kRGBA_8888_SkColorType,
                /* colorSpace= */ nullptr,
                                                         GrMipmapped::kNo,
                                                         skgpu::Protected(isProtected),
                                                         &surfaceProps);
    } else {
        surface = sk_gpu_test::MakeBackendRenderTargetSurface(dContext,
                                                              {kW, kH},
                                                              kTopLeft_GrSurfaceOrigin,
                                                              1,
                                                              kRGBA_8888_SkColorType,
                /* colorSpace= */ nullptr,
                                                              skgpu::Protected(isProtected),
                                                              &surfaceProps);
    }
    if (!surface) {
        SkDebugf("Could not create %s surface.", isProtected ? "protected" : "unprotected");
        return nullptr;
    }

    SkCanvas* canvas = surface->getCanvas();

    canvas->clear(SkColors::kBlue);

    if (textureable) {
        GrBackendTexture backendTex = SkSurfaces::GetBackendTexture(
                surface.get(), SkSurfaces::BackendHandleAccess::kFlushRead);
        SkASSERT(backendTex.isValid());
        SkASSERT(backendTex.isProtected() == isProtected);
    } else {
        GrBackendRenderTarget backendRT = SkSurfaces::GetBackendRenderTarget(
                surface.get(), SkSurfaces::BackendHandleAccess::kFlushRead);
        SkASSERT(backendRT.isValid());
        SkASSERT(backendRT.isProtected() == isProtected);
    }

    return surface;
}
#endif

void check_image_be_protection(SkImage* image,
                               bool expectingProtected) {
    GrBackendTexture beTex;
    GrSurfaceOrigin origin;
    bool result = SkImages::GetBackendTextureFromImage(image,
                                                       &beTex,
                                                       true,
                                                       &origin);
    if (!result) {
        SkDebugf("GetBackendTextureFromImage failed");
        return;
    }

    SkASSERT(beTex.isValid());
    SkASSERT(beTex.isProtected() == expectingProtected);
}

sk_sp<SkImage> create_protected_skimage(GrDirectContext* dContext,
                                        SkISize size,
                                        SkColor4f color,
                                        bool isProtected) {
    SkImageInfo ii = SkImageInfo::Make(size, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkImage> image = sk_gpu_test::MakeBackendTextureImage(dContext,
                                                                ii,
                                                                color,
                                                                GrMipmapped::kNo,
                                                                GrRenderable::kNo,
                                                                kTopLeft_GrSurfaceOrigin,
                                                                skgpu::Protected(isProtected));
    if (!image) {
        SkDebugf("Could not create %s image.", isProtected ? "protected" : "unprotected");
        return nullptr;
    }

    check_image_be_protection(image.get(), isProtected);

    return image;
}

} // anonymous namespace

namespace skiagm {

class ProtectedGM : public GpuGM {
public:
    ProtectedGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString onShortName() override {
        return SkString("protected1");
    }

    SkISize onISize() override {
        return SkISize::Make(256, 256);
    }

    DrawResult onDraw(GrRecordingContext* rContext, SkCanvas* canvas, SkString* errorMsg) override {
        GrDirectContext* dContext = GrAsDirectContext(rContext);
        if (!dContext) {
            SkDebugf("No context\n");
            *errorMsg = kErrorMsg_DrawSkippedGpuOnly;
            return DrawResult::kSkip;
        }

        if (!context_supports_protected(dContext)) {
            SkDebugf("No protected support\n");
//            *errorMsg = kErrorMsg_DrawSkippedGpuOnly;
//            return DrawResult::kSkip;
        }

        if (!fImage) {
            fImage = create_protected_skimage(dContext, { 256, 256 }, SkColors::kRed,
                                              /* isProtected= */ true);
//            fImage = create_image();
        }

        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
//        paint.setShader(fImage->makeShader({}));
//        paint.setImageFilter(SkImageFilters::Blur(10, 10, nullptr));

        canvas->drawRect(SkRect::MakeWH(256, 256), paint);

        SkPaint stroke;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(2);
        canvas->drawRect(SkRect::MakeWH(256, 256), paint);

        return DrawResult::kOk;
    }

private:
    sk_sp<SkImage> fImage;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ProtectedGM;)

}  // namespace skiagm
