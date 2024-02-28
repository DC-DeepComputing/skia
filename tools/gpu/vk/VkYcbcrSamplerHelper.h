/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VkYcbcrSamplerHelper_DEFINED
#define VkYcbcrSamplerHelper_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_VULKAN

#if defined(SK_GRAPHITE)

#include "include/gpu/graphite/BackendTexture.h"

namespace skgpu::graphite {
class Recorder;
}

#elif defined(SK_GANESH) // !SK_GRAPHITE

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/vk/GrVkTypes.h"

class GrDirectContext;
class GrVkGpu;

#endif // !SK_GRAPHITE && SK_GANESH

// This helper will create and hold data for a Vulkan YCbCr backend texture. This format is
// particularly interesting because its sampler is immutable.
class VkYcbcrSamplerHelper {
public:

#if defined(SK_GRAPHITE)
    VkYcbcrSamplerHelper(skgpu::graphite::Recorder*);

    const skgpu::graphite::BackendTexture& backendTexture() const { return fTexture; }
#elif defined(SK_GANESH)
    VkYcbcrSamplerHelper(GrDirectContext*);

    const GrBackendTexture& backendTexture() const { return fTexture; }
#endif

    ~VkYcbcrSamplerHelper();

    bool isYCbCrSupported();

    bool createBackendTexture(uint32_t width, uint32_t height);

    static int GetExpectedY(int x, int y, int width, int height);
    static std::pair<int, int> GetExpectedUV(int x, int y, int width, int height);

private:

#if defined(SK_GRAPHITE)
    skgpu::graphite::Recorder*      fRecorder;
    skgpu::graphite::BackendTexture fTexture;
#elif defined(SK_GANESH)
    GrVkGpu* vkGpu();

    GrDirectContext* fDContext;
    GrBackendTexture fTexture;
#endif

    VkImage fImage = VK_NULL_HANDLE;
    VkDeviceMemory fImageMemory = VK_NULL_HANDLE;
};

#endif // SK_VULKAN

#endif // VkYcbcrSamplerHelper_DEFINED
