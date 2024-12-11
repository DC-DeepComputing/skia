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

#if defined(SK_VERBOSE_PIPELINE_LIFETIME_LOGGING)
namespace {

uint32_t next_id() {
    static std::atomic<uint32_t> nextId{0};
    // Not worried about overflow since we don't expect that many GraphicsPipelines.
    // Even if it wraps around to 0, this is solely for debug logging.
    return nextId.fetch_add(1, std::memory_order_relaxed);
}

} // anonymous namespce
#endif

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
    SkAutoSpinlock lock{ fSpinLock };

    fGraphicsPipelineCache.reset();
    fComputePipelineCache.reset();
    fStaticResource.clear();
}

sk_sp<GraphicsPipeline> GlobalCache::findGraphicsPipeline(
        const UniqueKey& key
#if defined(SK_VERBOSE_PIPELINE_LIFETIME_LOGGING)
        , SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags,
        uint32_t *futureID
#endif
        ) {

#if defined(SK_VERBOSE_PIPELINE_LIFETIME_LOGGING)
    bool forPrecompile =
            SkToBool(pipelineCreationFlags & PipelineCreationFlags::kForPrecompilation);
#endif

    SkAutoSpinlock lock{fSpinLock};

    sk_sp<GraphicsPipeline>* entry = fGraphicsPipelineCache.find(key);
    if (entry) {
#if defined(GPU_TEST_UTILS)
        ++fStats.fGraphicsCacheHits;
#endif

#if defined(SK_VERBOSE_PIPELINE_LIFETIME_LOGGING)
        static const char* kNames[2] = { "CacheHitForN", "CacheHitForP" };
        TRACE_EVENT_INSTANT2("skia.gpu",
                             TRACE_STR_STATIC(kNames[forPrecompile]),
                             TRACE_EVENT_SCOPE_THREAD,
                             "key", key.hash(),
                             "pipelineID", (*entry)->getPipelineInfo().fID);
#endif

        return *entry;
    } else {
#if defined(GPU_TEST_UTILS)
        ++fStats.fGraphicsCacheMisses;
#endif

#if defined(SK_VERBOSE_PIPELINE_LIFETIME_LOGGING)
        if (futureID) {
            *futureID = next_id();
        }

        static const char* kNames[2] = { "CacheMissForN", "CacheMissForP" };
        TRACE_EVENT_INSTANT2("skia.gpu",
                             TRACE_STR_STATIC(kNames[forPrecompile]),
                             TRACE_EVENT_SCOPE_THREAD,
                             "key", key.hash(),
                             "pipelineID", *futureID);
#endif

        return nullptr;
    }
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
        entry = fGraphicsPipelineCache.insert(key, std::move(pipeline));

#if defined(GPU_TEST_UTILS)
        ++fStats.fGraphicsCacheAdditions;
#endif

#if defined(SK_VERBOSE_PIPELINE_LIFETIME_LOGGING)
        static const char* kNames[2] = { "AddedN", "AddedP" };
        TRACE_EVENT_INSTANT2("skia.gpu",
                             TRACE_STR_STATIC(kNames[pipeline->fromPrecompile1()]),
                             TRACE_EVENT_SCOPE_THREAD,
                             "key", key.hash(),
                             "pipelineID", pipeline->getPipelineInfo().fID);
#endif
    } else {
#if defined(GPU_TEST_UTILS)
        // else there was a race creating the same pipeline and this thread lost, so return
        // the winner
        ++fStats.fGraphicsRaces;
#endif

#if SK_HISTOGRAMS_ENABLED || defined(SK_VERBOSE_PIPELINE_LIFETIME_LOGGING)
        [[maybe_unused]] int race = (*entry)->fromPrecompile1() * 2 + pipeline->fromPrecompile1();
#endif

#if defined(SK_VERBOSE_PIPELINE_LIFETIME_LOGGING)
        static const char* kNames[4] = {
                "NWonRaceOverN",
                "NWonRaceOverP",
                "PWonRaceOverN",
                "PWonRaceOverP"
        };
        TRACE_EVENT_INSTANT2("skia.gpu",
                             TRACE_STR_STATIC(kNames[race]),
                             TRACE_EVENT_SCOPE_THREAD,
                             "key", key.hash(),
                             "pipelineID", pipeline->getPipelineInfo().fID);  // The losing pipelineID
#endif

#if SK_HISTOGRAMS_ENABLED
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
