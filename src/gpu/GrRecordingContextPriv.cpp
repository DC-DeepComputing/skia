/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrRecordingContextPriv.h"

#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDrawingManager.h"
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

std::unique_ptr<GrSurfaceFillContext> GrRecordingContextPriv::makeSDC2(
                                            GrRecordingContext* rContext,
                                            GrColorType colorType,
                                            sk_sp<SkColorSpace> colorSpace,
                                            SkBackingFit fit,
                                            SkISize dimensions,
                                            const SkSurfaceProps& props,
                                            int sampleCount,
                                            GrMipmapped mipmapped,
                                            GrProtected isProtected,
                                            GrSurfaceOrigin origin,
                                            SkBudgeted budgeted) {

    std::unique_ptr<GrSurfaceFillContext> sfc;

    if (this->options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
#endif
    } else {
#if SK_GPU_V1
        sfc = skgpu::v1::SurfaceDrawContext::Make(rContext,
                                                  colorType,
                                                  std::move(colorSpace),
                                                  fit,
                                                  dimensions,
                                                  props,
                                                  sampleCount,
                                                  mipmapped,
                                                  isProtected,
                                                  origin,
                                                  budgeted);
#endif
    }

    return sfc;
}

std::unique_ptr<GrSurfaceFillContext> GrRecordingContextPriv::makeSDC1(
                                            GrRecordingContext* rContext,
                                            sk_sp<SkColorSpace> colorSpace,
                                            SkBackingFit fit,
                                            SkISize dimensions,
                                            const GrBackendFormat& format,
                                            int sampleCount,
                                            GrMipmapped mipmapped,
                                            GrProtected isProtected,
                                            GrSwizzle readSwizzle,
                                            GrSwizzle writeSwizzle,
                                            GrSurfaceOrigin origin,
                                            SkBudgeted budgeted,
                                            const SkSurfaceProps& props) {
    std::unique_ptr<GrSurfaceFillContext> sfc;

    if (this->options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
#endif
    } else {
#if SK_GPU_V1
        sfc = skgpu::v1::SurfaceDrawContext::Make(rContext,
                                                  std::move(colorSpace),
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
                                                  props);
#endif
    }

    return sfc;
}

std::unique_ptr<GrSurfaceFillContext> GrRecordingContextPriv::makeSDCWithFallback(
                                            GrRecordingContext* rContext,
                                            GrColorType colorType,
                                            sk_sp<SkColorSpace> colorSpace,
                                            SkBackingFit fit,
                                            SkISize dimensions,
                                            const SkSurfaceProps& props,
                                            int sampleCount,
                                            GrMipmapped mipmapped,
                                            GrProtected isProtected,
                                            GrSurfaceOrigin origin,
                                            SkBudgeted budgeted) {
    std::unique_ptr<GrSurfaceFillContext> sfc;

    if (this->options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
#endif
    } else {
#if SK_GPU_V1
        sfc = skgpu::v1::SurfaceDrawContext::MakeWithFallback77(rContext,
                                                              colorType,
                                                              std::move(colorSpace),
                                                              fit,
                                                              dimensions,
                                                              props,
                                                              sampleCount,
                                                              mipmapped,
                                                              isProtected,
                                                              origin,
                                                              budgeted);
#endif
    }

    return sfc;
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
