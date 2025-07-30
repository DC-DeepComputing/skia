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

namespace skgpu::graphite {

Compilinator::Compilinator(SkExecutor* executor) {
    if (executor) {
        fTaskGroup = std::make_unique<SkTaskGroup>(*executor);
    }
}

Compilinator::~Compilinator() {}

static void resolve_handle(ResourceProvider* resourceProvider,
                           const RuntimeEffectDictionary* runtimeDict,
                           GraphicsPipelineHandle* handle,
                           const GraphicsPipelineDesc& pipelineDesc,
                           const RenderPassDesc& renderPassDesc) {
    sk_sp<GraphicsPipeline> pipeline = resourceProvider->findOrCreateGraphicsPipeline(
            runtimeDict,
            pipelineDesc,
            renderPassDesc);
    if (!pipeline) {
        return;
    }

    handle->update(std::move(pipeline));
}


void Compilinator::compilinate(ResourceProvider* resourceProvider,
                               const RuntimeEffectDictionary* runtimeDict,
                               GraphicsPipelineHandle* handle,
                               const GraphicsPipelineDesc& pipelineDesc,
                               const RenderPassDesc& renderPassDesc,
                               SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags) {
    if (fTaskGroup) {
        auto compileTask = [resourceProvider, runtimeDict, handle, pipelineDesc, renderPassDesc] {
            resolve_handle(resourceProvider, runtimeDict, handle, pipelineDesc, renderPassDesc);
        };

        fTaskGroup->add(std::move(compileTask));
    } else {
        resolve_handle(resourceProvider, runtimeDict, handle, pipelineDesc, renderPassDesc);
    }
}

PipelineManager::PipelineManager() {}
PipelineManager::~PipelineManager() {}

GraphicsPipelineHandle* PipelineManager::createHandle(sk_sp<GraphicsPipeline> pipeline,
                                                      bool autoDelete) {
    SkAutoSpinlock lock{fSpinLock};

    SkASSERT(!autoDelete || !pipeline);
    GraphicsPipelineHandle* tmp = new GraphicsPipelineHandle(std::move(pipeline), autoDelete);
    fMemoryToDeallocate.push_back(tmp);

    return tmp;
}

} // namespace skgpu::graphite
