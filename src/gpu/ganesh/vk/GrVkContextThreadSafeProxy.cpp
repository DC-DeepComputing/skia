/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/vk/GrVkContextThreadSafeProxy.h"

#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/private/base/SkAssert.h"
#include "include/private/chromium/GrSurfaceCharacterization.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/vk/GrVkCaps.h"

class SkSurfaceProps;
struct GrContextOptions;

GrVkContextThreadSafeProxy::GrVkContextThreadSafeProxy(const GrContextOptions& opts)
        : GrContextThreadSafeProxy(GrBackendApi::kVulkan, opts) {}

GrSurfaceCharacterization GrVkContextThreadSafeProxy::createCharacterization(
        size_t cacheMaxResourceBytes,
        const SkImageInfo& ii,
        const GrBackendFormat& backendFormat,
        int sampleCount,
        GrSurfaceOrigin origin,
        const SkSurfaceProps& surfaceProps,
        bool isMipMapped,
        bool willUseGLFBO0,
        bool isTextureable,
        GrProtected isProtected,
        bool vkRTSupportsInputAttachment,
        bool forVulkanSecondaryCommandBuffer) {
    SkASSERT(fCaps);
    SkASSERT(fBackend == GrBackendApi::kVulkan);
    if (!backendFormat.isValid()) {
        return {};
    }

    SkASSERT(isTextureable || !isMipMapped);

    if (willUseGLFBO0) {
        // The willUseGLFBO0 flags can only be used for a GL backend.
        return {};
    }

    if (!fCaps->mipmapSupport()) {
        isMipMapped = false;
    }

    if (ii.width() < 1 || ii.width() > fCaps->maxRenderTargetSize() || ii.height() < 1 ||
        ii.height() > fCaps->maxRenderTargetSize()) {
        return {};
    }

    GrColorType grColorType = SkColorTypeToGrColorType(ii.colorType());

    if (!fCaps->areColorTypeAndFormatCompatible(grColorType, backendFormat)) {
        return {};
    }

    if (!fCaps->isFormatAsColorTypeRenderable(grColorType, backendFormat, sampleCount)) {
        return {};
    }

    sampleCount = fCaps->getRenderTargetSampleCount(sampleCount, backendFormat);
    SkASSERT(sampleCount);

    if (willUseGLFBO0 && isTextureable) {
        return {};
    }

    if (isTextureable && !fCaps->isFormatTexturable(backendFormat, backendFormat.textureType())) {
        // Skia doesn't agree that this is textureable.
        return {};
    }

    if (forVulkanSecondaryCommandBuffer &&
        (isTextureable || isMipMapped || willUseGLFBO0 || vkRTSupportsInputAttachment)) {
        return {};
    }

    if (GrBackendApi::kVulkan != fBackend) {
        return {};
    }

    const GrVkCaps* vkCaps = (const GrVkCaps*)fCaps.get();
    // The protection status of the characterization and the context need to match
    if (isProtected != GrProtected(vkCaps->supportsProtectedContent())) {
        return {};
    }

    return GrSurfaceCharacterization(
            sk_ref_sp<GrContextThreadSafeProxy>(this),
            cacheMaxResourceBytes,
            ii,
            backendFormat,
            origin,
            sampleCount,
            GrSurfaceCharacterization::Textureable(isTextureable),
            GrSurfaceCharacterization::MipMapped(isMipMapped),
            GrSurfaceCharacterization::UsesGLFBO0(willUseGLFBO0),
            GrSurfaceCharacterization::VkRTSupportsInputAttachment(vkRTSupportsInputAttachment),
            GrSurfaceCharacterization::VulkanSecondaryCBCompatible(forVulkanSecondaryCommandBuffer),
            isProtected,
            surfaceProps);
}
