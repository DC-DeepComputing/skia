/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanResourceProvider_DEFINED
#define skgpu_graphite_VulkanResourceProvider_DEFINED

#include "src/gpu/graphite/ResourceProvider.h"

#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"
#include <map>

namespace skgpu::graphite {

class VulkanCommandBuffer;
class VulkanSharedContext;

class VulkanResourceProvider final : public ResourceProvider {
public:
    VulkanResourceProvider(SharedContext* sharedContext, SingleOwner*);
    ~VulkanResourceProvider() override;

    sk_sp<Texture> createWrappedTexture(const BackendTexture&) override;

    // Finds or creates a compatible VulkanDescriptorPool for the requested types and counts.
    // The refcount is incremented and a pointer returned.
    // TODO: Currently this will just create a descriptor pool without holding onto a ref itself
    //       so we currently do not reuse them. Requires knowing if another draw is currently using
    //       the VkDescriptorPool, the ability to reset pools, and the ability to purge pools out
    //       of our cache of VkDescriptorPools.
    VulkanDescriptorPool* findOrCreateCompatibleDescriptorPool(
            std::map<VulkanDescriptorPool::DescriptorSetType, int> DsTypesAndCounts);

private:
    const VulkanSharedContext* vulkanSharedContext();

    sk_sp<GraphicsPipeline> createGraphicsPipeline(const RuntimeEffectDictionary*,
                                                   const GraphicsPipelineDesc&,
                                                   const RenderPassDesc&) override;
    sk_sp<ComputePipeline> createComputePipeline(const ComputePipelineDesc&) override;

    sk_sp<Texture> createTexture(SkISize, const TextureInfo&, skgpu::Budgeted) override;
    sk_sp<Buffer> createBuffer(size_t size, BufferType type, PrioritizeGpuReads) override;

    sk_sp<Sampler> createSampler(const SkSamplingOptions&,
                                 SkTileMode xTileMode,
                                 SkTileMode yTileMode) override;

    BackendTexture onCreateBackendTexture(SkISize dimensions, const TextureInfo&) override;
    void onDeleteBackendTexture(BackendTexture&) override {}
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanResourceProvider_DEFINED
