/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/ProtectedUtils.h"

#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "tools/gpu/BackendSurfaceFactory.h"
#include "tools/gpu/BackendTextureImageFactory.h"

#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Texture.h"
#include "src/image/SkImage_Base.h"

using namespace skgpu;
using namespace skgpu::graphite;

namespace ProtectedUtils {

sk_sp<SkSurface> CreateProtectedSkSurface(Recorder* recorder,
                                          SkISize size,
//                                          bool textureable,
                                          Protected isProtected) {
    SkImageInfo ii = SkImageInfo::Make(size, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkSurfaceProps surfaceProps = SkSurfaceProps(0, kRGB_H_SkPixelGeometry);

    sk_sp<SkSurface> surface = sk_gpu_test::MakeBackendTextureSurface(recorder,
                                                                      ii,
                                                                      Origin::kTopLeft,
                                                                      /* sampleCount=*/ 1,
                                                                      Mipmapped::kNo,
                                                                      Protected(isProtected),
                                                                      &surfaceProps);
    if (!surface) {
        SK_ABORT("Could not create %s surface.",
                 isProtected == Protected::kYes ? "protected" : "unprotected");
        return nullptr;
    }

    SkCanvas* canvas = surface->getCanvas();

    canvas->clear(SkColors::kBlue);

#if 0
    BackendTexture backendTex = SkSurfaces::GetBackendTexture(
            surface.get() /*, SkSurfaces::BackendHandleAccess::kFlushRead */);
    SkASSERT(backendTex.isValid());
    SkASSERT(backendTex.info().isProtected() == isProtected);
#endif

    return surface;
}

void CheckSurfaceBEProtection(const SkSurface* surface, Protected expected) {

}

void CheckImageBEProtection(const SkImage* image, Protected expected) {
    SkASSERT(Protected(image->isProtected()) == expected);

    const SkImage_Base* ib = as_IB(image);

    if (!ib->isGraphiteBacked()) {
        SkASSERT(0);
        return;
    }

    const Image* imgGraphite = static_cast<const Image*>(ib);

    const TextureProxyView& view = imgGraphite->textureProxyView();
    TextureProxy* proxy = view.proxy();
    SkASSERT(proxy->isProtected() == expected);

    Texture* texture = proxy->texture();

    SkASSERT(texture->textureInfo().isValid());
    SkASSERT(texture->textureInfo().isProtected() == expected);
}

sk_sp<SkImage> CreateProtectedSkImage(Recorder* recorder,
                                      SkISize size,
                                      SkColor4f color,
                                      bool isProtected) {
    SkImageInfo ii = SkImageInfo::Make(size, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkImage> image = sk_gpu_test::MakeBackendTextureImage2(recorder,
                                                                ii,
                                                                color,
                                                                Mipmapped::kNo,
                                                                GrRenderable::kNo,
                                                                Origin::kBottomLeft,
                                                                Protected(isProtected));
    if (!image) {
        SK_ABORT("Could not create %s image.", isProtected ? "protected" : "unprotected");
        return nullptr;
    }

    CheckImageBEProtection(image.get(), isProtected);

    return image;
}

}  // namespace ProtectedUtils
