/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnMtlQueueManager.h"

#include "src/gpu/graphite/GpuWorkSubmission.h"
#include "src/gpu/graphite/dawn/DawnMtlSharedContext.h"
#include "src/gpu/graphite/mtl/MtlCommandBuffer.h"

namespace skgpu::graphite {

DawnMtlQueueManager::DawnMtlQueueManager(sk_cfp<id<MTLCommandQueue>> queue,
                                         const SharedContext* sharedContext)
: MtlQueueManager(std::move(queue), sharedContext) {
}

QueueManager::OutstandingSubmission DawnMtlQueueManager::onSubmitToGpu(const SubmitInfo& info) {
    //this->dawnMtlSharedContext()->dawnDevice().AboutToCommitMtlCommandBuffer();
    return MtlQueueManager::onSubmitToGpu(info);
}

const DawnMtlSharedContext* DawnMtlQueueManager::dawnMtlSharedContext() const {
    return static_cast<const DawnMtlSharedContext*>(fSharedContext);
}

} // namespace skgpu::graphite
