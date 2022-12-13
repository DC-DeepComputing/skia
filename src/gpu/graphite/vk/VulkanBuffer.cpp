/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanBuffer.h"

#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "src/gpu/graphite/vk/VulkanDescriptorSet.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/graphite/vk/VulkanResourceProvider.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/vk/VulkanMemory.h"

namespace skgpu::graphite {

static const VulkanDescriptorSet* make_uniform_desc_set(const VulkanSharedContext* context,
                                                        VkBuffer buffer,
                                                        size_t size,
                                                        const VulkanResourceProvider* rsrcProvider) {
    // const VulkanDescriptorSet* descriptorSet = rsrcProvider.getUniformDescriptorSet();
    // if (!descriptorSet) {
        return nullptr;
    // }

    // VkDescriptorBufferInfo bufferInfo;
    // memset(&bufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    // bufferInfo.buffer = buffer;
    // bufferInfo.offset = 0;
    // bufferInfo.range = size;

    // VkWriteDescriptorSet descriptorWrite;
    // memset(&descriptorWrite, 0, sizeof(VkWriteDescriptorSet));
    // descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    // descriptorWrite.pNext = nullptr;
    // descriptorWrite.dstSet = *descriptorSet->descriptorSet();
    // descriptorWrite.dstBinding = GrVkUniformHandler::kUniformBinding;
    // descriptorWrite.dstArrayElement = 0;
    // descriptorWrite.descriptorCount = 1;
    // descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // descriptorWrite.pImageInfo = nullptr;
    // descriptorWrite.pBufferInfo = &bufferInfo;
    // descriptorWrite.pTexelBufferView = nullptr;

    // GR_VK_CALL(gpu->vkInterface(),
    //            UpdateDescriptorSets(gpu->device(), 1, &descriptorWrite, 0, nullptr));
    // return descriptorSet;
}

sk_sp<Buffer> VulkanBuffer::Make(const VulkanSharedContext* sharedContext,
                                 size_t size,
                                 BufferType type,
                                 PrioritizeGpuReads prioritizeGpuReads,
                                 const VulkanResourceProvider* resourceProvider) {
    if (size <= 0) {
        return nullptr;
    }
    VkBuffer buffer;
    skgpu::VulkanAlloc alloc;

    // The only time we don't require mappable buffers is when we're on a device where gpu only
    // memory has faster reads on the gpu than memory that is also mappable on the cpu. Protected
    // memory always uses mappable buffers.
    bool requiresMappable = sharedContext->isProtected() == Protected::kYes ||
                            prioritizeGpuReads == PrioritizeGpuReads::kNo ||
                            !sharedContext->vulkanCaps().gpuOnlyBuffersMorePerformant();

    using BufferUsage = skgpu::VulkanMemoryAllocator::BufferUsage;
    BufferUsage allocUsage;

    // Create the buffer object
    VkBufferCreateInfo bufInfo;
    memset(&bufInfo, 0, sizeof(VkBufferCreateInfo));
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.flags = 0;
    bufInfo.size = size;

    // To support SkMesh buffer updates we make Vertex and Index buffers capable of being transfer
    // dsts.
    switch (type) {
        case BufferType::kVertex:
            bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            allocUsage = requiresMappable ? BufferUsage::kCpuWritesGpuReads : BufferUsage::kGpuOnly;
            break;
        case BufferType::kIndex:
            bufInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            allocUsage = requiresMappable ? BufferUsage::kCpuWritesGpuReads : BufferUsage::kGpuOnly;
            break;
        case BufferType::kXferCpuToGpu:
            bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            allocUsage = BufferUsage::kTransfersFromCpuToGpu;
            break;
        case BufferType::kXferGpuToCpu:
            bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            allocUsage = BufferUsage::kTransfersFromGpuToCpu;
            break;
        case BufferType::kUniform:
            bufInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            allocUsage = BufferUsage::kCpuWritesGpuReads;
            break;
        case BufferType::kStorage:
            bufInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            allocUsage = BufferUsage::kCpuWritesGpuReads;
            break;
    }

    // We may not always get a mappable buffer for non-dynamic access buffers. Thus we set the
    // transfer dst usage bit in case we need to do a copy to write data. It doesn't really hurt
    // to set this extra usage flag, but we could narrow the scope of buffers we set it on more than
    // just not dynamic.
    if (!requiresMappable) {
        bufInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufInfo.queueFamilyIndexCount = 0;
    bufInfo.pQueueFamilyIndices = nullptr;

    VkResult result;
    VULKAN_CALL_RESULT(sharedContext->interface(), result, CreateBuffer(sharedContext->device(),
                                                           &bufInfo,
                                                           nullptr, /*const VkAllocationCallbacks*/
                                                           &buffer));
    if (result != VK_SUCCESS) {
        return nullptr;
    }

    auto allocator = sharedContext->memoryAllocator();
    bool shouldPersistentlyMapCpuToGpu =
        sharedContext->vulkanCaps().shouldPersistentlyMapCpuToGpuBuffers();
    //AllocBufferMemory
    if (!skgpu::VulkanMemory::AllocBufferMemory(allocator,
                                                buffer,
                                                allocUsage,
                                                shouldPersistentlyMapCpuToGpu,
                                                /*const std::function<CheckResult>&=*/nullptr,
                                                &alloc)) {
        VULKAN_CALL(sharedContext->interface(), DestroyBuffer(sharedContext->device(),
                buffer,
                /*const VkAllocationCallbacks*=*/nullptr));
        return nullptr;
    }

    // Bind buffer
    VULKAN_CALL_RESULT(sharedContext->interface(), result, BindBufferMemory(sharedContext->device(),
                                                                            buffer,
                                                                            alloc.fMemory,
                                                                            alloc.fOffset));
    if (result != VK_SUCCESS) {
        skgpu::VulkanMemory::FreeBufferMemory(allocator, alloc);
        VULKAN_CALL(sharedContext->interface(), DestroyBuffer(sharedContext->device(),
                buffer,
                /*const VkAllocationCallbacks*=*/nullptr));
        return nullptr;
    }

    // If this is a uniform buffer, we must set up a descriptor set. TODO - Move upwards
    const VulkanDescriptorSet* uniformDescSet = nullptr;
    if (type == BufferType::kUniform) {
        uniformDescSet = make_uniform_desc_set(sharedContext, buffer, size, resourceProvider);
        if (!uniformDescSet) {
            VULKAN_CALL(sharedContext->interface(), DestroyBuffer(sharedContext->device(),
                    buffer,
                    /*const VkAllocationCallbacks*=*/nullptr));
            skgpu::VulkanMemory::FreeBufferMemory(allocator, alloc);
            return nullptr;
        }
    }

    return sk_sp<Buffer>(new VulkanBuffer(sharedContext,
                                          size,
                                          type,
                                          prioritizeGpuReads,
                                          std::move(buffer),
                                          uniformDescSet));
}

VulkanBuffer::VulkanBuffer(const VulkanSharedContext* sharedContext,
                           size_t size,
                           BufferType type,
                           PrioritizeGpuReads prioritizeGpuReads,
                           VkBuffer buffer,
                           const VulkanDescriptorSet* uniformDescriptorSet)
        : Buffer(sharedContext, size, type, prioritizeGpuReads)
        , fBuffer(std::move(buffer))
        , fUniformDescriptorSet(uniformDescriptorSet) {}

} // namespace skgpu::graphite

