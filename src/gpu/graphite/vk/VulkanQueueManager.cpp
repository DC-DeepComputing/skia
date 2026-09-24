/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanQueueManager.h"

#include "src/gpu/graphite/GpuWorkSubmission.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"
#include "src/gpu/graphite/vk/VulkanResourceProvider.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

VulkanQueueManager::VulkanQueueManager(VkQueue queue, VkQueue protectedQueue,
                                       const SharedContext* sharedContext)
        : QueueManager(sharedContext)
        , fQueue(queue)
        , fProtectedQueue(protectedQueue) {
}

const VulkanSharedContext* VulkanQueueManager::vkSharedContext() const {
    return static_cast<const VulkanSharedContext*>(fSharedContext);
}

std::unique_ptr<CommandBuffer> VulkanQueueManager::getNewCommandBuffer(
        ResourceProvider* resourceProvider, Protected isProtected) {
    VulkanResourceProvider* vkResourceProvider =
            static_cast<VulkanResourceProvider*>(resourceProvider);

    auto cmdBuffer = VulkanCommandBuffer::Make(this->vkSharedContext(),
                                               vkResourceProvider,
                                               isProtected);
    return cmdBuffer;
}

class VulkanWorkSubmission final : public GpuWorkSubmission {
public:
    VulkanWorkSubmission(skia_private::TArray<std::unique_ptr<CommandBuffer>> cmdBuffers,
                         QueueManager* queueManager)
        : GpuWorkSubmission(std::move(cmdBuffers), queueManager) {}
    VulkanWorkSubmission(std::unique_ptr<CommandBuffer> cmdBuffer, QueueManager* queueManager)
        : GpuWorkSubmission(std::move(cmdBuffer), queueManager) {}
    ~VulkanWorkSubmission() override {}

private:
    bool onIsFinished(const SharedContext*) override {
        return static_cast<VulkanCommandBuffer*>(this->commandBuffer())->isFinished();
    }
    void onWaitUntilFinished(const SharedContext*) override {
        return static_cast<VulkanCommandBuffer*>(this->commandBuffer())->waitUntilFinished();
    }
};

QueueManager::OutstandingSubmission VulkanQueueManager::onSubmitToGpu(const SubmitInfo& submitInfo) {
    skia_private::TArray<std::unique_ptr<CommandBuffer>> cmdBuffers;

    if (fCurrentCommandBuffer) {
        VulkanCommandBuffer* vkCmdBuffer =
                static_cast<VulkanCommandBuffer*>(fCurrentCommandBuffer.get());
        bool useFence = !fCurrentProtectedCommandBuffer;
        if (!vkCmdBuffer->submit(fQueue, submitInfo, useFence)) {
            SKIA_LOG_E("VulkanQueueManager::onSubmitToGpu - UNPROTECTED submission failed!");
            fCurrentCommandBuffer->callFinishedProcs(/*success=*/false);
            return nullptr;
        }
        cmdBuffers.emplace_back(std::move(fCurrentCommandBuffer));
    }

    if (fCurrentProtectedCommandBuffer) {
        VulkanCommandBuffer* vkCmdBuffer =
                static_cast<VulkanCommandBuffer*>(fCurrentProtectedCommandBuffer.get());
        if (!vkCmdBuffer->submit(fProtectedQueue, submitInfo, /*useFence=*/true)) {
            SKIA_LOG_E("VulkanQueueManager::onSubmitToGpu - PROTECTED submission failed!");
            fCurrentProtectedCommandBuffer->callFinishedProcs(/*success=*/false);
            return nullptr;
        }
        cmdBuffers.emplace_back(std::move(fCurrentProtectedCommandBuffer));
    }

    if (cmdBuffers.empty()) {
        SKIA_LOG_W("VulkanQueueManager::onSubmitToGpu - no command buffers to submit");
        return nullptr;
    }

    std::unique_ptr<GpuWorkSubmission> submission(
            new VulkanWorkSubmission(std::move(cmdBuffers), this));
    return submission;
}

} // namespace skgpu::graphite
