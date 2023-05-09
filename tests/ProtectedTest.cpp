/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_GANESH)

#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "tests/Test.h"
#include "tools/gpu/BackendSurfaceFactory.h"
#include "tools/gpu/BackendTextureImageFactory.h"

#ifdef SK_GL
#include "src/gpu/ganesh/gl/GrGLCaps.h"
#endif
#ifdef SK_VULKAN
#include "src/gpu/ganesh/vk/GrVkCaps.h"
#endif

namespace {

bool context_supports_protected(GrDirectContext* dContext) {
    const GrCaps* caps = dContext->priv().caps();

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

sk_sp<SkSurface> create_protected_sksurface(GrDirectContext* dContext,
                                            skiatest::Reporter* reporter,
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
                                                         /* color space */ nullptr,
                                                         GrMipmapped::kNo,
                                                         skgpu::Protected(isProtected),
                                                         &surfaceProps);
    } else {
        surface = sk_gpu_test::MakeBackendRenderTargetSurface(dContext,
                                                              {kW, kH},
                                                              kTopLeft_GrSurfaceOrigin,
                                                              1,
                                                              kRGBA_8888_SkColorType,
                                                              /* color space */ nullptr,
                                                              skgpu::Protected(isProtected),
                                                              &surfaceProps);
    }
    if (!surface) {
        ERRORF(reporter, "Could not create %s surface.", isProtected ? "protected" : "unprotected");
        return nullptr;
    }

    if (textureable) {
        GrBackendTexture backendTex = SkSurfaces::GetBackendTexture(
                surface.get(), SkSurfaces::BackendHandleAccess::kFlushRead);
        REPORTER_ASSERT(reporter, backendTex.isValid());
        SkDebugf("S: %d %d\n", backendTex.isProtectedS(), isProtected);
        REPORTER_ASSERT(reporter, backendTex.isProtectedS() == isProtected);
    } else {
        GrBackendRenderTarget backendRT = SkSurfaces::GetBackendRenderTarget(
                surface.get(), SkSurfaces::BackendHandleAccess::kFlushRead);
        REPORTER_ASSERT(reporter, backendRT.isValid());
        SkDebugf("RT: %d %d\n", backendRT.isProtectedRT(), isProtected);
        REPORTER_ASSERT(reporter, backendRT.isProtectedRT() == isProtected);
    }

    return surface;
}

sk_sp<SkImage> create_protected_skimage(GrDirectContext* dContext,
                                        skiatest::Reporter* reporter,
                                        bool isProtected) {
    const int kW = 8;
    const int kH = 8;

    SkImageInfo ii = SkImageInfo::Make(kW, kH, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkImage> image = sk_gpu_test::MakeBackendTextureImage(dContext,
                                                                ii,
                                                                SkColors::kBlue,
                                                                GrMipmapped::kNo,
                                                                GrRenderable::kNo,
                                                                kTopLeft_GrSurfaceOrigin,
                                                                skgpu::Protected(isProtected));
    if (!image) {
        ERRORF(reporter, "Could not create %s image.", isProtected ? "protected" : "unprotected");
        return nullptr;
    }

    GrBackendTexture beTex;
    GrSurfaceOrigin origin;
    bool result = SkImages::GetBackendTextureFromImage(image.get(),
                                                       &beTex,
                                                       true,
                                                       &origin);
    if (!result) {
        ERRORF(reporter, "Could not create %s image.", isProtected ? "protected" : "unprotected");
        return nullptr;
    }

    REPORTER_ASSERT(reporter, beTex.isValid());
    SkDebugf("RT: %d %d\n", beTex.isProtectedS(), isProtected);
    REPORTER_ASSERT(reporter, beTex.isProtectedS() == isProtected);

    return image;
}


} // anonymous namespace

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(Protected_SmokeTest, reporter, ctxInfo, CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();

    if (!context_supports_protected(dContext)) {
        SkDebugf("Protected content not supported by %s\n", GrBackendApiToStr(dContext->backend()));
        return;
    }

    SkDebugf("Protected content _IS_ supported by %s\n", GrBackendApiToStr(dContext->backend()));

    SkDebugf("Surfaces\n");
    for (bool textureable : { true, false }) {
        for (bool isProtected : { true, false }) {
            create_protected_sksurface(dContext, reporter, textureable, isProtected);
        }
    }

    SkDebugf("Images\n");
    for (bool isProtected : { true, false }) {
        create_protected_skimage(dContext, reporter, isProtected);
    }

    SkDebugf("BackendTextures\n");
    for (bool renderable : { true, false }) {
        for (bool isProtected : { true, false }) {
            GrBackendTexture beTex = dContext->createBackendTexture(10,
                                                                    10,
                                                                    kRGBA_8888_SkColorType,
                                                                    SkColors::kTransparent,
                                                                    GrMipmapped::kNo,
                                                                    GrRenderable(renderable),
                                                                    GrProtected(isProtected));

            REPORTER_ASSERT(reporter, beTex.isValid());
            SkDebugf("S: %d %d\n", beTex.isProtectedS(), isProtected);
            REPORTER_ASSERT(reporter, beTex.isProtectedS() == isProtected);
        }
    }
}

#endif  // defined(SK_GANESH)
