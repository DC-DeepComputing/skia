/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <condition_variable>
#include <mutex>
#include <optional>

#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkOpts.h"
#include "src/core/SkTaskGroup.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkPipelineState.h"
#include "src/gpu/vk/GrVkPipelineStateBuilder.h"
#include "src/gpu/vk/GrVkResourceProvider.h"

#ifdef SK_DEBUG
// Display pipeline state cache usage
static const bool c_DisplayVkPipelineCache{false};
#endif

struct GrVkResourceProvider::PipelineStateCache::Entry {
public:
    Entry(GrVkGpu* gpu) : fGpu(gpu) {}

    ~Entry() {
        waitForReady();
        if (*fPipelineState) {
            (*fPipelineState)->freeGPUResources(fGpu);
        }
    }

    GrVkPipelineState* pipelineState() const {
        waitForReady();
        return fPipelineState ? fPipelineState->get() : nullptr;
    }

    void setPipelineState(GrVkPipelineState* pipelineState) {
        std::unique_lock uniqueLock(fMutex);
        SkASSERT(!fPipelineState);
        fPipelineState.emplace(pipelineState);
        fReadyConditionVariable.notify_all();
    }

private:
    void waitForReady() const {
        std::unique_lock uniqueLock(fMutex);
        while (!fPipelineState) {
            fReadyConditionVariable.wait(uniqueLock);
        }
    }

    mutable std::mutex fMutex;
    mutable std::condition_variable fReadyConditionVariable;
    GrVkGpu* fGpu;
    std::optional<std::unique_ptr<GrVkPipelineState>> fPipelineState;
};

GrVkResourceProvider::PipelineStateCache::PipelineStateCache(GrVkGpu* gpu)
    : fMap(gpu->getContext()->priv().options().fRuntimeProgramCacheSize)
    , fGpu(gpu) {
}

GrVkResourceProvider::PipelineStateCache::~PipelineStateCache() {
    SkASSERT(0 == fMap.count());
    // dump stats
#ifdef SK_DEBUG
    if (c_DisplayVkPipelineCache) {
        using CacheResult = Stats::ProgramCacheResult;

        int misses = fStats.numInlineProgramCacheResult(CacheResult::kMiss) +
                     fStats.numPreProgramCacheResult(CacheResult::kMiss);

        int total = misses + fStats.numInlineProgramCacheResult(CacheResult::kHit) +
                             fStats.numPreProgramCacheResult(CacheResult::kHit);

        SkDebugf("--- Pipeline State Cache ---\n");
        SkDebugf("Total requests: %d\n", total);
        SkDebugf("Cache misses: %d\n", misses);
        SkDebugf("Cache miss %%: %f\n", (total > 0) ? 100.f * misses / total : 0.0f);
    }
#endif
}

void GrVkResourceProvider::PipelineStateCache::release() {
    fMap.reset();
}

GrVkPipelineState* GrVkResourceProvider::PipelineStateCache::findOrCreatePipelineState(
        GrRenderTarget* renderTarget,
        const GrProgramInfo& programInfo,
        VkRenderPass compatibleRenderPass,
        bool overrideSubpassForResolveLoad) {
#ifdef SK_DEBUG
    if (programInfo.isStencilEnabled()) {
        SkASSERT(renderTarget->getStencilAttachment(programInfo.numSamples() > 1));
        SkASSERT(renderTarget->numStencilBits(programInfo.numSamples() > 1) == 8);
        SkASSERT(renderTarget->getStencilAttachment(programInfo.numSamples() > 1)->numSamples() ==
                 programInfo.numSamples());
    }
#endif

    auto flags = overrideSubpassForResolveLoad
            ? GrCaps::ProgramDescOverrideFlags::kVulkanHasResolveLoadSubpass
            : GrCaps::ProgramDescOverrideFlags::kNone;

    GrProgramDesc desc = fGpu->caps()->makeDesc(renderTarget, programInfo, flags);
    if (!desc.isValid()) {
        GrCapsDebugf(fGpu->caps(), "Failed to build vk program descriptor!\n");
        return nullptr;
    }

    Stats::ProgramCacheResult stat;
    auto tmp = this->findOrCreatePipelineStateImpl(
            desc, programInfo, compatibleRenderPass, overrideSubpassForResolveLoad, &stat, false);
    if (!tmp) {
        fStats.incNumInlineCompilationFailures();
    } else {
        fStats.incNumInlineProgramCacheResult(stat);
    }

    return tmp;
}

void GrVkResourceProvider::PipelineStateCache::preparePipelineState(
        GrRenderTarget* renderTarget,
        const GrProgramInfo& programInfo,
        VkRenderPass compatibleRenderPass,
        bool overrideSubpassForResolveLoad) {
#ifdef SK_DEBUG
    if (programInfo.isStencilEnabled()) {
        SkASSERT(renderTarget->getStencilAttachment(programInfo.numSamples() > 1));
        SkASSERT(renderTarget->numStencilBits(programInfo.numSamples() > 1) == 8);
        SkASSERT(renderTarget->getStencilAttachment(programInfo.numSamples() > 1)->numSamples() ==
                 programInfo.numSamples());
    }
#endif

    auto flags = overrideSubpassForResolveLoad
                         ? GrCaps::ProgramDescOverrideFlags::kVulkanHasResolveLoadSubpass
                         : GrCaps::ProgramDescOverrideFlags::kNone;

    GrProgramDesc desc = fGpu->caps()->makeDesc(renderTarget, programInfo, flags);
    if (!desc.isValid()) {
        GrCapsDebugf(fGpu->caps(), "Failed to build vk program descriptor!\n");
        return;
    }

    Stats::ProgramCacheResult stat;
    this->findOrCreatePipelineStateImpl(
            desc, programInfo, compatibleRenderPass, overrideSubpassForResolveLoad, &stat, true);
}

GrVkPipelineState* GrVkResourceProvider::PipelineStateCache::findOrCreatePipelineStateImpl(
        const GrProgramDesc& desc,
        const GrProgramInfo& programInfo,
        VkRenderPass compatibleRenderPass,
        bool overrideSubpassForResolveLoad,
        Stats::ProgramCacheResult* stat,
        bool prepare) {
    if (stat) {
        *stat = Stats::ProgramCacheResult::kHit;
    }

    SkTaskGroup* taskGroup = fGpu->getContext()->priv().getTaskGroup();
    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (!entry) {
        if (stat) {
            *stat = Stats::ProgramCacheResult::kMiss;
        }

        if (taskGroup && prepare) {
            entry = fMap.insert(desc, std::make_unique<Entry>(fGpu));
            auto* entryPtr = entry->get();
            taskGroup->add([=] {
                GrVkPipelineState* pipelineState(GrVkPipelineStateBuilder::CreatePipelineState(
                        fGpu,
                        desc,
                        programInfo,
                        compatibleRenderPass,
                        overrideSubpassForResolveLoad));
                entryPtr->setPipelineState(pipelineState);
            });
            return nullptr;
        }

        GrVkPipelineState* pipelineState(GrVkPipelineStateBuilder::CreatePipelineState(
                fGpu, desc, programInfo, compatibleRenderPass, overrideSubpassForResolveLoad));
        if (!pipelineState) {
            return nullptr;
        }

        entry = fMap.insert(desc, std::make_unique<Entry>(fGpu));
        (*entry)->setPipelineState(pipelineState);
    }

    return (*entry)->pipelineState();
}
