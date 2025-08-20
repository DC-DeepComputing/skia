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

GraphicsPipelineHandle::GraphicsPipelineHandle(sk_sp<GraphicsPipeline> pipeline)
        : fPipeline2(std::move(pipeline)) {}

GraphicsPipelineHandle::GraphicsPipelineHandle(sk_sp<PipelineTask> task)
        : fPipelineTask(std::move(task)) {}

namespace {


} // anonymous namespace

void PipelineManager::Doit(ResourceProvider* resourceProvider,
                           const RuntimeEffectDictionary* runtimeDict,
                           PipelineManager* pipelineManager,
                           PipelineTask* task) {
    sk_sp<GraphicsPipeline> pipeline = resourceProvider->findOrCreateGraphicsPipeline(
            runtimeDict,
            task->fPipelineKey,
            task->fGraphicsPipelineDesc,
            task->fRenderPassDesc);
    task->fPipeline1 = pipeline;

    task->signalComplete();

    pipelineManager->removeTask1(task);
}

PipelineManager::PipelineManager(SkExecutor* executor) {
    if (executor) {
        fTaskGroup = std::make_unique<SkTaskGroup>(*executor);
    }
}
PipelineManager::~PipelineManager() {}

sk_sp<PipelineTask> PipelineManager::addTask1(
        const UniqueKey& pipelineKey,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags) {
    SkAutoSpinlock lock{fSpinLock};

    sk_sp<PipelineTask>* task = fActiveTasks.find(pipelineKey);
    if (task) {
        return *task;
    }

    sk_sp<PipelineTask> newTask = sk_make_sp<PipelineTask>(pipelineKey,
                                                           pipelineDesc,
                                                           renderPassDesc,
                                                           pipelineCreationFlags);
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

void PipelineManager::startTask(ResourceProvider* resourceProvider,
                                const RuntimeEffectDictionary* runtimeDict,
                                PipelineTask* task) {
    if (fTaskGroup) {
        auto compileTask = [resourceProvider, runtimeDict, this, task] {
            Doit(resourceProvider, runtimeDict, this, task);
        };

        fTaskGroup->add(std::move(compileTask));
    } else {
        Doit(resourceProvider, runtimeDict, this, task);
    }
}

std::unique_ptr<GraphicsPipelineHandle> PipelineManager::createHandle(
        ResourceProvider* resourceProvider,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags) {
    const SharedContext* sharedContext = resourceProvider->sharedContext();

    UniqueKey pipelineKey = sharedContext->caps()->makeGraphicsPipelineKey(pipelineDesc,
                                                                           renderPassDesc);

    if (sk_sp<PipelineTask> task = this->findTask1(pipelineKey)) {
        // There is a task in progress to compile this Pipeline so it can't be ready yet
        return std::unique_ptr<GraphicsPipelineHandle>(new GraphicsPipelineHandle(std::move(task)));
    }

    sk_sp<GraphicsPipeline> pipeline = resourceProvider->findGraphicsPipeline(pipelineKey,
                                                                              pipelineCreationFlags);
    if (pipeline) {
        return std::unique_ptr<GraphicsPipelineHandle>(
                new GraphicsPipelineHandle(std::move(pipeline)));
    }

    sk_sp<PipelineTask> task = this->addTask1(pipelineKey,
                                             pipelineDesc,
                                             renderPassDesc,
                                             pipelineCreationFlags);

    return std::unique_ptr<GraphicsPipelineHandle>(new GraphicsPipelineHandle(std::move(task)));
}

sk_sp<GraphicsPipeline> PipelineManager::tradeIn(GraphicsPipelineHandle* handle) {
    if (handle->fPipeline2) {
        return handle->fPipeline2;
    }

    // Since 'fPipeline' is null it must not have existed when the handle was created so
    // a compilation task must've been created to compile it
    SkASSERT(handle->fPipelineTask);

    handle->fPipelineTask->wait();

    // Once completed a compilation task should've generated a Pipeline and locked it in the
    // cache (via its 'fPipeline' ref). If 'fPipeline' is null compilation failed and some
    // draws will need to be dropped!
    SkASSERT(handle->fPipelineTask->fPipeline1);
    return handle->fPipelineTask->fPipeline1;
}

} // namespace skgpu::graphite
