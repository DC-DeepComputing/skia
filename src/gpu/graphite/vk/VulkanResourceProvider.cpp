/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanResourceProvider.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/vk/VulkanBuffer.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"
#include "src/gpu/graphite/vk/VulkanDescriptorSet.h"
#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"
#include "src/gpu/graphite/vk/VulkanSampler.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"

namespace skgpu::graphite {

VulkanResourceProvider::VulkanResourceProvider(SharedContext* sharedContext,
                                               SingleOwner* singleOwner,
                                               uint32_t recorderID)
        : ResourceProvider(sharedContext, singleOwner, recorderID) {}

VulkanResourceProvider::~VulkanResourceProvider() {}

const VulkanSharedContext* VulkanResourceProvider::vulkanSharedContext() {
    return static_cast<const VulkanSharedContext*>(fSharedContext);
}

sk_sp<Texture> VulkanResourceProvider::createWrappedTexture(const BackendTexture&) {
    return nullptr;
}

sk_sp<GraphicsPipeline> VulkanResourceProvider::createGraphicsPipeline(
        const RuntimeEffectDictionary*,
        const GraphicsPipelineDesc&,
        const RenderPassDesc&) {
    // TODO: Generate shaders and depth-stencil state

    return VulkanGraphicsPipeline::Make(this->vulkanSharedContext());
}

sk_sp<ComputePipeline> VulkanResourceProvider::createComputePipeline(const ComputePipelineDesc&) {
    return nullptr;
}

sk_sp<Texture> VulkanResourceProvider::createTexture(SkISize size, const TextureInfo& info,
                                                     skgpu::Budgeted budgeted) {
    return VulkanTexture::Make(this->vulkanSharedContext(), size, info, budgeted);
}

sk_sp<Buffer> VulkanResourceProvider::createBuffer(size_t size,
                                                   BufferType type,
                                                   AccessPattern accessPattern) {
    return VulkanBuffer::Make(this->vulkanSharedContext(), size, type, accessPattern);
}

sk_sp<Sampler> VulkanResourceProvider::createSampler(const SkSamplingOptions& samplingOptions,
                                                     SkTileMode xTileMode,
                                                     SkTileMode yTileMode) {
    return VulkanSampler::Make(this->vulkanSharedContext(), samplingOptions, xTileMode, yTileMode);
}

BackendTexture VulkanResourceProvider::onCreateBackendTexture(SkISize dimensions,
                                                              const TextureInfo&) {
    return {};
}

sk_sp<VulkanDescriptorSet> VulkanResourceProvider::findOrCreateDescriptorSet(
        DescTypeAndCount requestedDescriptors[kDescriptorTypeCount]) {
    GraphiteResourceKey key;
    this->buildKeyFromDescriptorList(requestedDescriptors, &key);
    VulkanDescriptorSet* descSet = static_cast<VulkanDescriptorSet*>(
            fResourceCache->findAndRefResource(key,skgpu::Budgeted::kNo));
    // TODO: If no matching resource (descriptor set) is available, create the a matching
    // descriptor pool and allocate the max number of sets from that pool. Add them to cache.
    return nullptr;
}

void VulkanResourceProvider::buildKeyFromDescriptorList(
        const DescTypeAndCount descriptors [kDescriptorTypeCount], GraphiteResourceKey* key) {
    // Fow now, have each descriptor type and quantity take up an entire uint32_t. This is
    // inefficient. TODO: Optimize key structure.
    static const int kNum32DataCnt = (kDescriptorTypeCount * 2);
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();
    // TODO: Can descriptor sets be shared? I think not.
    GraphiteResourceKey::Builder builder(key, kType, kNum32DataCnt, Shareable::kNo);

    for (int i = 0; i < kDescriptorTypeCount; i++) {
        builder[i++] = static_cast<uint32_t>(descriptors[i].type);
        builder[i] = descriptors[i - 1].count;
    }
}

} // namespace skgpu::graphite
