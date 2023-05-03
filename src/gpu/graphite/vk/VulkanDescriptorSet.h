/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanDescriptorSet_DEFINED
#define skgpu_graphite_VulkanDescriptorSet_DEFINED

#include "src/gpu/graphite/Resource.h"

#include "src/gpu/graphite/Descriptors.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"

namespace skgpu::graphite {

class VulkanDescriptorPool;
class VulkanSharedContext;

static VkDescriptorType ds_type_enum_to_vk_ds(DescriptorType type) {
    switch (type) {
        case DescriptorType::kUniformBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescriptorType::kTextureSampler:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case DescriptorType::kTexture:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case DescriptorType::kCombinedTextureSampler:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case DescriptorType::kStorageBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescriptorType::kInputAttachment:
            return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    }
    SkUNREACHABLE;
}

/**
 * Wrapper around VkDescriptorSet which maintains a reference to its descriptor pool. Once the ref
 * count on that pool is 0, it will be destroyed.
*/
class VulkanDescriptorSet : public Resource {
public:
    static sk_sp<VulkanDescriptorSet> Make(const VulkanSharedContext*,
                                           sk_sp<VulkanDescriptorPool>,
                                           const VkDescriptorSetLayout*);

    VulkanDescriptorSet(const VulkanSharedContext*, VkDescriptorSet, sk_sp<VulkanDescriptorPool>);

    VkDescriptorSetLayout layout() const { return fDescLayout; }

    bool isAvailable() const { return fIsAvailable; }

    VkDescriptorSet descriptorSet() {
        SkASSERT(fIsAvailable);
        fIsAvailable = false;
        return fDescSet;
    }

    void setAvailability(bool isAvailable) {
        fIsAvailable = isAvailable;
    }

private:
    void freeGpuData() override;

    VkDescriptorSet             fDescSet;
    // Have this class hold on to a reference of the descriptor pool. When a pool's reference count
    // is 0, that means all the descriptor sets that came from that pool are no longer needed, so
    // the pool can safely be destroyed.
    sk_sp<VulkanDescriptorPool> fPool;
    // TODO: Are the below member attributes needed?
    bool                        fIsAvailable;
    VkDescriptorSetLayout       fDescLayout;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanDescriptorSet_DEFINED
