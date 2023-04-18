/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/image/SkImage_LazyTexture.h"

#include "include/gpu/GpuTypes.h"
#include "include/core/SkImageGenerator.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/GrYUVATextureProxies.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceContext.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#include "src/gpu/ganesh/effects/GrYUVtoRGBEffect.h"
#include "src/gpu/ganesh/image/GrImageUtils.h"

sk_sp<SkImage> SkImage_LazyTexture::onMakeSubset(const SkIRect& subset,
                                                 GrDirectContext* direct) const {
    auto pixels = direct ? SkImages::TextureFromImage(direct, this) : this->makeRasterImage();
    return pixels ? pixels->makeSubset(subset, direct) : nullptr;
}

bool SkImage_LazyTexture::readPixelsProxy(GrDirectContext* ctx, const SkPixmap& pixmap) const {
    if (!ctx) {
        return false;
    }
    GrSurfaceProxyView view = skgpu::ganesh::LockTextureProxyView(ctx,
                                                                  this,
                                                                  GrImageTexGenPolicy::kDraw,
                                                                  GrMipmapped::kNo);

    if (!view) {
        return false;
    }

    GrColorType ct = skgpu::ganesh::ColorTypeOfLockTextureProxy(ctx->priv().caps(),
                                                                this->colorType());
    GrColorInfo colorInfo(ct, this->alphaType(), this->refColorSpace());
    auto sContext = ctx->priv().makeSC(std::move(view), colorInfo);
    if (!sContext) {
        return false;
    }
    size_t rowBytes = this->imageInfo().minRowBytes();
    return sContext->readPixels(ctx, {this->imageInfo(), pixmap.writable_addr(), rowBytes}, {0, 0});
}

namespace SkImages {
sk_sp<SkImage> DeferredFromTextureGenerator(std::unique_ptr<SkImageGenerator> generator) {
    SkImage_Lazy::Validator validator(
            SharedGenerator::Make(std::move(generator)), nullptr, nullptr);

    return validator ? sk_make_sp<SkImage_LazyTexture>(&validator) : nullptr;
}
}