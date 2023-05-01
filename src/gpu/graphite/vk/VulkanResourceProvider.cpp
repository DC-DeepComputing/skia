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
#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"
#include "src/gpu/graphite/vk/VulkanSampler.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"

namespace skgpu::graphite {
// TODO: Remove / relocate this.
namespace { // anonymous namespace
VkDescriptorType ds_type_enum_to_vk_ds(DescriptorType type) {
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
} // anonymous namespace

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
    // At this point, determine (or access already determined?) descriptor set layout
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

void VulkanResourceProvider::groupDescriptors(std::map<DescriptorType, int> dsTypesAndCounts) {
    // cache layouts; check if equivalent to past layout (compare maps)

    // Iterate through map and create DescriptorSetLayoutBinding structures for each unique
    // descriptor type with a quantitity > 0.

    // TODO: Could consolidate this perusal with similar functionality in
    // VulkanDescriptorPool::Make().
    skia_private::STArray<kDescriptorTypeCount, VkDescriptorSetLayoutBinding> bindingLayouts;
    int i = 0;
    for (auto descriptorType : dsTypesAndCounts) {
        VkDescriptorSetLayoutBinding* layoutBinding = &bindingLayouts.at(i++);
        memset(layoutBinding, 0, sizeof(VkDescriptorSetLayoutBinding));
        layoutBinding->binding = 0;
        layoutBinding->descriptorType = ds_type_enum_to_vk_ds(descriptorType.first);
        layoutBinding->descriptorCount = descriptorType.second;
        // TODO: Obtain layout binding stage flags from visibility (vertex or shader)
        layoutBinding->stageFlags = 0;
        // TODO: Obtain immutable samplers from VulkanSamplers
        layoutBinding->pImmutableSamplers = nullptr;
    }


    VkDescriptorSetLayoutCreateInfo layoutCreateInfo;
    memset(&layoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.pNext = nullptr;
    layoutCreateInfo.flags = 0;
    layoutCreateInfo.bindingCount = bindingLayouts.size();
    layoutCreateInfo.pBindings = &bindingLayouts.front();
}

// std::map<DescriptorType, int> VulkanResourceProvider::getDescriptorMapFromRenderPassDesc(
//         const RenderPassDesc& renderPassDesc) {
//     // TODO: Query Caps's resourceBindingRequirements for fSeparateTextureAndSamplerBinding.
//     // For now, assume combined texture/samplers since that's what we seem to do in Ganesh.
//     std::map<DescriptorType, int> dsTypesAndCounts;
//     if (renderPassDesc.fColorAttachment) {
//         dsTypesAndCounts.at()
//     }
//     if (renderPassDesc.fDepthStencilAttachment) {

//     }
//     if (renderPassDesc.fColorResolveAttachment) {

//     }
//     return dsTypesAndCounts;
//     // BindBufferInfo will say what kind of buffers are involved; slots; collate descriptors into
//     // sets.

//     // ContextUtils::ExtractRenderStepData to see uniforms and textures?
//     // How to determine kUniformBuffer? PipelineData::hasUniforms,
//     //     PipelineDataGatherer::hasUniforms?
//     // kCombinedTextureSampler,
//     // How to determine kStorageBuffer?
//     // How to determine kInputAttachment? -> image,
// }

} // namespace skgpu::graphite
