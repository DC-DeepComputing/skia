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
        : fGraphicsPipelineCache(2, &fLogHash)  // TODO: find a good value for these limits
        , fComputePipelineCache(256)
        , fLogHash() {}

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
        const UniqueKey& key,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags) {
    SkLog* log = this->gimme(key);

    bool forPrecompile = SkToBool(pipelineCreationFlags & PipelineCreationFlags::kForPrecompilation);

    SkAutoSpinlock lock{fSpinLock};

    sk_sp<GraphicsPipeline>* entry = fGraphicsPipelineCache.find(key);
#if defined(GPU_TEST_UTILS)
    if (entry) {
        ++fStats.fGraphicsCacheHits;
        ++fStats.fBar[(*entry)->fromPrecompile()][forPrecompile];

        auto event = forPrecompile ? GlobalCache::SkLog::EventType2::kCacheHitForP
                                   : GlobalCache::SkLog::EventType2::kCacheHitForN;
        log->addEvent2(event);

        TRACE_EVENT_INSTANT2("skia.gpu", "Hit??", TRACE_EVENT_SCOPE_THREAD,
                             "key", key.dataSize(),
                             "extra", 0);
    } else {
        ++fStats.fGraphicsCacheMisses;
        ++fStats.fBaz[forPrecompile];

        auto event = forPrecompile ? GlobalCache::SkLog::EventType2::kCacheMissForP
                                   : GlobalCache::SkLog::EventType2::kCacheMissForN;
        log->addEvent2(event);

        TRACE_EVENT_INSTANT2("skia.gpu", "Miss??", TRACE_EVENT_SCOPE_THREAD,
                             "key", key.dataSize(),
                             "extra", 0);
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
        auto event = pipeline->fromPrecompile() ? GlobalCache::SkLog::EventType2::kAddedP
                                                : GlobalCache::SkLog::EventType2::kAddedN;
        entry = fGraphicsPipelineCache.insert(key, std::move(pipeline));
        log->addEvent2(event);

        TRACE_EVENT_INSTANT2("skia.gpu", "Added??", TRACE_EVENT_SCOPE_THREAD,
                             "key", key.dataSize(),
                             "extra", 0);
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
        log->addEvent2(kCombos[race]);

        TRACE_EVENT_INSTANT2("skia.gpu", "WonRace??", TRACE_EVENT_SCOPE_THREAD,
                             "key", key.dataSize(),
                             "extra", 0);
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

void GlobalCache::SkLog::addEvent2(EventType2 type) {
    uint32_t curEvent = fCurEvent1.fetch_add(1, std::memory_order_relaxed);
    SkASSERT_RELEASE(curEvent < kMaxNumMsgs);

    int ancillary = -1;
    if (type == EventType2::kAddedN || type == EventType2::kAddedP) {
        SkASSERT(fNumHits == 0);
    } else if (type == EventType2::kCacheHitForN || type == EventType2::kCacheHitForP) {
        fNumHits++;
    } else if (type == EventType2::kPurgedN || type == EventType2::kPurgedP) {
        if (fNumHits == 0 && type == EventType2::kPurgedP) {
            SkDebugf("------------------ Wasted!!!\n");
        }
        ancillary = fNumHits;
        fNumHits = 0;
    }

    fEvents[curEvent] = { type, micros_since(fTimeTime), ancillary };
}

GlobalCache::SkLog* GlobalCache::gimme(const UniqueKey& key) {
    SkAutoSpinlock lock{fSpinLock};

    sk_sp<SkLog>* log = fLogHash.find(key);
    if (!log) {
        sk_sp<SkLog> tmp = sk_make_sp<SkLog>(key);
        log = fLogHash.set(std::move(tmp));
    }

    return (*log).get();
}

void GlobalCache::dump() {
    SkAutoSpinlock lock{fSpinLock};

    int count = 0;
    fLogHash.foreach([&count](sk_sp<SkLog>* log) {
        uint32_t numEvents = (*log)->fCurEvent1.fetch_add(0, std::memory_order_relaxed);
        numEvents = std::min(numEvents, SkLog::kMaxNumMsgs);

        SkDebugf("%2d: %10u ", count++, (*log)->fKey.hash());
        for (uint32_t i = 0; i < numEvents; ++i) {
            SkDebugf("[%5lld %s",
                     (*log)->fEvents[i].fTime,
                     SkLog::Name((*log)->fEvents[i].fType));
            if ((*log)->fEvents[i].fType == SkLog::EventType2::kPurgedN ||
                (*log)->fEvents[i].fType == SkLog::EventType2::kPurgedP) {
                SkDebugf("-%d", (*log)->fEvents[i].fAncillary);
            }
            SkDebugf("]");
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
