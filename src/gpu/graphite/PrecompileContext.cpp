/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/PrecompileContext.h"

#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SharedContext.h"

#include "include/gpu/graphite/precompile/BlobOStuff.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"

namespace skgpu::graphite {

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(&fSingleOwner)

PrecompileContext::~PrecompileContext() {
    ASSERT_SINGLE_OWNER
}

PrecompileContext::PrecompileContext(sk_sp<SharedContext> sharedContext)
    : fSharedContext(sharedContext) {

    // ResourceProviders are not thread-safe. Here we create a ResourceProvider
    // specifically for the thread on which precompilation will occur.
    static constexpr size_t kEmptyBudget = 0;
    fResourceProvider =
            fSharedContext->makeResourceProvider(&fSingleOwner, SK_InvalidGenID, kEmptyBudget);
}

void PrecompileContext::purgePipelinesNotUsedInMs(std::chrono::milliseconds msNotUsed) {
    ASSERT_SINGLE_OWNER

    auto purgeTime = skgpu::StdSteadyClock::now() - msNotUsed;

    fSharedContext->globalCache()->purgePipelinesNotUsedSince(purgeTime);
}


bool PrecompileContext::androidSpecificPrecompile(sk_sp<SkData> androidStyleKey) {
    //$$
    auto rtEffectDict = std::make_unique<RuntimeEffectDictionary>(false);

    std::unique_ptr<BlobOStuff> blob = BlobOStuff::MakeFromData(androidStyleKey.get(),
                                                                this->fSharedContext->shaderCodeDictionary9(),
                                                                this->fSharedContext->caps());

    sk_sp<GraphicsPipeline> pipeline = fResourceProvider->findOrCreateGraphicsPipeline99(
            rtEffectDict.get(),
            blob->gpd(),
            blob->rpd(),
            PipelineCreationFlags::kForPrecompilation);
    if (!pipeline) {
        SKGPU_LOG_W("Failed to create GraphicsPipeline in precompile!");
        return false;
    }

    return true;
}

} // namespace skgpu::graphite
