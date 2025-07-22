/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnQueueManager.h"

#include "src/gpu/graphite/dawn/DawnAsyncWait.h"
#include "src/gpu/graphite/dawn/DawnCommandBuffer.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"
#include "src/gpu/graphite/dawn/DawnResourceProvider.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"

namespace skgpu::graphite {
namespace {
class DawnWorkSubmissionWithFuture final : public GpuWorkSubmission {
public:
    DawnWorkSubmissionWithFuture(std::unique_ptr<CommandBuffer> cmdBuffer,
                                 DawnQueueManager* queueManager);

private:
    bool onIsFinished(const SharedContext* sharedContext) override;
    void onWaitUntilFinished(const SharedContext* sharedContext) override;

    wgpu::Future fSubmittedWorkDoneFuture;
};

DawnWorkSubmissionWithFuture::DawnWorkSubmissionWithFuture(std::unique_ptr<CommandBuffer> cmdBuffer,
                                                           DawnQueueManager* queueManager)
        : GpuWorkSubmission(std::move(cmdBuffer), queueManager) {
    fSubmittedWorkDoneFuture = queueManager->dawnQueue().OnSubmittedWorkDone(
            wgpu::CallbackMode::WaitAnyOnly, [](wgpu::QueueWorkDoneStatus, wgpu::StringView) {});
}

bool DawnWorkSubmissionWithFuture::onIsFinished(const SharedContext* sharedContext) {
    wgpu::FutureWaitInfo waitInfo{};
    waitInfo.future = fSubmittedWorkDoneFuture;
    const auto& instance = static_cast<const DawnSharedContext*>(sharedContext)
                                   ->instance();
    if (instance.WaitAny(1, &waitInfo, /*timeoutNS=*/0) != wgpu::WaitStatus::Success) {
        return false;
    }

    return waitInfo.completed;
}

void DawnWorkSubmissionWithFuture::onWaitUntilFinished(const SharedContext* sharedContext) {
    wgpu::FutureWaitInfo waitInfo{};
    waitInfo.future = fSubmittedWorkDoneFuture;
    const auto& instance = static_cast<const DawnSharedContext*>(sharedContext)
                                   ->instance();
    [[maybe_unused]] auto status =
            instance.WaitAny(1, &waitInfo, /*timeoutNS=*/std::numeric_limits<uint64_t>::max());
    SkASSERT(status == wgpu::WaitStatus::Success);
    SkASSERT(waitInfo.completed);
}
} // namespace

DawnQueueManager::DawnQueueManager(wgpu::Queue queue, const SharedContext* sharedContext)
        : QueueManager(sharedContext), fQueue(std::move(queue)) {}

void DawnQueueManager::tick() const { this->dawnSharedContext()->tick(); }

const DawnSharedContext* DawnQueueManager::dawnSharedContext() const {
    return static_cast<const DawnSharedContext*>(fSharedContext);
}

std::unique_ptr<CommandBuffer> DawnQueueManager::getNewCommandBuffer(
        ResourceProvider* resourceProvider, Protected) {
    return DawnCommandBuffer::Make(dawnSharedContext(),
                                   static_cast<DawnResourceProvider*>(resourceProvider));
}

QueueManager::OutstandingSubmission DawnQueueManager::onSubmitToGpu() {
    SkASSERT(fCurrentCommandBuffer);
    DawnCommandBuffer* dawnCmdBuffer = static_cast<DawnCommandBuffer*>(fCurrentCommandBuffer.get());
    auto wgpuCmdBuffer = dawnCmdBuffer->finishEncoding();
    if (!wgpuCmdBuffer) {
        fCurrentCommandBuffer->callFinishedProcs(/*success=*/false);
        return nullptr;
    }

    fQueue.Submit(/*commandCount=*/1, &wgpuCmdBuffer);
    return std::make_unique<DawnWorkSubmissionWithFuture>(std::move(fCurrentCommandBuffer), this);
}

#if defined(GPU_TEST_UTILS)
void DawnQueueManager::startCapture() {
    // TODO: Dawn doesn't have capturing feature yet.
}

void DawnQueueManager::stopCapture() {
    // TODO: Dawn doesn't have capturing feature yet.
}
#endif

} // namespace skgpu::graphite
