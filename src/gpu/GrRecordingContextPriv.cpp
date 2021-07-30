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
#endif
#if SK_GPU_V2
#include "src/gpu/GrSurfaceFillContext.h"
#include "src/gpu/v2/Device_v2.h"
#endif

sk_sp<skgpu::BaseDevice> GrRecordingContextPriv::createDevice(GrColorType colorType,
                                                              sk_sp<GrSurfaceProxy> proxy,
                                                              sk_sp<SkColorSpace> colorSpace,
                                                              GrSurfaceOrigin origin,
                                                              const SkSurfaceProps& props,
                                                              skgpu::BaseDevice::InitContents init) {
#if GR_TEST_UTILS
    if (this->options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
        return skgpu::v2::Device::Make(fContext, colorType, std::move(proxy), std::move(colorSpace),
                                       origin, props, init);
#else
        return nullptr;
#endif
    } else
#endif
    {
#if SK_GPU_V1
        return skgpu::v1::Device::Make(fContext, colorType, std::move(proxy), std::move(colorSpace),
                                       origin, props, init);
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
        return skgpu::v2::Device::Make(fContext, budgeted, ii, fit, sampleCount,
                                       mipmapped, isProtected, origin, props, init);
#else
        return nullptr;
#endif
    } else
#endif
    {
#if SK_GPU_V1
        return skgpu::v1::Device::Make(fContext, budgeted, ii, fit, sampleCount,
                                       mipmapped, isProtected, origin, props, init);
#else
        return nullptr;
#endif
    }
}

void GrRecordingContextPriv::moveRenderTasksToDDL(SkDeferredDisplayList* ddl) {
    fContext->drawingManager()->moveRenderTasksToDDL(ddl);
}

GrSDFTControl GrRecordingContextPriv::getSDFTControl(bool useSDFTForSmallText) const {
    return GrSDFTControl{
            this->caps()->shaderCaps()->supportsDistanceFieldText(),
            useSDFTForSmallText,
            this->options().fMinDistanceFieldFontSize,
            this->options().fGlyphsAsPathsFontSize};
}

std::unique_ptr<GrSurfaceFillContext> GrRecordingContextPriv::makeRawSDC(GrRecordingContext* rContext,
                                                                         GrSurfaceProxyView readView,
                                                                         GrSurfaceProxyView writeView,
                                                                         GrColorType colorType,
                                                                         sk_sp<SkColorSpace> cs,
                                                                         const SkSurfaceProps& props,
                                                                         bool flushTimeOpsTask) {
    std::unique_ptr<GrSurfaceFillContext> sfc;

    if (this->options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
#endif
    } else {
#if SK_GPU_V1
        sfc = std::make_unique<skgpu::v1::SurfaceDrawContext>(rContext,
                                                              std::move(readView),
                                                              std::move(writeView),
                                                              colorType,
                                                              std::move(cs),
                                                              props,
                                                              flushTimeOpsTask);
#endif
    }

    return sfc;
}

std::unique_ptr<GrSurfaceFillContext> GrRecordingContextPriv::makeSFC(GrImageInfo info,
                                                                      SkBackingFit fit,
                                                                      int sampleCount,
                                                                      GrMipmapped mipmapped,
                                                                      GrProtected isProtected,
                                                                      GrSurfaceOrigin origin,
                                                                      SkBudgeted budgeted) {

    if (this->options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
        return nullptr;
#endif
    } else {
#if SK_GPU_V1
        if (info.alphaType() == kPremul_SkAlphaType || info.alphaType() == kOpaque_SkAlphaType) {
            return skgpu::v1::SurfaceDrawContext::Make(fContext,
                                                       info.colorType(),
                                                       info.refColorSpace(),
                                                       fit,
                                                       info.dimensions(),
                                                       SkSurfaceProps(),
                                                       sampleCount,
                                                       mipmapped,
                                                       isProtected,
                                                       origin,
                                                       budgeted);
        }
        GrBackendFormat format = this->caps()->getDefaultBackendFormat(info.colorType(),
                                                                       GrRenderable::kYes);
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
        GrSwizzle readSwizzle  = this->caps()->getReadSwizzle (format, info.colorType());
        GrSwizzle writeSwizzle = this->caps()->getWriteSwizzle(format, info.colorType());

        GrSurfaceProxyView readView(            proxy, origin,  readSwizzle);
        GrSurfaceProxyView writeView(std::move(proxy), origin, writeSwizzle);
        auto fillContext = std::make_unique<GrSurfaceFillContext>(fContext,
                                                                  std::move(readView),
                                                                  std::move(writeView),
                                                                  info.colorInfo());
        fillContext->discard();
        return fillContext;
#endif
    }

    return nullptr;
}

