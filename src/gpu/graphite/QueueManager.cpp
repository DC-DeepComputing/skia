/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/QueueManager.h"

#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Recording.h"
#include "include/private/SkAssert.h"
#include "include/private/SkLog.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/GpuWorkSubmission.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/UploadBufferManager.h"
#include "src/gpu/graphite/task/Task.h"
#include "src/gpu/graphite/task/TaskList.h"

namespace skgpu::graphite {

// This constant determines how many OutstandingSubmissions are allocated together as a block in
// the deque. As such it needs to balance allocating too much memory vs. incurring
// allocation/deallocation thrashing. It should roughly correspond to the max number of outstanding
// submissions we expect to see.
static constexpr int kDefaultOutstandingAllocCnt = 8;

QueueManager::QueueManager(const SharedContext* sharedContext)
        : fSharedContext(sharedContext)
        , fOutstandingSubmissions(sizeof(OutstandingSubmission), kDefaultOutstandingAllocCnt) {
    fProtectedTransitionSemaphore = this->initProtectedTransitionSemaphore();
}

QueueManager::~QueueManager() {
    if (fSharedContext->caps()->allowCpuSync()) {
        this->checkForFinishedWork(SyncToCpu::kYes);
    } else if (!fOutstandingSubmissions.empty()) {
        SK_ABORT("When ContextOptions::fNeverYieldToWebGPU is specified all GPU work "
                 "must be finished before destroying Context.");
    }
}

std::vector<std::unique_ptr<CommandBuffer>>*
QueueManager::getAvailableCommandBufferList(Protected isProtected) {
    return isProtected == Protected::kNo ? &fAvailableCommandBuffers
                                         : &fAvailableProtectedCommandBuffers;
}

bool QueueManager::setupCommandBuffer(ResourceProvider* resourceProvider, Protected isProtected) {
    if (!this->protectedMultiQueueSupport()) {
        isProtected = fSharedContext->isProtected();
    }
    std::unique_ptr<CommandBuffer>& commandBuffer
            = isProtected == Protected::kNo ? fCurrentCommandBuffer
                                            : fCurrentProtectedCommandBuffer;
    if (!commandBuffer) {
        std::vector<std::unique_ptr<CommandBuffer>>* bufferList =
                this->getAvailableCommandBufferList(isProtected);
        if (!bufferList->empty()) {
            commandBuffer = std::move(bufferList->back());
            bufferList->pop_back();
            if (!commandBuffer->setNewCommandBufferResources()) {
                commandBuffer.reset();
            }
        }
    } else {
        if (commandBuffer->isProtected() != isProtected) {
            // If we're doing things where we are switching between using protected and unprotected
            // command buffers, it is our job to make sure previous work was submitted.
            SKIA_LOG_E("Trying to use a CommandBuffer with protectedness that differs from our "
                        "current active command buffer.");
            return false;
        }
    }
    if (!commandBuffer) {
        commandBuffer = this->getNewCommandBuffer(resourceProvider, isProtected);
    }
    if (!commandBuffer) {
        return false;
    }

    return true;
}

InsertStatus QueueManager::addRecording(const InsertRecordingInfo& info, Context* context) {
    TRACE_EVENT0_ALWAYS("skia.gpu", "AddRecording");

    // Configure the callback before validation so that failures are propagated to the finish
    // procs that were registered on `info` as well.
    GpuStatsFlags activeStatsFlags = GpuStatsFlags::kNone;
    sk_sp<RefCntedCallback> callback;
    if (info.fFinishedWithStatsProc) {
        activeStatsFlags = info.fGpuStatsFlags;
        if (activeStatsFlags != GpuStatsFlags::kNone) {
            GpuStatsFlags unsupportedStatsFlags = activeStatsFlags & ~context->supportedGpuStats();
            if (unsupportedStatsFlags != GpuStatsFlags::kNone) {
                activeStatsFlags &= ~unsupportedStatsFlags;
                SKIA_LOG_W("Requested GpuStats reporting (0x%x) but not supported by Context.",
                            static_cast<uint32_t>(unsupportedStatsFlags));
            }
        }
        callback = RefCntedCallback::Make(info.fFinishedWithStatsProc, info.fFinishedContext);
    } else if (info.fFinishedProc) {
        callback = RefCntedCallback::Make(info.fFinishedProc, info.fFinishedContext);
    }

#define RETURN_FAIL_IF(failureCase, status, fmt, ...)                   \
    if (failureCase) {                                                  \
        if (callback) { callback->setFailureResult(); }                 \
        if (info.fRecording) {                                          \
            info.fRecording->priv().setFailureResultForFinishedProcs(); \
            info.fRecording->priv().deinstantiateVolatileLazyProxies(); \
        }                                                               \
        SKIA_LOG_E(fmt, ##__VA_ARGS__);                                 \
        return status;                                                  \
    } do {} while(false)
#define SIMULATE_FAIL(status) \
    RETURN_FAIL_IF(info.fSimulatedStatus == status, status, "Simulating '" #status "' failure")

    RETURN_FAIL_IF(!info.fRecording,
                   InsertStatus::kInvalidRecording,
                   "Cannot insert null Recording");

    // Recordings from a Recorder that requires ordered recordings will have a valid recorder ID.
    // Recordings that don't have any required order are assigned SK_InvalidID.
    uint32_t recorderID = info.fRecording->priv().recorderID();
    if (recorderID != SK_InvalidGenID) {
        uint32_t* recordingID = fLastAddedRecordingIDs.find(recorderID);
        RETURN_FAIL_IF(recordingID && info.fRecording->priv().uniqueID() != *recordingID + 1,
                       InsertStatus::kOutOfOrderRecording,
                       "Recordings are expected to be replayed in order");

        // Note the new Recording ID.
        fLastAddedRecordingIDs.set(recorderID, info.fRecording->priv().uniqueID());
    }

    RETURN_FAIL_IF(info.fTargetSurface && !asSB(info.fTargetSurface)->isGraphiteBacked(),
                   InsertStatus::kInvalidRecording,
                    "Target surface passed into addRecording call is not Graphite-backed");

    SIMULATE_FAIL(InsertStatus::kInvalidRecording);

    auto resourceProvider = context->priv().resourceProvider();
    TaskList* taskList = info.fRecording->priv().taskList();

    if (this->protectedMultiQueueSupport()) {
        const bool hasUnprotected = taskList->hasTasks(Protected::kNo);
        const bool hasProtected   = taskList->hasTasks(Protected::kYes);

        SKIA_LOG_E("QueueManager::addRecording [MultiQueue] - tasks: unprotected=%d, protected=%d. targetSurface=%p",
                   hasUnprotected, hasProtected, info.fTargetSurface);

        if (hasUnprotected) {
            RETURN_FAIL_IF(!this->setupCommandBuffer(resourceProvider, Protected::kNo),
                           InsertStatus::kAddCommandsFailed,
                           "Unprotected CommandBuffer creation failed");
        }
        if (hasProtected) {
            RETURN_FAIL_IF(!this->setupCommandBuffer(resourceProvider, Protected::kYes),
                           InsertStatus::kAddCommandsFailed,
                           "Protected CommandBuffer creation failed");
        }
    } else {
        Protected isProtected = fSharedContext->isProtected();
        SKIA_LOG_E("QueueManager::addRecording [SingleQueue] - isProtected=%d", (int)isProtected);
        RETURN_FAIL_IF(!this->setupCommandBuffer(resourceProvider, isProtected),
                       InsertStatus::kAddCommandsFailed,
                       "CommandBuffer creation failed");
    }

    // This must happen before instantiating the lazy proxies, because the target for draws in this
    // recording may itself be a lazy proxy whose instantiation must be handled specially here.
    // We must also make sure the lazy proxies are instantiated successfully before we make any
    // modifications to the current command buffer, so we can't just do all this work in
    // Recording::addCommands below.
    TextureProxy* deferredTargetProxy = info.fRecording->priv().deferredTargetProxy();
    AutoDeinstantiateTextureProxy autoDeinstantiateTargetProxy(deferredTargetProxy);
    const Texture* replayTarget = nullptr;
    if (deferredTargetProxy) {
        RETURN_FAIL_IF(!info.fTargetSurface,
                       InsertStatus::kPromiseImageInstantiationFailed,
                       "No surface provided to instantiate deferred replay target");

        replayTarget = info.fRecording->priv().setupDeferredTarget(
                resourceProvider,
                static_cast<Surface*>(info.fTargetSurface),
                info.fTargetTranslation,
                info.fTargetClip);

        RETURN_FAIL_IF(!replayTarget,
                        InsertStatus::kPromiseImageInstantiationFailed,
                        "Failed to set up deferred replay target");
    }

    RETURN_FAIL_IF(info.fRecording->priv().hasNonVolatileLazyProxies() &&
                   !info.fRecording->priv().instantiateNonVolatileLazyProxies(resourceProvider),
                   InsertStatus::kPromiseImageInstantiationFailed,
                   "Non-volatile PromiseImage instantiation has failed");

    RETURN_FAIL_IF(info.fRecording->priv().hasVolatileLazyProxies() &&
                   !info.fRecording->priv().instantiateVolatileLazyProxies(resourceProvider),
                   InsertStatus::kPromiseImageInstantiationFailed,
                   "Volitile PromiseImage instantiation has failed");

    SIMULATE_FAIL(InsertStatus::kPromiseImageInstantiationFailed);

    if (activeStatsFlags != GpuStatsFlags::kNone) {
        if (fCurrentCommandBuffer && !fCurrentCommandBuffer->startStatsQuery(activeStatsFlags)) {
            activeStatsFlags = GpuStatsFlags::kNone;
        }
        if (fCurrentProtectedCommandBuffer && !fCurrentProtectedCommandBuffer->startStatsQuery(activeStatsFlags)) {
            activeStatsFlags = GpuStatsFlags::kNone;
        }
    }

    if (this->protectedMultiQueueSupport()) {
        bool unprotectedSuccess = true;
        bool protectedSuccess = true;

        if (fCurrentCommandBuffer) {
            fCurrentCommandBuffer->addWaitSemaphores(info.fNumWaitSemaphores, info.fWaitSemaphores);
            unprotectedSuccess = info.fRecording->priv().addCommands(context,
                                                                      fCurrentCommandBuffer.get(),
                                                                      replayTarget,
                                                                      info.fTargetTranslation,
                                                                      info.fTargetClip);
        }

        if (fCurrentProtectedCommandBuffer) {
            if (!fCurrentCommandBuffer) {
                fCurrentProtectedCommandBuffer->addWaitSemaphores(info.fNumWaitSemaphores, info.fWaitSemaphores);
            }
            protectedSuccess = info.fRecording->priv().addCommands(context,
                                                                    fCurrentProtectedCommandBuffer.get(),
                                                                    replayTarget,
                                                                    info.fTargetTranslation,
                                                                    info.fTargetClip);
        }

        if (!unprotectedSuccess || !protectedSuccess) {
            SKIA_LOG_E("[DIAG] QueueManager::addRecording FAILED - unprotectedSuccess: %d, protectedSuccess: %d, fCurrentCommandBuffer: %p, fCurrentProtectedCommandBuffer: %p",
                       (int)unprotectedSuccess, (int)protectedSuccess, fCurrentCommandBuffer.get(), fCurrentProtectedCommandBuffer.get());
            std::string failureMsg;
            const bool validPipelines = info.fRecording->priv().taskList()->visitPipelines(
                    [&failureMsg](const GraphicsPipeline* pipeline) {
                        if (auto failure = pipeline->didAsyncCompilationFail()) {
                            failureMsg = *failure;
                            return false;
                        }
                        return true;
                    });

            RETURN_FAIL_IF(validPipelines,
                           InsertStatus::kAddCommandsFailed,
                           "Adding Recording commands to the CommandBuffer has failed");
            RETURN_FAIL_IF(
                    true,
                    InsertStatus(InsertStatus::kAsyncShaderCompilesFailed, std::move(failureMsg)),
                    "Async pipeline compiles failed, unable to add Recording commands");
        }

        if (fCurrentCommandBuffer && fCurrentProtectedCommandBuffer) {
            RETURN_FAIL_IF(!fProtectedTransitionSemaphore.isValid(),
                           InsertStatus::kAddCommandsFailed,
                           "Failed to add semaphore to transition from unprotected to protected");

            fCurrentCommandBuffer->addSignalSemaphores(1, &fProtectedTransitionSemaphore);
            fCurrentProtectedCommandBuffer->addWaitSemaphores(1, &fProtectedTransitionSemaphore);
        }
    } else {
        CommandBuffer* activeCmdBuffer
            = fSharedContext->isProtected() == Protected::kNo ? fCurrentCommandBuffer.get()
                                                              : fCurrentProtectedCommandBuffer.get();
        activeCmdBuffer->addWaitSemaphores(info.fNumWaitSemaphores, info.fWaitSemaphores);
        if (!info.fRecording->priv().addCommands(context,
                                                 activeCmdBuffer,
                                                 replayTarget,
                                                 info.fTargetTranslation,
                                                 info.fTargetClip)) {
            std::string failureMsg;
            const bool validPipelines = info.fRecording->priv().taskList()->visitPipelines(
                    [&failureMsg](const GraphicsPipeline* pipeline) {
                        if (auto failure = pipeline->didAsyncCompilationFail()) {
                            failureMsg = *failure;
                            return false;
                        }
                        return true;
                    });

            RETURN_FAIL_IF(validPipelines,
                           InsertStatus::kAddCommandsFailed,
                           "Adding Recording commands to the CommandBuffer has failed");
            RETURN_FAIL_IF(
                    true,
                    InsertStatus(InsertStatus::kAsyncShaderCompilesFailed, std::move(failureMsg)),
                    "Async pipeline compiles failed, unable to add Recording commands");
        }
    }

    SIMULATE_FAIL(InsertStatus::kAddCommandsFailed);
    SIMULATE_FAIL(InsertStatus::kAsyncShaderCompilesFailed);

    CommandBuffer* lastCmdBuffer = fCurrentProtectedCommandBuffer ? fCurrentProtectedCommandBuffer.get()
                                                                  : fCurrentCommandBuffer.get();

    SKIA_LOG_E("QueueManager::addRecording - lastCmdBuffer=%p (isProtected=%d), numSignalSemaphores=%zu, hasTargetState=%d",
               lastCmdBuffer, lastCmdBuffer ? (int)lastCmdBuffer->isProtected() : -1,
               info.fNumSignalSemaphores, (info.fTargetTextureState != nullptr));

    if (lastCmdBuffer) {
        lastCmdBuffer->addSignalSemaphores(info.fNumSignalSemaphores, info.fSignalSemaphores);
        if (info.fTargetTextureState) {
            lastCmdBuffer->prepareSurfaceForStateUpdate(info.fTargetSurface,
                                                        info.fTargetTextureState);
        }

        if (callback) {
            lastCmdBuffer->addFinishedProc(std::move(callback));
        }
    }

    info.fRecording->priv().deinstantiateVolatileLazyProxies();

    // If we got here, the simulated status should be kSuccess or it means we missed returning the
    // simulated error earlier.
    SkASSERT(info.fSimulatedStatus == InsertStatus::kSuccess);
    return InsertStatus::kSuccess;
}

bool QueueManager::addTask(Task* task,
                           Context* context,
                           Protected isProtected) {
    SkASSERT(task);
    if (!task) {
        SKIA_LOG_E("No valid Task passed into addTask call");
        return false;
    }

    if (!this->protectedMultiQueueSupport()) {
        isProtected = fSharedContext->isProtected();
    }

    if (!this->setupCommandBuffer(context->priv().resourceProvider(), isProtected)) {
        SKIA_LOG_E("CommandBuffer creation failed");
        return false;
    }

    CommandBuffer* activeCmdBuffer
        = isProtected == Protected::kNo ? fCurrentCommandBuffer.get()
                                        : fCurrentProtectedCommandBuffer.get();
    if (task->addCommands(context, activeCmdBuffer, {}) == Task::Status::kFail) {
        SKIA_LOG_E("Adding Task commands to the CommandBuffer has failed");
        return false;
    }

    return true;
}

bool QueueManager::addFinishInfo(const InsertFinishInfo& info,
                                 ResourceProvider* resourceProvider,
                                 SkSpan<const sk_sp<Buffer>> buffersToAsyncMap) {
    sk_sp<RefCntedCallback> callback;
    if (info.fFinishedProc) {
        callback = RefCntedCallback::Make(info.fFinishedProc, info.fFinishedContext);
    }

    SKIA_LOG_E("QueueManager::addFinishInfo %p %p",
               fCurrentCommandBuffer.get(), fCurrentProtectedCommandBuffer.get());
    skgpu::Protected isProtected
        = this->protectedMultiQueueSupport() ? skgpu::Protected::kNo
                                             : fSharedContext->caps()->protectedSupport()
                                                    ? skgpu::Protected::kYes
                                                    : skgpu::Protected::kNo;
    if (!this->setupCommandBuffer(resourceProvider, isProtected)) {
        if (callback) {
            callback->setFailureResult();
        }
        SKIA_LOG_E("CommandBuffer creation failed");
        return false;
    }

    CommandBuffer* activeCmdBuffer
        = isProtected == Protected::kNo ? fCurrentCommandBuffer.get()
                                        : fCurrentProtectedCommandBuffer.get();
    if (callback) {
        activeCmdBuffer->addFinishedProc(std::move(callback));
    }
    activeCmdBuffer->addBuffersToAsyncMapOnSubmit(buffersToAsyncMap);

    return true;
}

bool QueueManager::submitToGpu(const SubmitInfo& submitInfo) {
    TRACE_EVENT0_ALWAYS("skia.gpu", "SubmitToGPU");

    SKIA_LOG_E("QueueManager::submitToGpu - active buffers: unprotected=%p, protected=%p",
               fCurrentCommandBuffer.get(), fCurrentProtectedCommandBuffer.get());

    sk_sp<RefCntedCallback> callback;
    if (submitInfo.fFinishedProc) {
        callback = RefCntedCallback::Make(submitInfo.fFinishedProc, submitInfo.fFinishedContext);
    }

    if (this->protectedMultiQueueSupport()) {
        if (!fCurrentCommandBuffer && !fCurrentProtectedCommandBuffer) {
            // If a finish proc was provided, attach it to the most recent outstanding submission,
            // or let it fire immediately if the GPU is idle (when callback goes out of scope).
            if (callback) {
                if (!fOutstandingSubmissions.empty()) {
                    OutstandingSubmission* back =
                            (OutstandingSubmission*)fOutstandingSubmissions.back();
                    (*back)->addFinishedProc(std::move(callback));
                }
                return true;
            }
            // We warn because this probably representative of a bad client state, where they don't
            // need to submit but didn't notice, but technically the submit itself is fine (no-op), so
            // we return true.
            SKIA_LOG_D("Submit called with no active command buffer!");
            return true;
        }

        // Attach the callback to the last submitted command buffer (we submit unprotected then
        // protected).
        if (callback) {
            if (fCurrentProtectedCommandBuffer) {
                fCurrentProtectedCommandBuffer->addFinishedProc(std::move(callback));
            } else {
                SkASSERT(fCurrentCommandBuffer);
                fCurrentCommandBuffer->addFinishedProc(std::move(callback));
            }
        }
    } else {
        CommandBuffer* activeCmdBuffer
            = fSharedContext->isProtected() == Protected::kNo ? fCurrentCommandBuffer.get()
                                                              : fCurrentProtectedCommandBuffer.get();
        if (!activeCmdBuffer) {
            // If a finish proc was provided, attach it to the most recent outstanding submission,
            // or let it fire immediately if the GPU is idle (when callback goes out of scope).
            if (callback) {
                if (!fOutstandingSubmissions.empty()) {
                    OutstandingSubmission* back =
                            (OutstandingSubmission*)fOutstandingSubmissions.back();
                    (*back)->addFinishedProc(std::move(callback));
                }
                return true;
            }
            // We warn because this probably representative of a bad client state, where they don't
            // need to submit but didn't notice, but technically the submit itself is fine (no-op), so
            // we return true.
            SKIA_LOG_D("Submit called with no active command buffer!");
            return true;
        }

#ifdef SK_DEBUG
        if (!activeCmdBuffer->hasWork()) {
            SKIA_LOG_D("Submitting empty command buffer!");
        }
#endif

        if (callback) {
            activeCmdBuffer->addFinishedProc(std::move(callback));
        }
    }

    auto submission = this->onSubmitToGpu(submitInfo);
    if (!submission) {
        return false;
    }

    new (fOutstandingSubmissions.push_back()) OutstandingSubmission(std::move(submission));
    return true;
}

bool QueueManager::hasUnfinishedGpuWork() { return !fOutstandingSubmissions.empty(); }

bool QueueManager::hasPendingGPUWork() const {
    // Only check if fCurrentCommandBuffer/fCurrentProtectedCommandBuffer is non-null.
    // If there is no command in the command buffer i.e. fCurrentCommandBuffer->hasWork() is false,
    // it's still considered valid. Because clients can insert a recording without any command just
    // to track the finish proc.
    return fCurrentCommandBuffer != nullptr || fCurrentProtectedCommandBuffer != nullptr;
}

void QueueManager::checkForFinishedWork(SyncToCpu sync) {
    TRACE_EVENT1("skia.gpu", TRACE_FUNC, "sync", sync == SyncToCpu::kYes);

    if (sync == SyncToCpu::kYes) {
        SkASSERT(fSharedContext->caps()->allowCpuSync());
        // wait for the last submission to finish
        OutstandingSubmission* back = (OutstandingSubmission*)fOutstandingSubmissions.back();
        if (back) {
            (*back)->waitUntilFinished(fSharedContext);
        }
    }

    // Iterate over all the outstanding submissions to see if any have finished. The work
    // submissions are in order from oldest to newest, so we start at the front to check if they
    // have finished. If so we pop it off and move onto the next.
    // Repeat till we find a submission that has not finished yet (and all others afterwards are
    // also guaranteed to not have finished).
    OutstandingSubmission* front = (OutstandingSubmission*)fOutstandingSubmissions.front();
    while (front && (*front)->isFinished(fSharedContext)) {
        // Make sure we remove before deleting as deletion might try to kick off another submit
        // (though hopefully *not* in Graphite).
        fOutstandingSubmissions.pop_front();

        // Since we used placement new we are responsible for calling the destructor manually.
        front->~OutstandingSubmission();
        front = (OutstandingSubmission*)fOutstandingSubmissions.front();
    }
    SkASSERT(sync == SyncToCpu::kNo || fOutstandingSubmissions.empty());
}

void QueueManager::returnCommandBuffer(std::unique_ptr<CommandBuffer> commandBuffer) {
    std::vector<std::unique_ptr<CommandBuffer>>* bufferList =
            this->getAvailableCommandBufferList(commandBuffer->isProtected());
    bufferList->push_back(std::move(commandBuffer));
}

void QueueManager::addUploadBufferManagerRefs(UploadBufferManager* uploadManager,
                                              ResourceProvider* resourceProvider) {
    skgpu::Protected isProtected
        = this->protectedMultiQueueSupport() ? skgpu::Protected::kNo
                                             : fSharedContext->caps()->protectedSupport()
                                                    ? skgpu::Protected::kYes
                                                    : skgpu::Protected::kNo;
    this->setupCommandBuffer(resourceProvider, isProtected);
    CommandBuffer* activeCmdBuffer
        = isProtected == Protected::kNo ? fCurrentCommandBuffer.get()
                                        : fCurrentProtectedCommandBuffer.get();
    SkASSERT(activeCmdBuffer);
    uploadManager->transferToCommandBuffer(activeCmdBuffer);
}

bool QueueManager::protectedMultiQueueSupport() const {
    return fSharedContext->caps()->protectedMultiQueueSupport();
}

} // namespace skgpu::graphite
