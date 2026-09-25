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
        : GpuWorkSubmission(std::move(cmdBuffer), /*protectedCmdBuffer=*/nullptr, queueManager) {}

GpuWorkSubmission::GpuWorkSubmission(std::unique_ptr<CommandBuffer> cmdBuffer,
                                     std::unique_ptr<CommandBuffer> protectedCmdBuffer,
                                     QueueManager* queueManager)
        : fCommandBuffer(std::move(cmdBuffer))
        , fProtectedCommandBuffer(std::move(protectedCmdBuffer))
        , fQueueManager(queueManager) {
    auto setupAsyncMaps = [this](CommandBuffer* cb) {
        if (!cb) {
            return;
        }
        SkSpan<const sk_sp<Buffer>> buffers = cb->buffersToAsyncMapOnSubmit();
        if (!buffers.empty()) {
            if (!fOutstandingAsyncMapCounter) {
                fOutstandingAsyncMapCounter = sk_make_sp<SkRefCnt>();
            }
            for (auto& buffer : buffers) {
                SkASSERT(!buffer->isUnmappable());
                fOutstandingAsyncMapCounter->ref();
                buffer->asyncMap([](GpuFinishedContext c, CallbackResult) {
                                     static_cast<SkRefCnt*>(c)->unref();
                                 }, fOutstandingAsyncMapCounter.get());
            }
        }
    };
    setupAsyncMaps(fCommandBuffer.get());
    setupAsyncMaps(fProtectedCommandBuffer.get());
}

GpuWorkSubmission::~GpuWorkSubmission() {
    auto cleanup = [this](std::unique_ptr<CommandBuffer>& cb) {
        if (cb) {
            cb->callFinishedProcs(/*success=*/true);
            cb->resetCommandBuffer();
            fQueueManager->returnCommandBuffer(std::move(cb));
        }
    };
    cleanup(fCommandBuffer);
    cleanup(fProtectedCommandBuffer);
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
    if (fProtectedCommandBuffer) {
        fProtectedCommandBuffer->addFinishedProc(std::move(finishedProc));
    } else if (fCommandBuffer) {
        fCommandBuffer->addFinishedProc(std::move(finishedProc));
    }
}

} // namespace skgpu::graphite

