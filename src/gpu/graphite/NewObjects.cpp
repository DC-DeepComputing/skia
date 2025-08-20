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

    pipelineManager->removeTask(task);
}

PipelineManager::PipelineManager(SkExecutor* executor) {
    if (executor) {
        fTaskGroup = std::make_unique<SkTaskGroup>(*executor);
    }
}
PipelineManager::~PipelineManager() {}

void PipelineManager::addTask(const UniqueKey& pipelineKey,
                              const GraphicsPipelineDesc& pipelineDesc,
                              const RenderPassDesc& renderPassDesc,
                              SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags) {
    SkAutoSpinlock lock{fSpinLock};

    sk_sp<PipelineTask>* task = fActiveTasks.find(pipelineKey);
    if (task) {
        return;
    }

    sk_sp<PipelineTask> newTask = sk_make_sp<PipelineTask>(pipelineKey,
                                                           pipelineDesc,
                                                           renderPassDesc,
                                                           pipelineCreationFlags);
    fActiveTasks.set(std::move(newTask));
}

void PipelineManager::removeTask(PipelineTask* task) {
    SkAutoSpinlock lock{fSpinLock};

    fActiveTasks.remove(task->fPipelineKey);
}

sk_sp<PipelineManager::PipelineTask> PipelineManager::findTask(const UniqueKey& pipelineKey) {
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

    if (sk_sp<PipelineTask> task = this->findTask(pipelineKey)) {
        // There is a task in progress to compile this Pipeline so it can't be ready yet
        return std::unique_ptr<GraphicsPipelineHandle>(new GraphicsPipelineHandle(pipelineKey));
    }

    sk_sp<GraphicsPipeline> pipeline = resourceProvider->findGraphicsPipeline(pipelineKey,
                                                                              pipelineCreationFlags);
    if (pipeline) {
        return std::unique_ptr<GraphicsPipelineHandle>(
                new GraphicsPipelineHandle(std::move(pipeline)));
    }

    sk_sp<PipelineTask> task = this->addTask(pipelineKey,
                                             pipelineDesc,
                                             renderPassDesc,
                                             pipelineCreationFlags);

    return std::unique_ptr<GraphicsPipelineHandle>(new GraphicsPipelineHandle(pipelineKey));
}

sk_sp<GraphicsPipeline> PipelineManager::tradeIn(ResourceProvider* resourceProvider,
                                                 GraphicsPipelineHandle* handle) {
    if (handle->fPipeline2) {
        return handle->fPipeline2;
    }

    SkASSERT(handle->fPipelineKey.isValid());

    sk_sp<PipelineTask> task = this->findTask(handle->fPipelineKey);
    if (task) {
        task->wait();
    }

    sk_sp<GraphicsPipeline> pipeline = resourceProvider->findGraphicsPipeline(
            handle->fPipelineKey,
            PipelineCreationFlags::kNone); // This could mess up logging!
    if (pipeline) {
        handle->fPipeline2 = pipeline; // lock this Pipeline in memory
        return pipeline;
    }

    // Since fPipeline2 was null at the beginning this means the Pipeline didn't exist when
    // the Handle was created so a PipelineTask must've been created to compile it.
    // Unfortunately, if that task had completed long before we got to this point there
    // was nothing to lock it in the cache and it could've been pushed out. In that case
    // all of these could be null:
    //    handle->fPipeline2
    //    task
    //    pipeline
    // To deal with this we need to create a Task, hold on to it and immediately wait on it.

//    this->addTask(handle->fPipelineKey, ??, ??, ??);

    return nullptr;
}

} // namespace skgpu::graphite
