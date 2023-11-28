/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkContextThreadSafeProxy_DEFINED
#define GrVkContextThreadSafeProxy_DEFINED

#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrContextThreadSafeProxy.h"
#include "include/gpu/GrTypes.h"

#include <cstddef>

class GrBackendFormat;
class GrSurfaceCharacterization;
class SkSurfaceProps;
struct GrContextOptions;
struct SkImageInfo;

class GrVkContextThreadSafeProxy : public GrContextThreadSafeProxy {
public:
    GrSurfaceCharacterization createCharacterization(
            size_t cacheMaxResourceBytes,
            const SkImageInfo& ii,
            const GrBackendFormat& backendFormat,
            int sampleCount,
            GrSurfaceOrigin origin,
            const SkSurfaceProps& surfaceProps,
            bool isMipMapped,
            bool willUseGLFBO0 = false,
            bool isTextureable = true,
            GrProtected isProtected = GrProtected::kNo,
            bool vkRTSupportsInputAttachment = false,
            bool forVulkanSecondaryCommandBuffer = false) override;

    GrVkContextThreadSafeProxy(const GrContextOptions&);
};

#endif
