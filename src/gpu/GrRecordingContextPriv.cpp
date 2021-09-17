/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrRecordingContextPriv.h"

#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrSurfaceProxyView.h"

#if SK_GPU_V1
#include "src/gpu/v1/Device_v1.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"
#include "src/gpu/v1/SurfaceFillContext_v1.h"
#endif
#if SK_GPU_V2
#include "src/gpu/v2/Device_v2.h"
#include "src/gpu/v2/SurfaceDrawContext_v2.h"
#include "src/gpu/v2/SurfaceFillContext_v2.h"
#endif

namespace {

std::unique_ptr<skgpu::SurfaceFillContext> alloc_SFC(GrRecordingContext* rContext,
                                                     GrSurfaceProxyView readView,
                                                     GrSurfaceProxyView writeView,
                                                     const GrColorInfo& info) {
#if GR_TEST_UTILS
    if (rContext->priv().options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
        return std::make_unique<skgpu::v2::SurfaceFillContext>(rContext,
                                                               std::move(readView),
                                                               std::move(writeView),
                                                               info);
#endif
    } else
#endif // GR_TEST_UTILS
    {
#if SK_GPU_V1
        return std::make_unique<skgpu::v1::SurfaceFillContext>(rContext,
                                                               std::move(readView),
                                                               std::move(writeView),
                                                               info);
#endif
    }

    return nullptr;
}

std::unique_ptr<skgpu::SurfaceFillContext> make_SDC0(GrRecordingContext* rContext,
                                                     GrSurfaceProxyView readView,
                                                     GrSurfaceProxyView writeView,
                                                     GrColorType colorType,
                                                     sk_sp<SkColorSpace> cs) {
#if GR_TEST_UTILS
    if (rContext->priv().options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
        return skgpu::v2::SurfaceDrawContext::Make0(rContext,
                                                    std::move(readView),
                                                    std::move(writeView),
                                                    colorType,
                                                    std::move(cs),
                                                    SkSurfaceProps());
#endif
    } else
#endif // GR_TEST_UTILS
    {
#if SK_GPU_V1
        return skgpu::v1::SurfaceDrawContext::Make0(rContext,
                                                    std::move(readView),
                                                    std::move(writeView),
                                                    colorType,
                                                    std::move(cs),
                                                    SkSurfaceProps(),
                                                    /* flushTimeOpsTask */ false);
#endif
    }

    return nullptr;
}

std::unique_ptr<skgpu::SurfaceFillContext> make_SFC_or_SDC(GrRecordingContext* rContext,
                                                           sk_sp<GrTextureProxy> proxy,
                                                           GrSwizzle readSwizzle,
                                                           GrSwizzle writeSwizzle,
                                                           GrSurfaceOrigin origin,
                                                           const GrColorInfo& info) {
    GrSurfaceProxyView readView (          proxy,  origin, readSwizzle);
    GrSurfaceProxyView writeView(std::move(proxy), origin, writeSwizzle);

    if (info.alphaType() == kPremul_SkAlphaType || info.alphaType() == kOpaque_SkAlphaType) {
        return make_SDC0(rContext,
                         std::move(readView),
                         std::move(writeView),
                         info.colorType(),
                         info.refColorSpace());
    }

    return alloc_SFC(rContext,
                     std::move(readView),
                     std::move(writeView),
                     std::move(info));
}

} // anonymous namespace

void GrRecordingContextPriv::addOnFlushCallbackObject(GrOnFlushCallbackObject* onFlushCBObject) {
    this->context()->addOnFlushCallbackObject(onFlushCBObject);
}

sk_sp<skgpu::BaseDevice> GrRecordingContextPriv::createDevice(GrColorType colorType,
                                                              sk_sp<GrSurfaceProxy> proxy,
                                                              sk_sp<SkColorSpace> colorSpace,
                                                              GrSurfaceOrigin origin,
                                                              const SkSurfaceProps& props,
                                                              skgpu::BaseDevice::InitContents init) {
#if GR_TEST_UTILS
    if (this->options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
        return skgpu::v2::Device::Make(this->context(), colorType, std::move(proxy),
                                       std::move(colorSpace), origin, props, init);
#else
        return nullptr;
#endif // GR_TEST_UTILS
    } else
#endif
    {
#if SK_GPU_V1
        return skgpu::v1::Device::Make(this->context(), colorType, std::move(proxy),
                                       std::move(colorSpace), origin, props, init);
#else
        return nullptr;
#endif
    }
}

