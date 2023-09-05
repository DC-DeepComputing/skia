/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanFrameBuffer.h"

#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

sk_sp<VulkanFrameBuffer> VulkanFrameBuffer::Make(const VulkanSharedContext* context,
                                                 const VkFramebufferCreateInfo& frameBufferInfo) {
    VkFramebuffer frameBuffer;
    VkResult result;
    VULKAN_CALL_RESULT(context->interface(), result,
                       CreateFramebuffer(context->device(),
                                         &frameBufferInfo,
                                         nullptr,
                                         &frameBuffer));
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    return sk_sp<VulkanFrameBuffer>(new VulkanFrameBuffer(context, frameBuffer));
}

VulkanFrameBuffer::VulkanFrameBuffer(const VulkanSharedContext* context, VkFramebuffer frameBuffer)
        : Resource(context, Ownership::kOwned, skgpu::Budgeted::kNo, /*gpuMemorySize=*/0)
        , fSharedContext(context)
        , fFrameBuffer(frameBuffer) {
}

void VulkanFrameBuffer::freeGpuData() {
    VULKAN_CALL(fSharedContext->interface(),
                DestroyFramebuffer(fSharedContext->device(), fFrameBuffer, nullptr));
}

} // namespace skgpu::graphite
