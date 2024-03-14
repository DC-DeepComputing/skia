/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnQueueManager.h"

#include "src/gpu/graphite/dawn/DawnAsyncWait.h"
#include "src/gpu/graphite/dawn/DawnCommandBuffer.h"
#include "src/gpu/graphite/dawn/DawnResourceProvider.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/gpu/graphite/dawn/DawnUtilsPriv.h"

namespace skgpu::graphite {
namespace {
class DawnWorkSubmission final : public GpuWorkSubmission {
public:
    ~DawnWorkSubmission() override = default;
    DawnWorkSubmission(std::unique_ptr<CommandBuffer> cmdBuffer,
                       DawnQueueManager* queueManager,
                       const DawnSharedContext* sharedContext);

private:
    bool onIsFinished() override;
    void onWaitUntilFinished() override;

#if defined(__EMSCRIPTEN__)
    DawnAsyncWait fAsyncWait;
#else
    wgpu::Instance fDawnInstance;
    wgpu::Future fSubmittedWorkDoneFuture;
#endif
};

#if defined(__EMSCRIPTEN__)
DawnWorkSubmission::DawnWorkSubmission(std::unique_ptr<CommandBuffer> cmdBuffer,
                                       DawnQueueManager* queueManager,
                                       const DawnSharedContext* sharedContext)
        : GpuWorkSubmission(std::move(cmdBuffer), queueManager), fAsyncWait(sharedContext) {
    queueManager->dawnQueue().OnSubmittedWorkDone(
            // This is parameter is being removed:
            // https://github.com/webgpu-native/webgpu-headers/issues/130
            /*signalValue=*/0,
            [](WGPUQueueWorkDoneStatus, void* userData) {
                auto asyncWaitPtr = static_cast<DawnAsyncWait*>(userData);
                asyncWaitPtr->signal();
            },
            &fAsyncWait);
}

bool DawnWorkSubmission::onIsFinished() { return fAsyncWait.yieldAndCheck(); }
void DawnWorkSubmission::onWaitUntilFinished() { fAsyncWait.busyWait(); }
#else
// Dawn native version.
DawnWorkSubmission::DawnWorkSubmission(std::unique_ptr<CommandBuffer> cmdBuffer,
                                       DawnQueueManager* queueManager,
                                       const DawnSharedContext* sharedContext)
        : GpuWorkSubmission(std::move(cmdBuffer), queueManager)
        , fDawnInstance(sharedContext->device().GetAdapter().GetInstance()) {
    wgpu::QueueWorkDoneCallbackInfo callbackInfo{};
    callbackInfo.mode = wgpu::CallbackMode::AllowSpontaneous;
    callbackInfo.callback = [](WGPUQueueWorkDoneStatus, void*) {};

    fSubmittedWorkDoneFuture = queueManager->dawnQueue().OnSubmittedWorkDone(callbackInfo);
}

bool DawnWorkSubmission::onIsFinished() {
    wgpu::FutureWaitInfo waitInfo{};
    waitInfo.future = fSubmittedWorkDoneFuture;
    if (fDawnInstance.WaitAny(1, &waitInfo, 0) != wgpu::WaitStatus::Success) {
        return false;
    }

    return waitInfo.completed;
}
void DawnWorkSubmission::onWaitUntilFinished() {
    wgpu::FutureWaitInfo waitInfo{};
    waitInfo.future = fSubmittedWorkDoneFuture;
    auto status = fDawnInstance.WaitAny(1, &waitInfo, std::numeric_limits<uint64_t>::max());
    SkASSERT(status == wgpu::WaitStatus::Success);
    SkASSERT(waitInfo.completed);
}
#endif
} // namespace

DawnQueueManager::DawnQueueManager(wgpu::Queue queue, const SharedContext* sharedContext)
        : QueueManager(sharedContext), fQueue(std::move(queue)) {}

void DawnQueueManager::tick() const { this->dawnSharedContext()->tick(); }

const DawnSharedContext* DawnQueueManager::dawnSharedContext() const {
    return static_cast<const DawnSharedContext*>(fSharedContext);
}

std::unique_ptr<CommandBuffer> DawnQueueManager::getNewCommandBuffer(
        ResourceProvider* resourceProvider) {
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

    return std::make_unique<DawnWorkSubmission>(std::move(fCurrentCommandBuffer),
                                                this,
                                                dawnSharedContext());
}

#if defined(GRAPHITE_TEST_UTILS)
void DawnQueueManager::startCapture() {
    // TODO: Dawn doesn't have capturing feature yet.
}

void DawnQueueManager::stopCapture() {
    // TODO: Dawn doesn't have capturing feature yet.
}
#endif

} // namespace skgpu::graphite
