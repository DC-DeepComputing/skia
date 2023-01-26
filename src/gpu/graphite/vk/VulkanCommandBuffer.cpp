/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"

#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

std::unique_ptr<VulkanCommandBuffer> VulkanCommandBuffer::Make(
        const VulkanSharedContext* sharedContext,
        VulkanResourceProvider* resourceProvider) {
    // Create VkCommandPool
    VkCommandPoolCreateFlags cmdPoolCreateFlags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    if (sharedContext->isProtected() == Protected::kYes) {
        cmdPoolCreateFlags |= VK_COMMAND_POOL_CREATE_PROTECTED_BIT;
    }

    const VkCommandPoolCreateInfo cmdPoolInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,  // sType
        nullptr,                                     // pNext
        cmdPoolCreateFlags,                          // CmdPoolCreateFlags
        sharedContext->queueIndex(),                 // queueFamilyIndex
    };
    auto interface = sharedContext->interface();
    VkResult result;
    VkCommandPool pool;
    VULKAN_CALL_RESULT(interface, result, CreateCommandPool(sharedContext->device(),
                                                            &cmdPoolInfo,
                                                            nullptr,
                                                            &pool));
    if (result != VK_SUCCESS) {
        return nullptr;
    }

    const VkCommandBufferAllocateInfo cmdInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // sType
        nullptr,                                          // pNext
        pool,                                             // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                  // level
        1                                                 // bufferCount
    };

    VkCommandBuffer primaryCmdBuffer;
    VULKAN_CALL_RESULT(interface, result, AllocateCommandBuffers(sharedContext->device(),
                                                                 &cmdInfo,
                                                                 &primaryCmdBuffer));
    if (result != VK_SUCCESS) {
        VULKAN_CALL(interface, DestroyCommandPool(sharedContext->device(), pool, nullptr));
        return nullptr;
    }

    return std::unique_ptr<VulkanCommandBuffer>(new VulkanCommandBuffer(pool,
                                                                        primaryCmdBuffer,
                                                                        sharedContext,
                                                                        resourceProvider));
}

VulkanCommandBuffer::VulkanCommandBuffer(VkCommandPool pool,
                                         VkCommandBuffer primaryCommandBuffer,
                                         const VulkanSharedContext* sharedContext,
                                         VulkanResourceProvider* resourceProvider)
        : fPool(pool)
        , fPrimaryCommandBuffer(primaryCommandBuffer)
        , fSharedContext(sharedContext)
        , fResourceProvider(resourceProvider) {

    // TODO: Remove these lines. They are only here to hide compiler warnings/errors about unused
    // member variables.
    (void) fPool;
    (void) fPrimaryCommandBuffer;
    (void) fSharedContext;
    (void) fResourceProvider;
    // When making a new command buffer, we automatically begin the command buffer
    this->begin();
}

VulkanCommandBuffer::~VulkanCommandBuffer() {}

void VulkanCommandBuffer::onResetCommandBuffer() {
    SkASSERT(!fActive);
    VULKAN_CALL_ERRCHECK(fSharedContext->interface(), ResetCommandPool(fSharedContext->device(),
                                                                       fPool,
                                                                       0));
}

bool VulkanCommandBuffer::setNewCommandBufferResources() {
    this->begin();
    return true;
}

