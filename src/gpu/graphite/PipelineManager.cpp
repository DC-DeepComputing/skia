/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PipelineManager.h"

#include "include/private/SkLog.h"
#include "src/core/SkTaskGroup.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/GraphicsPipelineHandle.h"
#include "src/gpu/graphite/PipelineCreationTask.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/SharedContext.h"

// The threaded PipelineManager is set up to work with a executor with two worklists.
// All work from the first work list must be completed before work from the second list
// is begun. In-line compiles will be put in the high priority list while precompiles
// will be put in the low priority work list. If a single work list executor is provided
// everything will collapse to just being interleaved on that single work list.
// Note that the number of worklists is orthogonal to the number of threads.
static constexpr int kHighPriorityWorkList = 0;
static constexpr int kLowPriorityWorkList = 1;

namespace skgpu::graphite {

// Directly accessing the two variants is thread safe because a given GraphicsPipelineHandle
// does not switch between a task or pipeline when the task completes. The 'fCompleted' check
// is atomic and, if the task is completed, the 'fPipeline' access is thread safe. There is,
// obviously, an inherent race wrt the 'fCompleted' access. Callers must either ensure that
// the Handle has already been resolved or accept some raciness in the response.
sk_sp<GraphicsPipeline> GraphicsPipelineHandle::pipelineOrNull() const {
    if (std::holds_alternative<sk_sp<GraphicsPipeline>>(fTaskOrPipeline)) {
        return std::get<sk_sp<GraphicsPipeline>>(fTaskOrPipeline);
    }
    sk_sp<PipelineCreationTask> task = std::get<sk_sp<PipelineCreationTask>>(fTaskOrPipeline);
    if (!task->fCompleted) {
        return nullptr;
    }
    return task->fPipeline;
}

GraphicsPipelineHandle::GraphicsPipelineHandle(sk_sp<PipelineCreationTask> task)
    : fTaskOrPipeline(std::move(task)) {}

GraphicsPipelineHandle::GraphicsPipelineHandle(sk_sp<GraphicsPipeline> pipeline)
    : fTaskOrPipeline(std::move(pipeline)) {}

PipelineManager::PipelineManager(SkExecutor* executor) {
    if (executor) {
        fTaskGroup = std::make_unique<SkTaskGroup>(*executor);
    }
}

PipelineManager::~PipelineManager() {
    // The task group should've been shutdown and deleted in Context's destructor (via
    // shutDown()).
    {
        SkDEBUGCODE(SkAutoSpinlock lock{fSpinLock};)
        SkASSERT(!fTaskGroup);
    }
}

const UniqueKey& PipelineManager::Traits::GetKey(const sk_sp<PipelineCreationTask>& task) {
    return task->fPipelineKey;
}

uint32_t PipelineManager::Traits::Hash(const UniqueKey& pipelineKey) {
    return pipelineKey.hash();
}

// TODO(robertphillips): If either 'findTask' or 'findOrCreateTask' returns a hit and it
// is 'fInProgress', if the search was for a non-Precompile task but we found a Precompile task
// then the task was initially added to the wrong worklist (i.e., the low-priority one). We need
// a way to move such tasks to the high priority work list.
GraphicsPipelineHandle PipelineManager::createHandle(
        SharedContext* sharedContext,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags) {
    GlobalCache* globalCache = sharedContext->globalCache();
    const Caps* caps = sharedContext->caps();

    UniqueKey pipelineKey = caps->makeGraphicsPipelineKey(pipelineDesc, renderPassDesc);

    if (sk_sp<PipelineCreationTask> task = this->findTask(pipelineKey)) {
        // There is a task in progress to compile this Pipeline so it can't be ready yet (i.e.,
        // it isn't in the Pipeline Cache).
        return GraphicsPipelineHandle(std::move(task));
    }

    // There is no task in flight but there could've been one in the past that
    // compiled the Pipeline - check to see if one exists.
    sk_sp<GraphicsPipeline> pipeline = globalCache->findGraphicsPipeline(
            pipelineKey,
            pipelineCreationFlags);
    if (pipeline) {
        return GraphicsPipelineHandle(std::move(pipeline));
    }

    // We dropped our SpinLock during the findGraphicsPipeline call so a new Task could've
    // been created in the interim. However, it is vanishingly unlikely that a Task would've
    // been created and finished during the gap (so the result of the findGraphicsPipeline
    // call is still valid).
    sk_sp<PipelineCreationTask> task = this->findOrCreateTask(pipelineKey,
                                                              pipelineDesc,
                                                              renderPassDesc,
                                                              pipelineCreationFlags);
    return GraphicsPipelineHandle(std::move(task));
}

void PipelineManager::startPipelineCreationTask(SharedContext* sharedContext,
                                                sk_sp<const RuntimeEffectDictionary> runtimeDict,
                                                const GraphicsPipelineHandle& handle) {
    if (std::holds_alternative<sk_sp<GraphicsPipeline>>(handle.fTaskOrPipeline)) {
        return;
    }

    sk_sp<PipelineCreationTask> task =
            std::get<sk_sp<PipelineCreationTask>>(handle.fTaskOrPipeline);

    if (task->fInProgress.exchange(true)) {
        // Tasks are only removed from the TaskList when they are complete. This means that
        // an in-flight task can be found and resubmitted for compilation. The 'fInProgress'
        // guard ensures each task is only kicked off once.
        return;
    }

    auto compileTask = [sharedContext, runtimeDict, this, task] {
        // Note: this lambda function relies on the continued existence of the shared
        // context and the PipelineManager. The RuntimeEffectDictionary and task are reffed.
        task->fPipeline = sharedContext->findOrCreateGraphicsPipeline(
                runtimeDict.get(),
                task->fPipelineKey,
                task->fGraphicsPipelineDesc,
                task->fRenderPassDesc,
                task->fPipelineCreationFlags);

        if (!task->fPipeline) {
            SKIA_LOG_W("Failed to create GraphicsPipeline!");
        }

        this->signalCompleted(task.get());
        this->removeTask(task.get());
    };

    {
        SkAutoSpinlock lock{fSpinLock};

        if (fTaskGroup) {
            int workList = task->forPrecompile() ? kLowPriorityWorkList : kHighPriorityWorkList;

            fTaskGroup->add(std::move(compileTask), workList);
            return;
        }
    }

    compileTask();
}

sk_sp<GraphicsPipeline> PipelineManager::resolveHandle(const GraphicsPipelineHandle& handle) {
    if (std::holds_alternative<sk_sp<GraphicsPipeline>>(handle.fTaskOrPipeline)) {
        return std::get<sk_sp<GraphicsPipeline>>(handle.fTaskOrPipeline);
    }

    // Since 'fTaskOrPipeline' doesn't hold a pipeline the pipeline must not have existed when
    // the handle was created so a compilation task must've been created to compile it
    sk_sp<PipelineCreationTask> task =
            std::get<sk_sp<PipelineCreationTask>>(handle.fTaskOrPipeline);

    // For the non-threaded PipelineManager, the GraphicsPipeline will have been compiled in-line
    // so will already have been completed.
    this->potentiallyWaitOn(task.get());
    return task->fPipeline;
}

void PipelineManager::shutDown() {
    // We null out 'fTaskGroup' so no more threaded work can be added after this point.
    std::unique_ptr<SkTaskGroup> tmp;
    {
        SkAutoSpinlock lock{fSpinLock};
        tmp = std::move(fTaskGroup);
    }
    if (tmp) {
        tmp->discardAllPendingWork();
        // We have to wait for the remaining tasks to complete bc they rely on the existence
        // of the SharedContext and the PipelineManager (this).
        tmp->wait();
    }
}

#if defined(GPU_TEST_UTILS)
void PipelineManager::wait_TestOnly() {
    SkTaskGroup* tmp;
    {
        SkAutoSpinlock lock{fSpinLock};
        tmp = fTaskGroup.get();
    }
    // This isn't safe (since 'fTaskGroup' could be altered on some other thread) but, hopefully,
    // the unit tests know what they're doing (i.e., don't delete the owning Context while
    // in this method).
    if (tmp) {
        tmp->wait();
    }
}

PipelineManager::Stats PipelineManager::getStats() const {
    SkAutoSpinlock lock{fSpinLock};

    return fStats;
}
#endif

sk_sp<PipelineCreationTask> PipelineManager::findTask(const UniqueKey& pipelineKey) {
    SkAutoSpinlock lock{fSpinLock};

    sk_sp<PipelineCreationTask> task = fActiveTasks.findOrNull(pipelineKey);

#if defined(GPU_TEST_UTILS)
    if (task) {
        fStats.fNumPreemptivelyFoundTasks++;
    }
#endif

    return task;
}

sk_sp<PipelineCreationTask> PipelineManager::findOrCreateTask(
        const UniqueKey& pipelineKey,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags) {
    SkAutoSpinlock lock{fSpinLock};

    sk_sp<PipelineCreationTask>* task = fActiveTasks.find(pipelineKey);
    if (task) {
        // There is a race in createHandle from when we first check for a task; then, failing that,
        // check for an existing pipeline; then, failing that, try to create a new task. Thus,
        // we can sometimes find our task here.
#if defined(GPU_TEST_UTILS)
        fStats.fNumTaskCreationRaces++;
#endif
        return *task;
    }

#if defined(GPU_TEST_UTILS)
    fStats.fNumTasksCreated++;
#endif

    sk_sp<PipelineCreationTask> newTask = sk_sp<PipelineCreationTask>(
            new PipelineCreationTask(pipelineKey,
                                     pipelineDesc,
                                     renderPassDesc,
                                     pipelineCreationFlags));
    fActiveTasks.set(newTask);
    return newTask;
}

void PipelineManager::removeTask(PipelineCreationTask* task) {
    SkAutoSpinlock lock{fSpinLock};

    fActiveTasks.remove(task->fPipelineKey);
}

void PipelineManager::signalCompleted(PipelineCreationTask* task) {
    std::unique_lock<std::mutex> lock(fMutex);

    task->fCompleted = true;

    lock.unlock();
    // potentiallyWaitOn should only ever be called from the main thread (on which
    // Context::insertRecording is called) so only one thread should ever be waiting
    fConditionVariable.notify_one();
}


void PipelineManager::potentiallyWaitOn(PipelineCreationTask* task) {
    std::unique_lock<std::mutex> lock(fMutex);

    if (task->fCompleted) {
        return;
    }
    fConditionVariable.wait(lock, [task]{ return task->fCompleted.load(); });

    SkASSERT(task->fCompleted);
}

} // namespace skgpu::graphite