sk_sp<skgpu::BaseDevice> GrRecordingContextPriv::createDevice(SkBudgeted budgeted,
                                                              const SkImageInfo& ii,
                                                              SkBackingFit fit,
                                                              int sampleCount,
                                                              GrMipmapped mipmapped,
                                                              GrProtected isProtected,
                                                              GrSurfaceOrigin origin,
                                                              const SkSurfaceProps& props,
                                                              skgpu::BaseDevice::InitContents init) {
#if GR_TEST_UTILS
    if (this->options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
        return skgpu::v2::Device::Make(this->context(), budgeted, ii, fit, sampleCount,
                                       mipmapped, isProtected, origin, props, init);
#else
        return nullptr;
#endif
    } else
#endif // GR_TEST_UTILS
    {
#if SK_GPU_V1
        return skgpu::v1::Device::Make(this->context(), budgeted, ii, fit, sampleCount,
                                       mipmapped, isProtected, origin, props, init);
#else
        return nullptr;
#endif
    }
}

void GrRecordingContextPriv::moveRenderTasksToDDL(SkDeferredDisplayList* ddl) {
    this->context()->drawingManager()->moveRenderTasksToDDL(ddl);
}

GrSDFTControl GrRecordingContextPriv::getSDFTControl(bool useSDFTForSmallText) const {
    return GrSDFTControl{
            this->caps()->shaderCaps()->supportsDistanceFieldText(),
            useSDFTForSmallText,
            this->options().fMinDistanceFieldFontSize,
            this->options().fGlyphsAsPathsFontSize};
}

std::unique_ptr<skgpu::SurfaceContext> GrRecordingContextPriv::makeSC(GrSurfaceProxyView readView,
                                                                      const GrColorInfo& info) {
    // It is probably not necessary to check if the context is abandoned here since uses of the
    // SurfaceContext which need the context will mostly likely fail later on w/o an issue.
    // However having this here adds some reassurance in case there is a path that doesn't
    // handle an abandoned context correctly. It also lets us early out of some extra work.
    if (this->context()->abandoned()) {
        return nullptr;
    }
    GrSurfaceProxy* proxy = readView.proxy();
    SkASSERT(proxy && proxy->asTextureProxy());

    std::unique_ptr<skgpu::SurfaceContext> sc;
    if (proxy->asRenderTargetProxy()) {
        // Will we ever want a swizzle that is not the default write swizzle for the format and
        // colorType here? If so we will need to manually pass that in.
        GrSwizzle writeSwizzle;
        if (info.colorType() != GrColorType::kUnknown) {
            writeSwizzle = this->caps()->getWriteSwizzle(proxy->backendFormat(),
                                                         info.colorType());
        }
        GrSurfaceProxyView writeView(readView.refProxy(), readView.origin(), writeSwizzle);
        if (info.alphaType() == kPremul_SkAlphaType || info.alphaType() == kOpaque_SkAlphaType) {
            sc = make_SDC0(this->context(),
                           std::move(readView),
                           std::move(writeView),
                           info.colorType(),
                           info.refColorSpace());
        } else {
            sc = alloc_SFC(this->context(), std::move(readView), std::move(writeView), info);
        }
    } else {
        sc = std::make_unique<skgpu::SurfaceContext>(this->context(), std::move(readView), info);
    }
    SkDEBUGCODE(sc->validate();)
    return sc;
}

std::unique_ptr<skgpu::SurfaceContext> GrRecordingContextPriv::makeSC(const GrImageInfo& info,
                                                                      const GrBackendFormat& format,
                                                                      SkBackingFit fit,
                                                                      GrSurfaceOrigin origin,
                                                                      GrRenderable renderable,
                                                                      int sampleCount,
                                                                      GrMipmapped mipmapped,
                                                                      GrProtected isProtected,
                                                                      SkBudgeted budgeted) {
    SkASSERT(renderable == GrRenderable::kYes || sampleCount == 1);
    if (this->abandoned()) {
        return nullptr;
    }
    sk_sp<GrTextureProxy> proxy = this->proxyProvider()->createProxy(format,
                                                                     info.dimensions(),
                                                                     renderable,
                                                                     sampleCount,
                                                                     mipmapped,
                                                                     fit,
                                                                     budgeted,
                                                                     isProtected);
    if (!proxy) {
        return nullptr;
    }

    GrSwizzle swizzle;
    if (info.colorType() != GrColorType::kUnknown && !this->caps()->isFormatCompressed(format)) {
        swizzle = this->caps()->getReadSwizzle(format, info.colorType());
    }

    GrSurfaceProxyView view(std::move(proxy), origin, swizzle);
    return this->makeSC(std::move(view), info.colorInfo());
}