std::unique_ptr<GrSurfaceFillContext> GrRecordingContextPriv::makeSFC(SkAlphaType alphaType,
                                                                      sk_sp<SkColorSpace> cs,
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

    if (fContext->options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
        return nullptr;
#endif
    } else {
#if SK_GPU_V1
        SkASSERT(!dimensions.isEmpty());
        SkASSERT(sampleCount >= 1);
        SkASSERT(format.isValid() && format.backend() == fContext->backend());
        if (alphaType == kPremul_SkAlphaType || alphaType == kOpaque_SkAlphaType) {
            return skgpu::v1::SurfaceDrawContext::Make(fContext,
                                                       std::move(cs),
                                                       fit,
                                                       dimensions,
                                                       format,
                                                       sampleCount,
                                                       mipmapped,
                                                       isProtected,
                                                       readSwizzle,
                                                       writeSwizzle,
                                                       origin,
                                                       budgeted,
                                                       SkSurfaceProps());
        }

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
        GrImageInfo info(GrColorType::kUnknown, alphaType, std::move(cs), dimensions);
        GrSurfaceProxyView readView(            proxy, origin,  readSwizzle);
        GrSurfaceProxyView writeView(std::move(proxy), origin, writeSwizzle);
        auto fillContext = std::make_unique<GrSurfaceFillContext>(fContext,
                                                                  std::move(readView),
                                                                  std::move(writeView),
                                                                  info.colorInfo());
        fillContext->discard();
        return fillContext;
#endif
    }

    return nullptr;
}

std::unique_ptr<GrSurfaceFillContext> GrRecordingContextPriv::makeSFCWithFallback(
        GrImageInfo info,
        SkBackingFit fit,
        int sampleCount,
        GrMipmapped mipmapped,
        GrProtected isProtected,
        GrSurfaceOrigin origin,
        SkBudgeted budgeted) {

    if (this->options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
        return nullptr;
#endif
    } else {
#if SK_GPU_V1
        if (info.alphaType() == kPremul_SkAlphaType || info.alphaType() == kOpaque_SkAlphaType) {
            return skgpu::v1::SurfaceDrawContext::MakeWithFallback77(fContext,
                                                                     info.colorType(),
                                                                     info.refColorSpace(),
                                                                     fit,
                                                                     info.dimensions(),
                                                                     SkSurfaceProps(),
                                                                     sampleCount,
                                                                     mipmapped,
                                                                     isProtected,
                                                                     origin,
                                                                     budgeted);
        }
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
#endif
    }

    return nullptr;
}

std::unique_ptr<GrSurfaceFillContext> GrRecordingContextPriv::makeSDCFromBackendTexture(
                                            GrRecordingContext* rContext,
                                            GrColorType colorType,
                                            sk_sp<SkColorSpace> colorSpace,
                                            const GrBackendTexture& tex,
                                            int sampleCount,
                                            GrSurfaceOrigin origin,
                                            const SkSurfaceProps& props,
                                            sk_sp<GrRefCntedCallback> releaseHelper) {
    std::unique_ptr<GrSurfaceFillContext> sfc;

    if (this->options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
#endif
    } else {
#if SK_GPU_V1
        sfc = skgpu::v1::SurfaceDrawContext::MakeFromBackendTexture(rContext,
                                                                    colorType,
                                                                    std::move(colorSpace),
                                                                    tex,
                                                                    sampleCount,
                                                                    origin,
                                                                    props,
                                                                    std::move(releaseHelper));
#endif
    }

    return sfc;
}
