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
            layoutBinding->descriptorType =
                    VulkanDescriptorSet::dsTypeEnumToVkDs(requestedDescriptors[i].type);
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
        SkDebugf("Failed to create VkDescriptorSetLayout\n");
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
    // Fow now, have each descriptor type and quantity take up an entire uint32_t. 1 additional
    // uint32_t added to include a unique identifier for the different descriptor sets w/ the same
    // layout. This is horrendously inefficient but functional. TODO: Optimize key structure.
    static const int kNum32DataCnt = (kDescriptorTypeCount * 2) + 1;
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();
    // TODO: Can descriptor sets be shared? I think not.
    GraphiteResourceKey::Builder builder(&key, kType, kNum32DataCnt, Shareable::kNo);

    // Assemble the base component of a descriptor set key which is determined by the type and
    // quantity of descriptors.
    for (int i = 0; i < kDescriptorTypeCount; i++) {
        builder[i++] = static_cast<uint32_t>(requestedDescriptors[i].type);
        builder[i] = requestedDescriptors[i - 1].count;
    }

    // Build the last part of the key, starting a search for an available descriptor set at index 0
    // and finally determine if a set is not existent in the cache up to kMaxNumSets. TODO: Optimize
    // by having the first desc set of a given layout and checking that first. If it doesn't exist,
    // then act accordingly.
    Resource* descSet = nullptr;
    for (uint32_t i = 0; i < VulkanDescriptorPool::kMaxNumSets; i++) {
        builder[kNum32DataCnt - 1] = i;
        // Search for a preexisting available descriptor set.
        if ((descSet = fResourceCache->findAndRefResource(key, skgpu::Budgeted::kNo))) {
            // A non-null resource pointer indicates we have found an available descriptor set.
            break;
        }
    }

    if (descSet) {
        builder.finish();
    } else {
        // If we did not find a desc set, create them here.
        auto pool = VulkanDescriptorPool::Make(this->vulkanSharedContext(), requestedDescriptors);
        VkDescriptorSetLayout layout = determine_descriptor_set_layout(this->vulkanSharedContext(),
                                                                        requestedDescriptors);

        // Store the key of the first descriptor set so it can be easily accessed later.
        GraphiteResourceKey firstSetKey;
        for (int j = 0; j < VulkanDescriptorPool::kMaxNumSets ; j++) {
            descSet = VulkanDescriptorSet::Make(this->vulkanSharedContext(), pool, &layout).get();
            builder[kNum32DataCnt - 1] = j;
            descSet->setKey(key);
            fResourceCache->insertResource(descSet);
            if (j == 0) {
                firstSetKey = key;
            }
        }
        descSet = fResourceCache->findAndRefResource(firstSetKey, skgpu::Budgeted::kNo);
        builder.finish();
    }
    return descSet ? static_cast<VulkanDescriptorSet*>(descSet) : nullptr;
}
} // namespace skgpu::graphite