std::unique_ptr<skgpu::SurfaceFillContext> GrRecordingContextPriv::makeSFC(GrImageInfo info,
                                                                           SkBackingFit fit,
                                                                           int sampleCount,
                                                                           GrMipmapped mipmapped,
                                                                           GrProtected isProtected,
                                                                           GrSurfaceOrigin origin,
                                                                           SkBudgeted budgeted) {

    auto format = this->caps()->getDefaultBackendFormat(info.colorType(), GrRenderable::kYes);
    if (!format.isValid()) {
        return nullptr;
    }
    sk_sp<GrTextureProxy> proxy = this->proxyProvider()->createProxy(format,
                                                                     info.dimensions(),
                                                                     GrRenderable::kYes,
                                                                     sampleCount,
                                                                     mipmapped,
                                                                     fit,
                                                                     budgeted,
                                                                     isProtected);
    if (!proxy) {
        return nullptr;
    }

    GrSwizzle readSwizzle = this->caps()->getReadSwizzle(format, info.colorType());
    GrSwizzle writeSwizzle = this->caps()->getWriteSwizzle(format, info.colorType());

    std::unique_ptr<skgpu::SurfaceFillContext> sfc = make_SFC_or_SDC(this->context(),
                                                                     std::move(proxy),
                                                                     readSwizzle,
                                                                     writeSwizzle,
                                                                     origin,
                                                                     info.colorInfo());
    if (sfc) {
        sfc->discard();
    }
    return sfc;
}

std::unique_ptr<skgpu::SurfaceFillContext> GrRecordingContextPriv::makeSFC(
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        SkISize dimensions,
        SkBackingFit fit,
        const GrBackendFormat& format,
        int sampleCount,
        GrMipmapped mipmapped,
        GrProtected isProtected,
        GrSwizzle readSwizzle,
        GrSwizzle writeSwizzle,
        GrSurfaceOrigin origin,
        SkBudgeted budgeted) {

    SkASSERT(!dimensions.isEmpty());
    SkASSERT(sampleCount >= 1);
    SkASSERT(format.isValid() && format.backend() == fContext->backend());

    sk_sp<GrTextureProxy> proxy = this->proxyProvider()->createProxy(format,
                                                                     dimensions,
                                                                     GrRenderable::kYes,
                                                                     sampleCount,
                                                                     mipmapped,
                                                                     fit,
                                                                     budgeted,
                                                                     isProtected);
    if (!proxy) {
        return nullptr;
    }

    GrImageInfo info(GrColorType::kUnknown, alphaType, std::move(colorSpace), dimensions);

    std::unique_ptr<skgpu::SurfaceFillContext> sfc = make_SFC_or_SDC(this->context(),
                                                                     std::move(proxy),
                                                                     readSwizzle,
                                                                     writeSwizzle,
                                                                     origin,
                                                                     info.colorInfo());
    if (sfc) {
        sfc->discard();
    }
    return sfc;
}

std::unique_ptr<skgpu::SurfaceFillContext> GrRecordingContextPriv::makeSFCWithFallback(
        GrImageInfo info,
        SkBackingFit fit,
        int sampleCount,
        GrMipmapped mipmapped,
        GrProtected isProtected,
        GrSurfaceOrigin origin,
        SkBudgeted budgeted) {

    const GrCaps* caps = this->caps();

    auto [ct, _] = caps->getFallbackColorTypeAndFormat(info.colorType(), sampleCount);
    if (ct == GrColorType::kUnknown) {
        return nullptr;
    }

    info = info.makeColorType(ct);
    return this->makeSFC(info,
                         fit,
                         sampleCount,
                         mipmapped,
                         isProtected,
                         origin,
                         budgeted);
}

std::unique_ptr<skgpu::SurfaceFillContext> GrRecordingContextPriv::makeSFCFromBackendTexture(
        GrColorInfo info,
        const GrBackendTexture& tex,
        int sampleCount,
        GrSurfaceOrigin origin,
        sk_sp<GrRefCntedCallback> releaseHelper) {

    SkASSERT(sampleCount > 0);

    if (info.colorType() == GrColorType::kUnknown) {
        return nullptr;
    }

    const GrBackendFormat& format = tex.getBackendFormat();
    if (!this->caps()->areColorTypeAndFormatCompatible(info.colorType(), format)) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy(this->proxyProvider()->wrapRenderableBackendTexture(
            tex, sampleCount, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
            std::move(releaseHelper)));
    if (!proxy) {
        return nullptr;
    }

    GrSwizzle readSwizzle  = this->caps()->getReadSwizzle (format, info.colorType());
    GrSwizzle writeSwizzle = this->caps()->getWriteSwizzle(format, info.colorType());

    return make_SFC_or_SDC(this->context(),
                           std::move(proxy),
                           readSwizzle,
                           writeSwizzle,
                           origin,
                           info);
}
