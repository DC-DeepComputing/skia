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

    // 1 uint32 needed for Vkformat, 2 for external format, and 1 to house remaining ycbcr info
    static const int kUint32sNeededForYcbcrInfo = 4;
    using YcbcrArray = skia_private::STArray<kUint32sNeededForYcbcrInfo, uint32_t>;

    // Gather all identifying YCbCr conversion information into compact numerical representation
    static YcbcrArray GetYcbcrConversionKeyInfo(const VulkanYcbcrConversionInfo& info);

    // Return a fully-formed GraphiteResourceKey that represents the conversion from conversion info
    static GraphiteResourceKey MakeYcbcrConversionKey(const VulkanSharedContext*,
                                                      const VulkanYcbcrConversionInfo&);

    // Return a fully-formed GraphiteResourceKey that represents the conversion from YCbCr key info
    // represented as a vector of integers.
    static GraphiteResourceKey MakeYcbcrConversionKeyFromInts(
            const std::vector<uint32_t>& ycbcrDataAsInts);

    VkSamplerYcbcrConversion ycbcrConversion() const { return fYcbcrConversion; }

    const char* getResourceType() const override { return "Vulkan Sampler YCbCr Conversion"; }


private:
    VulkanSamplerYcbcrConversion(const VulkanSharedContext*, VkSamplerYcbcrConversion);

    void freeGpuData() override;

    VkSamplerYcbcrConversion fYcbcrConversion;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanSamplerYcbcrConversion_DEFINED

