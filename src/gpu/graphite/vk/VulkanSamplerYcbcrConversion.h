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

namespace skgpu {
struct VulkanYcbcrConversionInfo;
}

namespace skgpu::graphite {

class VulkanSharedContext;

// TODO: Consider changing this class to not be a Resource that we cache, store, and share. It is
// relatively small and it may make more sense to simply have each texture that requires a
// conversion store its own conversion information even if it is duplicated.
class VulkanSamplerYcbcrConversion : public Resource {
public:
    static sk_sp<VulkanSamplerYcbcrConversion> Make(const VulkanSharedContext*,
                                                    const VulkanYcbcrConversionInfo&);

    // 1 uint32 needed for Vkformat, 2 for external format, and 1 to house remaining ycbcr info.
    // TODO: When using a known VkFormat, we only need 3 int32s because an external format is
    // stored as an int64 whereas a known VkFormat can be contained within an int32. Optimize for
    // this case.
    static const int kUint32sNeededForYcbcrInfo = 4;
    static uint16_t YcbcrConversionAsInt16(const VulkanYcbcrConversionInfo&);

    // Return a fully-formed GraphiteResourceKey that represents the conversion from conversion info
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

