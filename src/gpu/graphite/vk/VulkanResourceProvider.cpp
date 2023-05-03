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
#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"
#include "src/gpu/graphite/vk/VulkanDescriptorSet.h"
#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"
#include "src/gpu/graphite/vk/VulkanSampler.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"

namespace skgpu::graphite {

namespace { // Anonymous namespace

void build_key_from_descriptor_list(
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

VkDescriptorSetLayout determine_descriptor_set_layout(
        const VulkanSharedContext* ctxt,
        DescTypeAndCount requestedDescriptors[kDescriptorTypeCount]) {

    VkDescriptorSetLayout layout;
    skia_private::STArray<kDescriptorTypeCount, VkDescriptorSetLayoutBinding> bindingLayouts;

    for (int i = 0, j = 0; i < kDescriptorTypeCount; i++) {
        if (requestedDescriptors[i].count != 0) {
            VkDescriptorSetLayoutBinding* layoutBinding = &bindingLayouts.at(j++);
            memset(layoutBinding, 0, sizeof(VkDescriptorSetLayoutBinding));
            layoutBinding->binding = 0;
            layoutBinding->descriptorType = ds_type_enum_to_vk_ds(requestedDescriptors[i].type);
            layoutBinding->descriptorCount = requestedDescriptors[i].count;
            // TODO: Obtain layout binding stage flags from visibility (vertex or shader)
            layoutBinding->stageFlags = 0;
            // TODO: Obtain immutable samplers from VulkanSamplers
            layoutBinding->pImmutableSamplers = nullptr;
        }
    }

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo;
    memset(&layoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.pNext = nullptr;
    layoutCreateInfo.flags = 0;
    layoutCreateInfo.bindingCount = bindingLayouts.size();
    layoutCreateInfo.pBindings = &bindingLayouts.front();

    VkResult result;
    VULKAN_CALL_RESULT(ctxt->interface(),
                       result,
                       CreateDescriptorSetLayout(ctxt->device(),
                                                 &layoutCreateInfo,
                                                 nullptr,
                                                 &layout));
    if (result != VK_SUCCESS) {
        // TODO: Free unused unsuccessful objects / allocations? Throw error instead of null handle?
        layout = VK_NULL_HANDLE;
    }
    return layout;
}

} // Anonymous namespace

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

VulkanDescriptorSet* VulkanResourceProvider::findOrCreateDescriptorSet(
        DescTypeAndCount requestedDescriptors[kDescriptorTypeCount]) {

    GraphiteResourceKey key;
    build_key_from_descriptor_list(requestedDescriptors, &key);
    if (!key.isValid()) {
        // TODO: Throw error.
    }

    // Search for a preexisting available descriptor set.
    VulkanDescriptorSet* descSet = static_cast<VulkanDescriptorSet*>(
            fResourceCache->findAndRefResource(key,skgpu::Budgeted::kNo));

    // If there is not a matching descriptor set already allocated within the resource cache, then
    // allocate sets with the needed layout and add them to the cache.
    if (!descSet) {
        auto pool = VulkanDescriptorPool::Make(this->vulkanSharedContext(), requestedDescriptors);
        VkDescriptorSetLayout layout = determine_descriptor_set_layout(this->vulkanSharedContext(),
                                                                       requestedDescriptors);

        for (int j = 0; j < VulkanDescriptorPool::kMaxNumSets ; j++) {
                fResourceCache->insertResource(VulkanDescriptorSet::Make(
                        this->vulkanSharedContext(), pool, &layout).get());
        }

        descSet = static_cast<VulkanDescriptorSet*>(
                fResourceCache->findAndRefResource(key,skgpu::Budgeted::kNo));
    }

    return descSet;
}
} // namespace skgpu::graphite
