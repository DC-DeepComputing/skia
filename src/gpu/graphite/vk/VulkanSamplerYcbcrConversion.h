/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanSamplerYcbcrConversion_DEFINED
#define skgpu_graphite_VulkanSamplerYcbcrConversion_DEFINED

#include "src/gpu/graphite/Resource.h"

#include "include/private/base/SkTArray.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/core/SkChecksum.h"

#include <cinttypes>

namespace skgpu {
struct VulkanYcbcrConversionInfo;
}

namespace skgpu::graphite {

class VulkanSharedContext;

class VulkanSamplerYcbcrConversion : public Resource {
public:
    static sk_sp<VulkanSamplerYcbcrConversion> Make(const VulkanSharedContext*,
                                                    const VulkanYcbcrConversionInfo&);

    // Gather all identifying YCbCr conversion information into compact numerical representation
    static constexpr int kUint32sNeededForYcbcrInfo = 4;
    using YcbcrArray = skia_private::STArray<kUint32sNeededForYcbcrInfo, uint32_t>;
    static YcbcrArray GetYcbcrConversionKeyInfo(const VulkanYcbcrConversionInfo& info);

    // Return a fully-formed GraphiteResourceKey that represents the conversion
    static GraphiteResourceKey MakeYcbcrConversionKey(const VulkanSharedContext*,
                                                      const VulkanYcbcrConversionInfo&);

    VkSamplerYcbcrConversion ycbcrConversion() const { return fYcbcrConversion; }

    const char* getResourceType() const override { return "Vulkan Sampler YCbCr Conversion"; }

private:
    VulkanSamplerYcbcrConversion(const VulkanSharedContext*, VkSamplerYcbcrConversion);

    void freeGpuData() override;

    VkSamplerYcbcrConversion fYcbcrConversion;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanSamplerYcbcrConversion_DEFINED

