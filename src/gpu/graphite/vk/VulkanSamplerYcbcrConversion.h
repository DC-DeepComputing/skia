/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanSamplerYcbcrConversion_DEFINED
#define skgpu_graphite_VulkanSamplerYcbcrConversion_DEFINED

#include "src/gpu/graphite/Resource.h"

#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/core/SkChecksum.h"

#include <cinttypes>

namespace skgpu::graphite {

class VulkanSharedContext;
struct VulkanYcbcrConversionInfo;

class VulkanSamplerYcbcrConversion : public Resource {
public:
    static VulkanSamplerYcbcrConversion* Make(const VulkanSharedContext*,
                                              const VulkanYcbcrConversionInfo&);

    VkSamplerYcbcrConversion ycbcrConversion() const { return fYcbcrConversion; }


private:
    VulkanSamplerYcbcrConversion(const VulkanSharedContext*, VkSamplerYcbcrConversion);

    void freeGpuData() override;

    VkSamplerYcbcrConversion fYcbcrConversion;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanSamplerYcbcrConversion_DEFINED

