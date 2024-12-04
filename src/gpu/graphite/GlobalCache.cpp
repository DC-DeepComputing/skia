/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/GlobalCache.h"

#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Sampler.h"

namespace skgpu::graphite {

GlobalCache::GlobalCache()
        : fGraphicsPipelineCache(256)  // TODO: find a good value for these limits
        , fComputePipelineCache(256) {}

GlobalCache::~GlobalCache() {
    // These should have been cleared out earlier by deleteResources().
    SkDEBUGCODE(SkAutoSpinlock lock{ fSpinLock });
    SkASSERT(fGraphicsPipelineCache.count() == 0);
    SkASSERT(fComputePipelineCache.count() == 0);
    SkASSERT(fStaticResource.empty());
}

void GlobalCache::deleteResources() {
    SkAutoSpinlock lock{fSpinLock};

    for (int i = 0; i < kNumDynamicSamplers; ++i) {
        fDynamicSamplers[i] = nullptr;
    }

    fGraphicsPipelineCache.reset();
    fComputePipelineCache.reset();
    fStaticResource.clear();
}

bool GlobalCache::initializeDynamicSamplers(ResourceProvider* resourceProvider, const Caps* caps) {
    SkAutoSpinlock lock{fSpinLock};

    SkASSERT(fDynamicSamplers[0] == nullptr); // Must not already be initialized

    static constexpr SkTileMode kTileModes[] = {SkTileMode::kClamp,
                                                SkTileMode::kRepeat,
                                                SkTileMode::kMirror,
                                                SkTileMode::kDecal};
    // Manually unroll the SkSamplingOptions that can be dynamic samplers to avoid nesting so many
    // loops to create SamplerDescs. Cubic SkSamplingOptions do not need to contribute to this list.
    // TODO: Support anisotropic filters
    static constexpr SkSamplingOptions kSamplingOptions[] = {
        {SkFilterMode::kNearest, SkMipmapMode::kNone},
        {SkFilterMode::kLinear,  SkMipmapMode::kNone},
        {SkFilterMode::kNearest, SkMipmapMode::kNearest},
        {SkFilterMode::kLinear,  SkMipmapMode::kNearest},
        {SkFilterMode::kNearest, SkMipmapMode::kLinear},
        {SkFilterMode::kLinear,  SkMipmapMode::kLinear}
    };

    const bool skipDecal = !caps->clampToBorderSupport();
    for (auto samplingOption : kSamplingOptions) {
        for (auto tileX : kTileModes) {
            for (auto tileY : kTileModes) {
                if (skipDecal && (tileX == SkTileMode::kDecal || tileY == SkTileMode::kDecal)) {
                    continue;
                }

                SamplerDesc dynamicDesc{samplingOption, {tileX, tileY}};
                SkASSERT(!dynamicDesc.isImmutable() && dynamicDesc.asSpan().size() == 1);
                sk_sp<Sampler> sampler =
                        resourceProvider->findOrCreateCompatibleSampler(dynamicDesc);
                if (!sampler) {
                    return false;
                }

                // We already hold the spin lock, so add directly to fStaticResource
                fStaticResource.emplace_back(sampler);
                fDynamicSamplers[dynamicDesc.desc()] = sampler.get();
            }
        }
    }

    return true;
}

sk_sp<GraphicsPipeline> GlobalCache::findGraphicsPipeline(const UniqueKey& key) {
    SkAutoSpinlock lock{fSpinLock};

    sk_sp<GraphicsPipeline>* entry = fGraphicsPipelineCache.find(key);
#if defined(GPU_TEST_UTILS)
    if (entry) { ++fStats.fGraphicsCacheHits; } else { ++fStats.fGraphicsCacheMisses; }
#endif
    return entry ? *entry : nullptr;
}

#if SK_HISTOGRAMS_ENABLED
// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
//
// LINT.IfChange(PipelineCreationRace)
enum class PipelineCreationRace {
    // The <First>Over<Second> enum names mean the first type of compilation won a compilation race
    // over the second type of compilation and ended up in the cache.
    kNormalOverNormal                 = 0, // can happen w/ multiple Recorders on different threads
    kNormalOverPrecompilation         = 1,
    kPrecompilationOverNormal         = 2,
    kPrecompilationOverPrecompilation = 3, // can happen with multiple threaded precompilation calls

