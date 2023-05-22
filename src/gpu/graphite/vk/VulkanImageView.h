/*
* Copyright 2023 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanImageView_DEFINED
#define skgpu_graphite_VulkanImageView_DEFINED

#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/graphite/Resource.h"

namespace skgpu::graphite {

class VulkanSharedContext;

class VulkanImageView : public Resource {
public:
    enum class Usage {
        kShaderInput,
        kAttachment
    };

    static sk_sp<const VulkanImageView> Make(const VulkanSharedContext* sharedContext,
                                             VkImage image,
                                             VkFormat format,
                                             Usage usage,
                                             uint32_t miplevels);

    VkImageView imageView() const { return fImageView; }
    Usage usage() const { return fUsage; }

private:
    VulkanImageView(const VulkanSharedContext* sharedContext, VkImageView imageView, Usage usage);

    void freeGpuData() override;

    VkImageView  fImageView;
    Usage fUsage;
    // TODO: track associated SamplerYcbcrConversion
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_VulkanImageView_DEFINED
