/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/NewObjects.h"

#include "src/core/SkTaskGroup.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SharedContext.h"

namespace skgpu::graphite {

PipelineManager::PipelineManager(SkExecutor* executor) {
    if (executor) {
        fTaskGroup1 = std::make_unique<SkTaskGroup>(*executor);
    }
}
// TODO: do we need a join here?!
PipelineManager::~PipelineManager() {}

sk_sp<PipelineTask> PipelineManager::findOrCreateTask1(
        const UniqueKey& pipelineKey,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags) {
    SkAutoSpinlock lock{fSpinLock};

    sk_sp<PipelineTask>* task = fActiveTasks.find(pipelineKey);
    if (task) {
        return *task;
    }

    sk_sp<PipelineTask> newTask = sk_sp<PipelineTask>(new PipelineTask(pipelineKey,
                                                                       pipelineDesc,
                                                                       renderPassDesc,
                                                                       pipelineCreationFlags));
    fActiveTasks.set(newTask);
    return newTask;
}

void PipelineManager::removeTask1(PipelineTask* task) {
    SkAutoSpinlock lock{fSpinLock};

    fActiveTasks.remove(task->fPipelineKey);
}

sk_sp<PipelineTask> PipelineManager::findTask1(const UniqueKey& pipelineKey) {
    SkAutoSpinlock lock{fSpinLock};

    return fActiveTasks.findOrNull(pipelineKey);
}

// TODO: do we need refs on the resourceProvider or the runtimeDict ?
void PipelineManager::kickOffTask(ResourceProvider* resourceProvider,
                                  const RuntimeEffectDictionary* runtimeDict,
                                  GraphicsPipelineHandle* handle) {
    if (std::holds_alternative<sk_sp<GraphicsPipeline>>(handle->fTaskOrPipeline)) {
        return;
    }

    // !!! Being in the active task list doesn't mean that it has or hasn't already been kicked off
    // We need to guard against the case where two DrawPasses have the same Pipeline/Task and
    // don't kick it off multiple times.
    sk_sp<PipelineTask> task = std::get<sk_sp<PipelineTask>>(handle->fTaskOrPipeline);

    if (task->fInProgress.exchange(true)) {
        return; // someone else already kicked off the task
    }

    if (fTaskGroup1) {
        // It seems like this doesn't need a sk_sp<PipelineTask> - just a PipelineTask*
        auto compileTask = [resourceProvider, runtimeDict, this, task] {
            task->fPipeline1 = resourceProvider->findOrCreateGraphicsPipeline(
                    runtimeDict,
                    task->fPipelineKey,
                    task->fGraphicsPipelineDesc,
                    task->fRenderPassDesc);

            task->signalComplete();
            task->fCompleted1 = true;

            this->removeTask1(task.get());
        };

        fTaskGroup1->add(std::move(compileTask));
    } else {
        task->fPipeline1 = resourceProvider->findOrCreateGraphicsPipeline(
                runtimeDict,
                task->fPipelineKey,
                task->fGraphicsPipelineDesc,
                task->fRenderPassDesc);

        task->fCompleted1 = true;

        this->removeTask1(task.get());
    }
}

std::unique_ptr<GraphicsPipelineHandle> PipelineManager::createHandle(
        ResourceProvider* resourceProvider,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags) {
    const Caps* caps = resourceProvider->sharedContext()->caps();

    UniqueKey pipelineKey = caps->makeGraphicsPipelineKey(pipelineDesc, renderPassDesc);

    if (sk_sp<PipelineTask> task = this->findTask1(pipelineKey)) {
        // There is a task in progress to compile this Pipeline so it can't be ready yet (i.e.,
        // it isn't in the Pipeline Cache).
        return std::unique_ptr<GraphicsPipelineHandle>(new GraphicsPipelineHandle(std::move(task)));
    }

    sk_sp<GraphicsPipeline> pipeline = resourceProvider->findGraphicsPipeline(pipelineKey,
                                                                              pipelineCreationFlags);
    if (pipeline) {
        return std::unique_ptr<GraphicsPipelineHandle>(
                new GraphicsPipelineHandle(std::move(pipeline)));
    }

    sk_sp<PipelineTask> task = this->findOrCreateTask1(pipelineKey,
                                                       pipelineDesc,
                                                       renderPassDesc,
                                                       pipelineCreationFlags);

    return std::unique_ptr<GraphicsPipelineHandle>(new GraphicsPipelineHandle(std::move(task)));
}

sk_sp<GraphicsPipeline> PipelineManager::tradeIn(GraphicsPipelineHandle* handle) {
    if (std::holds_alternative<sk_sp<GraphicsPipeline>>(handle->fTaskOrPipeline)) {
        return std::get<sk_sp<GraphicsPipeline>>(handle->fTaskOrPipeline);
    }

    // Since 'fTaskOrPipeline' doesn't hold a pipeline it must not have existed when the handle
    // was created so a compilation task must've been created to compile it
    sk_sp<PipelineTask> task = std::get<sk_sp<PipelineTask>>(handle->fTaskOrPipeline);

    if (!task->fCompleted1) {
        task->wait1();
    }

    return task->fPipeline1;
}

} // namespace skgpu::graphite
