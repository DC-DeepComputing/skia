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

    // One uint32 for Vkformat, two for external format, and one to house all ycbcr information
    static const int kUint32sNeededForYcbcrInfo = 4;

    // Gather all identifying YCbCr conversion information into compact numerical representation
    using YcbcrArray = skia_private::STArray<kUint32sNeededForYcbcrInfo, uint32_t>;
    static YcbcrArray GetYcbcrConversionKeyInfo(const VulkanYcbcrConversionInfo& info);

    // Return a fully-formed GraphiteResourceKey that represents the conversion from conversion info
    static GraphiteResourceKey MakeYcbcrConversionKey(const VulkanSharedContext*,
                                                      const VulkanYcbcrConversionInfo&);

    // Return a fully-formed GraphiteResourceKey that represents the conversion from YCbCr key info
    // represented as a vector of integers.
    // TODO: expland upon this comment as to why we have this
    static GraphiteResourceKey MakeYcbcrConversionKeyFromInts(
            const std::vector<uint32_t>& ycbcrDataAsInts);

    // TODO: Consider a "add to existing builder/key" method

    VkSamplerYcbcrConversion ycbcrConversion() const { return fYcbcrConversion; }

    const char* getResourceType() const override { return "Vulkan Sampler YCbCr Conversion"; }

private:
    VulkanSamplerYcbcrConversion(const VulkanSharedContext*, VkSamplerYcbcrConversion);

    void freeGpuData() override;

    VkSamplerYcbcrConversion fYcbcrConversion;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanSamplerYcbcrConversion_DEFINED

