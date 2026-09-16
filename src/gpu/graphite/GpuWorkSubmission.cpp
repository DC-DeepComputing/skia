/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/GpuWorkSubmission.h"

#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/QueueManager.h"

namespace skgpu::graphite {

GpuWorkSubmission::GpuWorkSubmission(std::unique_ptr<CommandBuffer> cmdBuffer,
                                     QueueManager* queueManager)
        : GpuWorkSubmission(
                  [](std::unique_ptr<CommandBuffer> cb) {
                      skia_private::TArray<std::unique_ptr<CommandBuffer>> arr(1);
                      arr.emplace_back(std::move(cb));
                      return arr;
                  }(std::move(cmdBuffer)),
                  queueManager) {}

GpuWorkSubmission::GpuWorkSubmission(skia_private::TArray<std::unique_ptr<CommandBuffer>> cmdBuffers,
                                     QueueManager* queueManager)
        : fCommandBuffers(std::move(cmdBuffers))
        , fQueueManager(queueManager) {
    for (auto& cmdBuffer : fCommandBuffers) {
        if (!cmdBuffer) {
            continue;
        }
        SkSpan<const sk_sp<Buffer>> buffers = cmdBuffer->buffersToAsyncMapOnSubmit();
        if (!buffers.empty()) {
            if (!fOutstandingAsyncMapCounter) {
                fOutstandingAsyncMapCounter = sk_make_sp<SkRefCnt>();
            }
            for (auto& buffer : buffers) {
                SkASSERT(!buffer->isUnmappable());
                fOutstandingAsyncMapCounter->ref();
                buffer->asyncMap([](GpuFinishedContext c, CallbackResult) {
                                    static_cast<SkRefCnt*>(c)->unref();
                                 },
                                 fOutstandingAsyncMapCounter.get());
            }
        }
    }
}

GpuWorkSubmission::~GpuWorkSubmission() {
    for (auto& cmdBuffer : fCommandBuffers) {
        cmdBuffer->callFinishedProcs(/*success=*/true);
        cmdBuffer->resetCommandBuffer();
        fQueueManager->returnCommandBuffer(std::move(cmdBuffer));
    }
}

bool GpuWorkSubmission::isFinished(const SharedContext* sharedContext) {
    return this->onIsFinished(sharedContext) &&
           (!fOutstandingAsyncMapCounter || fOutstandingAsyncMapCounter->unique());
}

void GpuWorkSubmission::waitUntilFinished(const SharedContext* sharedContext) {
    this->onWaitUntilFinished(sharedContext);
    if (fOutstandingAsyncMapCounter) {
        while (!fOutstandingAsyncMapCounter->unique()) {
            fQueueManager->tick();
        }
    }
}

void GpuWorkSubmission::addFinishedProc(sk_sp<RefCntedCallback> finishedProc) {
    if (!fCommandBuffers.empty()) {
        fCommandBuffers.back()->addFinishedProc(std::move(finishedProc));
    }
}

} // namespace skgpu::graphite

