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

Compilinator::Compilinator(SkExecutor* executor) {
    if (executor) {
        fTaskGroup = std::make_unique<SkTaskGroup>(*executor);
    }
}

Compilinator::~Compilinator() {}

void Compilinator::compilinate(ResourceProvider* resourceProvider,
                               const RuntimeEffectDictionary* runtimeDict,
                               GraphicsPipelineHandle* handle,
                               const GraphicsPipelineDesc& pipelineDesc,
                               const RenderPassDesc& renderPassDesc,
                               SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags) {
    if (fTaskGroup) {
        auto compileTask = [resourceProvider, runtimeDict, handle, pipelineDesc, renderPassDesc] {
            ResolveHandle(resourceProvider, runtimeDict, handle, pipelineDesc, renderPassDesc);
        };

        fTaskGroup->add(std::move(compileTask));
    } else {
        ResolveHandle(resourceProvider, runtimeDict, handle, pipelineDesc, renderPassDesc);
    }
}

void Compilinator::ResolveHandle(ResourceProvider* resourceProvider,
                                 const RuntimeEffectDictionary* runtimeDict,
                                 GraphicsPipelineHandle* handle,
                                 const GraphicsPipelineDesc& pipelineDesc,
                                 const RenderPassDesc& renderPassDesc) {
    sk_sp<GraphicsPipeline> pipeline = resourceProvider->findOrCreateGraphicsPipeline(
            runtimeDict,
            pipelineDesc,
            renderPassDesc);

    if (!pipeline) {
        handle->markFailed();
        return;
    }

    handle->update1(std::move(pipeline));
}

PipelineManager::PipelineManager() {}
PipelineManager::~PipelineManager() {}

void PipelineManager::addTask(const UniqueKey& pipelineKey,
                              const GraphicsPipelineDesc& pipelineDesc,
                              const RenderPassDesc& renderPassDesc,
                              SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags) {
    SkAutoSpinlock lock{fSpinLock};

    Task1** entry = fActiveTasks.find(pipelineKey);
    if (entry) {
        return;
    }

    Task1* newTask = new Task1(pipelineDesc, renderPassDesc, pipelineCreationFlags);
    fActiveTasks.set(pipelineKey, newTask);
}

void PipelineManager::waitOnTask(const UniqueKey& pipelineKey) {
    SkAutoSpinlock lock{fSpinLock};

}

void PipelineManager::taskFinished(const UniqueKey& pipelineKey,
                                   sk_sp<GraphicsPipeline>) {
    SkAutoSpinlock lock{fSpinLock};

}

std::unique_ptr<GraphicsPipelineHandle> PipelineManager::createHandle(
        ResourceProvider* resourceProvider,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags) {
    const SharedContext* sharedContext = resourceProvider->sharedContext();

    UniqueKey pipelineKey = sharedContext->caps()->makeGraphicsPipelineKey(pipelineDesc,
                                                                           renderPassDesc);

    sk_sp<GraphicsPipeline> pipeline = resourceProvider->findGraphicsPipeline(pipelineKey,
                                                                              pipelineCreationFlags);
    if (pipeline) {
        return std::unique_ptr<GraphicsPipelineHandle>(
                new GraphicsPipelineHandle(std::move(pipeline)));
    }

    this->addTask(pipelineKey, pipelineDesc, renderPassDesc, pipelineCreationFlags);

    return std::unique_ptr<GraphicsPipelineHandle>(new GraphicsPipelineHandle(pipelineKey));
}

sk_sp<GraphicsPipeline> PipelineManager::tradeIn(ResourceProvider* resourceProvider,
                                                 GraphicsPipelineHandle* handle) {
    if (handle->fPipeline) {
        return handle->fPipeline;
    }

    SkASSERT(handle->fPipelineKey.isValid());

    for (int i = 0; i < 3; ++i) {
        this->waitOnTask(handle->fPipelineKey);

        sk_sp<GraphicsPipeline> pipeline = resourceProvider->findGraphicsPipeline(
                handle->fPipelineKey,
                PipelineCreationFlags::kNone); // This could mess up logging!
        if (pipeline) {
            handle->fPipeline = pipeline; // lock this Pipeline in memory
            return pipeline;
        }

//        this->addTask(handle->fPipelineKey, ??, ??, ??);
    }

    return nullptr;
}

} // namespace skgpu::graphite
