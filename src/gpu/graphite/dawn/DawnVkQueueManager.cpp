/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnVkQueueManager.h"

#include "src/gpu/graphite/GpuWorkSubmission.h"
#include "src/gpu/graphite/dawn/DawnVkSharedContext.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"

namespace skgpu::graphite {

DawnVkQueueManager::DawnVkQueueManager(VkQueue queue,
                                        const SharedContext* sharedContext)
    : VulkanQueueManager(queue, sharedContext) {
}

QueueManager::OutstandingSubmission DawnVkQueueManager::onSubmitToGpu(const SubmitInfo& info) {
    //this->dawnMtlSharedContext()->dawnDevice().AboutToCommitMtlCommandBuffer();
    return VulkanQueueManager::onSubmitToGpu(info);
}

const DawnVkSharedContext* DawnVkQueueManager::dawnVkSharedContext() const {
    return static_cast<const DawnVkSharedContext*>(fSharedContext);
}

} // namespace skgpu::graphite
