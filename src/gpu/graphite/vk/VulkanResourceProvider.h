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
    VulkanResourceProvider(SharedContext* sharedContext, SingleOwner*, uint32_t recorderID);
    ~VulkanResourceProvider() override;

    sk_sp<Texture> createWrappedTexture(const BackendTexture&) override;

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

    // Creates a compatible descriptor pool for the requested types and counts. Does not hold on to
    // a ref itself since we do not reuse pools.
    sk_sp<VulkanDescriptorPool> createCompatibleDescriptorPool(
            std::map<VulkanDescriptorPool::DescriptorSetType, int> DsTypesAndCounts);

    BackendTexture onCreateBackendTexture(SkISize dimensions, const TextureInfo&) override;
    void onDeleteBackendTexture(BackendTexture&) override {}
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanResourceProvider_DEFINED
