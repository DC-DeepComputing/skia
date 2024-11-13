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

extern skgpu::StdSteadyClock::time_point fTimeTime;

namespace skgpu::graphite {

GlobalCache::GlobalCache()
        : fGraphicsPipelineCache(2, &fMap)  // TODO: find a good value for these limits
        , fComputePipelineCache(256)
        , fMap() {}

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

sk_sp<GraphicsPipeline> GlobalCache::findGraphicsPipeline(const UniqueKey& key,
                                                          bool forPrecompile) {
    SkLog* log = this->gimme(key);

    SkAutoSpinlock lock{fSpinLock};

    sk_sp<GraphicsPipeline>* entry = fGraphicsPipelineCache.find(key);
#if defined(GPU_TEST_UTILS)
    if (entry) {
        ++fStats.fGraphicsCacheHits;
        ++fStats.fBar[(*entry)->fromPrecompile()][forPrecompile];

        log->addEvent2(GlobalCache::SkLog::EventType2::kCacheHit, forPrecompile);
    } else {
        ++fStats.fGraphicsCacheMisses;
        ++fStats.fBaz[forPrecompile];

        log->addEvent2(GlobalCache::SkLog::EventType2::kCacheMiss, forPrecompile);
    }
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
    SkLog* log = this->gimme(key);

    SkAutoSpinlock lock{fSpinLock};

    sk_sp<GraphicsPipeline>* entry = fGraphicsPipelineCache.find(key);
    if (!entry) {
        // No equivalent pipeline was stored in the cache between a previous call to
        // findGraphicsPipeline() that returned null (triggering the pipeline creation) and this
        // later adding to the cache.
#if defined(GPU_TEST_UTILS)
        ++fStats.fGraphicsCacheAdditions;
#endif
        bool fromPrecompile = pipeline->fromPrecompile();
        entry = fGraphicsPipelineCache.insert(key, std::move(pipeline));
        log->addEvent2(GlobalCache::SkLog::EventType2::kAdded, fromPrecompile);
        log->setType(fromPrecompile ? GlobalCache::SkLog::Type::kPrecompile
                                    : GlobalCache::SkLog::Type::kNormal);
    } else {
#if defined(GPU_TEST_UTILS)
        // else there was a race creating the same pipeline and this thread lost, so return
        // the winner
        ++fStats.fTotalGraphicsRaces;
        ++fStats.fFoo[(*entry)->fromPrecompile()][pipeline->fromPrecompile()];

        static const GlobalCache::SkLog::EventType2 kCombos[4] = {
                GlobalCache::SkLog::EventType2::kNWonRaceOverN,
                GlobalCache::SkLog::EventType2::kNWonRaceOverP,
                GlobalCache::SkLog::EventType2::kPWonRaceOverN,
                GlobalCache::SkLog::EventType2::kPWonRaceOverP,
        };

        int race = (*entry)->fromPrecompile() * 2 + pipeline->fromPrecompile();
        log->addEvent2(kCombos[race], pipeline->fromPrecompile());
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

namespace {

[[maybe_unused]] auto micros_since = [](skgpu::StdSteadyClock::time_point start) {
    skgpu::StdSteadyClock::duration elapsed = skgpu::StdSteadyClock::now() - start;
    return std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
};

}

void GlobalCache::SkLog::addEvent2(EventType2 type, bool forPrecompile) {
    uint32_t curEvent = fCurEvent1.fetch_add(1, std::memory_order_relaxed);
    SkASSERT_RELEASE(curEvent < kMaxNumMsgs);

    int ancillary = -1;
    if (type == EventType2::kAdded) {
        SkASSERT(fType == Type::kUnset);
        SkASSERT(fNumHits == 0);
        fType = forPrecompile ? Type::kPrecompile : Type::kNormal;
    } else if (type == EventType2::kCacheHit) {
        fNumHits++;
    } else if (type == EventType2::kPurged) {
        if (fNumHits == 0 && fType == Type::kPrecompile) {
            SkDebugf("------------------ Wasted!!!\n");
        }
        fType = Type::kUnset;
        ancillary = fNumHits;
        fNumHits = 0;
    }

    fEvents[curEvent] = { type, micros_since(fTimeTime), forPrecompile, ancillary };
}

GlobalCache::SkLog* GlobalCache::gimme(const UniqueKey& key) {
    SkAutoSpinlock lock{fSpinLock};

    sk_sp<SkLog>* log = fMap.find(key);
    if (!log) {
        sk_sp<SkLog> tmp = sk_make_sp<SkLog>(key);
        log = fMap.set(std::move(tmp));
    }

    return (*log).get();
}

void GlobalCache::dump() {
    SkAutoSpinlock lock{fSpinLock};

    int count = 0;
    fMap.foreach([&count](sk_sp<SkLog>* log) {
        uint32_t numEvents = (*log)->fCurEvent1.fetch_add(0, std::memory_order_relaxed);
        numEvents = std::min(numEvents, SkLog::kMaxNumMsgs);

        SkDebugf("%2d: %c %10u ", count++, SkLog::Name2((*log)->fType), (*log)->fKey.hash());
        for (uint32_t i = 0; i < numEvents; ++i) {
            SkDebugf("[%5lld %s",
                     (*log)->fEvents[i].fTime,
                     SkLog::Name((*log)->fEvents[i].fType, (*log)->fType));
            if ((*log)->fEvents[i].fType == SkLog::EventType2::kPurged) {
                SkDebugf("-%d", (*log)->fEvents[i].fAncillary);
            }
            SkDebugf(" %c]", (*log)->fEvents[i].fForPrecompile ? 'P' : 'N');
        }
        SkDebugf("\n");
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
