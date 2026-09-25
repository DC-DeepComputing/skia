/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_GpuWorkSubmission_DEFINED
#define skgpu_graphite_GpuWorkSubmission_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkTArray.h"

#include <memory>

namespace skgpu {
class RefCntedCallback;
}

namespace skgpu::graphite {
class CommandBuffer;
class SharedContext;
class QueueManager;

class GpuWorkSubmission {
public:
    static constexpr int kMaxCommandBuffers = 2;
    using SubmittedCommandBufferList =
            skia_private::STArray<kMaxCommandBuffers, std::unique_ptr<CommandBuffer>>;

    virtual ~GpuWorkSubmission();

    bool isFinished(const SharedContext* sharedContext);
    void waitUntilFinished(const SharedContext* sharedContext);

    void addFinishedProc(sk_sp<RefCntedCallback> finishedProc);

protected:
    CommandBuffer* commandBuffer() {
        return fCommandBuffers.empty() ? nullptr : fCommandBuffers.back().get();
    }

    GpuWorkSubmission(SubmittedCommandBufferList cmdBuffers, QueueManager* queueManager);
    GpuWorkSubmission(std::unique_ptr<CommandBuffer> cmdBuffer, QueueManager* queueManager);

private:
    virtual bool onIsFinished(const SharedContext* sharedContext) = 0;
    virtual void onWaitUntilFinished(const SharedContext* sharedContext) = 0;

    SubmittedCommandBufferList fCommandBuffers;
    sk_sp<SkRefCnt> fOutstandingAsyncMapCounter;
    QueueManager* fQueueManager;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_GpuWorkSubmission_DEFINED