    kMaxValue = kPrecompilationOverPrecompilation,
};
// LINT.ThenChange(//tools/metrics/histograms/enums.xml:SkiaPipelineCreationRace)

[[maybe_unused]] static constexpr int kPipelineCreationRaceCount =
        static_cast<int>(PipelineCreationRace::kMaxValue) + 1;
#endif // SK_HISTOGRAMS_ENABLED

sk_sp<GraphicsPipeline> GlobalCache::addGraphicsPipeline(const UniqueKey& key,
                                                         sk_sp<GraphicsPipeline> pipeline) {
    SkAutoSpinlock lock{fSpinLock};

    sk_sp<GraphicsPipeline>* entry = fGraphicsPipelineCache.find(key);
    if (!entry) {
        // No equivalent pipeline was stored in the cache between a previous call to
        // findGraphicsPipeline() that returned null (triggering the pipeline creation) and this
        // later adding to the cache.
#if defined(GPU_TEST_UTILS)
        ++fStats.fGraphicsCacheAdditions;
#endif
        entry = fGraphicsPipelineCache.insert(key, std::move(pipeline));
    } else {
#if defined(GPU_TEST_UTILS)
        // else there was a race creating the same pipeline and this thread lost, so return
        // the winner
        ++fStats.fGraphicsRaces;
#endif
#if SK_HISTOGRAMS_ENABLED
        [[maybe_unused]] int race = (*entry)->fromPrecompile() * 2 + pipeline->fromPrecompile();
        SK_HISTOGRAM_ENUMERATION("Graphite.PipelineCreationRace",
                                 race,
                                 kPipelineCreationRaceCount);
#endif
    }
    return *entry;
}

#if defined(GPU_TEST_UTILS)
int GlobalCache::numGraphicsPipelines() const {
    SkAutoSpinlock lock{fSpinLock};

    return fGraphicsPipelineCache.count();
}

void GlobalCache::resetGraphicsPipelines() {
    SkAutoSpinlock lock{fSpinLock};

    fGraphicsPipelineCache.reset();
}

void GlobalCache::forEachGraphicsPipeline(
        const std::function<void(const UniqueKey&, const GraphicsPipeline*)>& fn) {
    SkAutoSpinlock lock{fSpinLock};

    fGraphicsPipelineCache.foreach([&](const UniqueKey* k, const sk_sp<GraphicsPipeline>* v) {
        fn(*k, v->get());
    });
}

GlobalCache::PipelineStats GlobalCache::getStats() const {
    SkAutoSpinlock lock{fSpinLock};

    return fStats;
}
#endif // defined(GPU_TEST_UTILS)

sk_sp<ComputePipeline> GlobalCache::findComputePipeline(const UniqueKey& key) {
    SkAutoSpinlock lock{fSpinLock};
    sk_sp<ComputePipeline>* entry = fComputePipelineCache.find(key);
    return entry ? *entry : nullptr;
}

sk_sp<ComputePipeline> GlobalCache::addComputePipeline(const UniqueKey& key,
                                                       sk_sp<ComputePipeline> pipeline) {
    SkAutoSpinlock lock{fSpinLock};
    sk_sp<ComputePipeline>* entry = fComputePipelineCache.find(key);
    if (!entry) {
        entry = fComputePipelineCache.insert(key, std::move(pipeline));
    }
    return *entry;
}

void GlobalCache::addStaticResource(sk_sp<Resource> resource) {
    SkAutoSpinlock lock{fSpinLock};
    fStaticResource.push_back(std::move(resource));
}

} // namespace skgpu::graphite