void VulkanCommandBuffer::begin() {
    SkASSERT(!fActive);
    VkCommandBufferBeginInfo cmdBufferBeginInfo;
    memset(&cmdBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
    cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferBeginInfo.pNext = nullptr;
    cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmdBufferBeginInfo.pInheritanceInfo = nullptr;

    VULKAN_CALL_ERRCHECK(fSharedContext->interface(), BeginCommandBuffer(fPrimaryCommandBuffer,
                                                                         &cmdBufferBeginInfo));
    SkDEBUGCODE(fActive = true;)
}

void VulkanCommandBuffer::end() {
    SkASSERT(fActive);

    VULKAN_CALL_ERRCHECK(fSharedContext->interface(), EndCommandBuffer(fPrimaryCommandBuffer));

    SkDEBUGCODE(fActive = false;)
}

static bool submit_to_queue(const VulkanInterface* interface,
                            VkQueue queue,
                            VkFence fence,
                            uint32_t waitCount,
                            const VkSemaphore* waitSemaphores,
                            const VkPipelineStageFlags* waitStages,
                            uint32_t commandBufferCount,
                            const VkCommandBuffer* commandBuffers,
                            uint32_t signalCount,
                            const VkSemaphore* signalSemaphores,
                            Protected protectedContext) {
    VkProtectedSubmitInfo protectedSubmitInfo;
    if (protectedContext == Protected::kYes) {
        memset(&protectedSubmitInfo, 0, sizeof(VkProtectedSubmitInfo));
        protectedSubmitInfo.sType = VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO;
        protectedSubmitInfo.pNext = nullptr;
        protectedSubmitInfo.protectedSubmit = VK_TRUE;
    }

    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(VkSubmitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = protectedContext == Protected::kYes ? &protectedSubmitInfo : nullptr;
    submitInfo.waitSemaphoreCount = waitCount;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = commandBufferCount;
    submitInfo.pCommandBuffers = commandBuffers;
    submitInfo.signalSemaphoreCount = signalCount;
    submitInfo.pSignalSemaphores = signalSemaphores;
    VkResult result;
    VULKAN_CALL_RESULT(interface, result, QueueSubmit(queue, 1, &submitInfo, fence));
    if (result != VK_SUCCESS) {
        return false;
    }
    return true;
}

bool VulkanCommandBuffer::submit(VkQueue queue) {
    this->end();

    auto interface = fSharedContext->interface();
    auto device = fSharedContext->device();
    VkResult err;

    if (fSubmitFence == VK_NULL_HANDLE) {
        VkFenceCreateInfo fenceInfo;
        memset(&fenceInfo, 0, sizeof(VkFenceCreateInfo));
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VULKAN_CALL_RESULT(interface, err, CreateFence(device,
                                                       &fenceInfo,
                                                       nullptr,
                                                       &fSubmitFence));
        if (err) {
            fSubmitFence = VK_NULL_HANDLE;
            return false;
        }
    } else {
        // This cannot return DEVICE_LOST so we assert we succeeded.
        VULKAN_CALL_RESULT(interface, err, ResetFences(device, 1, &fSubmitFence));
        SkASSERT(err == VK_SUCCESS);
    }

    SkASSERT(fSubmitFence != VK_NULL_HANDLE);

    bool submitted = submit_to_queue(interface,
                                     queue,
                                     fSubmitFence,
                                     /*waitCount=*/0,
                                     /*waitSemaphores=*/nullptr,
                                     /*waitStages=*/nullptr,
                                     /*commandBufferCount*/1,
                                     &fPrimaryCommandBuffer,
                                     /*signalCount=*/0,
                                     /*signalSemaphores=*/nullptr,
                                     fSharedContext->isProtected());
    if (!submitted) {
        // Destroy the fence or else we will try to wait forever for it to finish.
        VULKAN_CALL(interface, DestroyFence(device, fSubmitFence, nullptr));
        fSubmitFence = VK_NULL_HANDLE;
        return false;
    }
    return true;
}

bool VulkanCommandBuffer::isFinished() {
    SkASSERT(!fActive);
    if (VK_NULL_HANDLE == fSubmitFence) {
        return true;
    }

    VkResult err;
    VULKAN_CALL_RESULT_NOCHECK(fSharedContext->interface(), err,
                               GetFenceStatus(fSharedContext->device(), fSubmitFence));
    switch (err) {
        case VK_SUCCESS:
        case VK_ERROR_DEVICE_LOST:
            return true;

        case VK_NOT_READY:
            return false;

        default:
            SKGPU_LOG_F("Error calling vkGetFenceStatus. Error: %d", err);
            SK_ABORT("Got an invalid fence status");
            return false;
    }
}

void VulkanCommandBuffer::waitUntilFinished() {
    if (fSubmitFence == VK_NULL_HANDLE) {
        return;
    }
    VULKAN_CALL_ERRCHECK(fSharedContext->interface(), WaitForFences(fSharedContext->device(),
                                                                    1,
                                                                    &fSubmitFence,
                                                                    /*waitAll=*/true,
                                                                    /*timeout=*/UINT64_MAX));
}

bool VulkanCommandBuffer::onAddRenderPass(
        const RenderPassDesc&,
        const Texture* colorTexture,
        const Texture* resolveTexture,
        const Texture* depthStencilTexture,
        SkRect viewport,
        const std::vector<std::unique_ptr<DrawPass>>& drawPasses) {
    return false;
}

bool VulkanCommandBuffer::onAddComputePass(const ComputePassDesc&,
                                           const ComputePipeline*,
                                           const std::vector<ResourceBinding>& bindings) {
    return false;
}

bool VulkanCommandBuffer::onCopyBufferToBuffer(const Buffer* srcBuffer,
                                               size_t srcOffset,
                                               const Buffer* dstBuffer,
                                               size_t dstOffset,
                                               size_t size) {
    return false;
}

bool VulkanCommandBuffer::onCopyTextureToBuffer(const Texture*,
                                                SkIRect srcRect,
                                                const Buffer*,
                                                size_t bufferOffset,
                                                size_t bufferRowBytes) {
    return false;
}

bool VulkanCommandBuffer::onCopyBufferToTexture(const Buffer* buffer,
                                                const Texture* texture,
                                                const BufferTextureCopyData* copyData,
                                                int count) {
    SkASSERT(!fActiveRenderCommandEncoder);
    SkASSERT(!fActiveComputeCommandEncoder);
    // this->addingWork(gpu);

    VkBuffer srcBuffer = static_cast<const VulkanBuffer*>(buffer)->vkBuffer();
    VulkanTexture* dstTexture = static_cast<const VulkanTexture*>(texture);
    VkImage dstTextureAsVkImage =  dstTexture->vkImage();

    // VkLayout uploadTexture->setImageLayout(this,
    //                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //                             VK_ACCESS_TRANSFER_WRITE_BIT,
    //                             VK_PIPELINE_STAGE_TRANSFER_BIT,
    //                             false);
//VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL

    // copyData info
    // size_t fBufferOffset;
    // size_t fBufferRowBytes;
    // SkIRect fRect;
    // unsigned int fMipLevel;

    /**
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkImage                                     dstImage,
        could also do getVkImage::getVkImage

    VkImageLayout                               dstImageLayout,
        backendtexture.h getVkImageLayout
    uint32_t                                    regionCount,
        mipLevel count
    const VkBufferImageCopy*                    pRegions);
        for each region (miplevel), set up a copy region
        // Set up copy region
        VkBufferImageCopy region;
        memset(&region, 0, sizeof(VkBufferImageCopy));
        region.bufferOffset = bufferOffset;
        region.bufferRowLength = (uint32_t)(rowBytes/bpp);
        region.bufferImageHeight = 0;
        region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        region.imageOffset = { rect.left(), rect.top(), 0 };
        region.imageExtent = { (uint32_t)rect.width(), (uint32_t)rect.height(), 1 };
        pointer to an array of VkBufferImageCopy structures specifying the regions to copy
    */
    VkBufferImageCopy region;
    memset(&region, 0, sizeof(VkBufferImageCopy));
    region.bufferOffset = copyData.fBufferOffset;
    region.bufferRowLength = (uint32_t)(copyData.fBufferRowBytes/bpp);
    region.bufferImageHeight = 0;
    region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.imageOffset = { copyData.fRect.left(), copyData.fRect.top(), 0 };
    region.imageExtent = { (uint32_t)copyData.fRect.width(), (uint32_t)copyData.fRect.height(), 1 };

    // From texture, should I obtain an image, and therefore a VkImage?
    VULKAN_CALL_RESULT(fSharedContext->interface(), CmdCopyBufferToImage(fPrimaryCommandBuffer,
                                                                         srcBuffer,
                                                                         dstTextureAsVkImage,
                                                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,//dstLayout,
                                                                         1, //copyRegionCount,
                                                                         &region));//copyRegions));
    if (result != VK_SUCCESS) {
        // Destroy resources?
        return false;
    }

    // this->addResource(dstImage->resource());
    return false;
}

bool VulkanCommandBuffer::onCopyTextureToTexture(const Texture* src,
                                                 SkIRect srcRect,
                                                 const Texture* dst,
                                                 SkIPoint dstPoint) {
    return false;
}

bool VulkanCommandBuffer::onSynchronizeBufferToCpu(const Buffer*, bool* outDidResultInWork) {
    return false;
}

bool VulkanCommandBuffer::onClearBuffer(const Buffer*, size_t offset, size_t size) {
    return false;
}

#ifdef SK_ENABLE_PIET_GPU
void VulkanCommandBuffer::onRenderPietScene(const skgpu::piet::Scene& scene,
                                            const Texture* target) {}
#endif

} // namespace skgpu::graphite

