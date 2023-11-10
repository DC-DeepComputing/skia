/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/GpuWorkSubmission.h"

#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/QueueManager.h"

namespace skgpu::graphite {

GpuWorkSubmission::GpuWorkSubmission(std::unique_ptr<CommandBuffer> cmdBuffer,
                                     QueueManager* queueManager)
        : fCommandBuffer(std::move(cmdBuffer))
        , fQueueManager(queueManager) {
    for (auto& buffer : fCommandBuffer->buffersToAsyncMapOnSubmit()) {
        SkASSERT(!buffer->isMapped());
        buffer->asyncMap();
    }
}

GpuWorkSubmission::~GpuWorkSubmission() {
    fCommandBuffer->callFinishedProcs(/*success=*/true);
    fCommandBuffer->resetCommandBuffer();
    fQueueManager->returnCommandBuffer(std::move(fCommandBuffer));
}

bool GpuWorkSubmission::isFinished() {
    if (!this->onIsFinished()) {
        return false;
    }
    for (auto& buffer : fCommandBuffer->buffersToAsyncMapOnSubmit()) {
        if (!buffer->isMapped()) {
            SkASSERT(buffer->hasPendingAsyncMap());
            return false;
        }
    }
    return true;
}

void GpuWorkSubmission::waitUntilFinished() {
    this->onWaitUntilFinished();
    for (auto& buffer : fCommandBuffer->buffersToAsyncMapOnSubmit()) {
        SkASSERT(buffer->isMapped() || buffer->hasPendingAsyncMap());
        // Complete the async mapping if it hasn't completed already.
        buffer->map();
    }
}

} // namespace skgpu::graphite

