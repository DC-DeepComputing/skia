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
        // TODO: Delete unused structs?
        return nullptr;
    }
    return sk_sp<VulkanFrameBuffer>(new VulkanFrameBuffer(context, frameBuffer));
}

VulkanFrameBuffer::VulkanFrameBuffer(const VulkanSharedContext* context, VkFramebuffer frameBuffer)
        : fSharedContext(context)
        , fFrameBuffer(frameBuffer) {
}

VulkanFrameBuffer::~VulkanFrameBuffer() {
    VULKAN_CALL(fSharedContext->interface(),
                DestroyFramebuffer(fSharedContext->device(), fFrameBuffer, nullptr));
}

} // namespace skgpu::graphite
