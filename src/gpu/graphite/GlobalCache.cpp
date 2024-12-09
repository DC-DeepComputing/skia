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

namespace {

static uint32_t next_id() {
    static std::atomic<uint32_t> nextId{0};
    // Not worried about overflow since we don't expect that many GraphicsPipelines.
    // Even if it wraps around to 0, this is solely for debug logging.
    return nextId.fetch_add(1, std::memory_order_relaxed);
}

}

extern skgpu::StdSteadyClock::time_point fTimeStart;

int64_t micros_since_1(skgpu::StdSteadyClock::time_point start) {
    skgpu::StdSteadyClock::duration elapsed = skgpu::StdSteadyClock::now() - start;
    return std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
};

int64_t micros_since_0() {
    return micros_since_1(fTimeStart);
}

namespace skgpu::graphite {

GlobalCache::GlobalCache()
        : fGraphicsPipelineCache(256 /* 2 */ , &fLogHash)  // TODO: find a good value for these limits
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
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags,
        uint32_t *futureID) {
    SkLog* log = this->gimme(key);

    bool forPrecompile = SkToBool(pipelineCreationFlags & PipelineCreationFlags::kForPrecompilation);

    SkAutoSpinlock lock{fSpinLock};

    sk_sp<GraphicsPipeline>* entry = fGraphicsPipelineCache.find(key);
#if defined(GPU_TEST_UTILS)
    if (entry) {
        ++fStats.fGraphicsCacheHits;
        ++fStats.fBar[(*entry)->fromPrecompile1()][forPrecompile];

        auto event = forPrecompile ? GlobalCache::SkLog::EventType2::kCacheHitForP
                                   : GlobalCache::SkLog::EventType2::kCacheHitForN;

        log->fNumHits++;
        (*entry)->incrHits();
        log->addEvent2(event, (*entry)->id1());

        static const char* kNames[2] = { "CacheHitForN", "CacheHitForP" };
        SkDebugf("{ %d, %lld, \"%s\", %u, -1, %u, UINT32_MAX },\n",
                 log->id(),
                 micros_since_0(),
                 kNames[forPrecompile],
                 key.hash(),
                 (*entry)->id1());
        TRACE_EVENT_INSTANT2("skia.gpu",
                             TRACE_STR_STATIC(kNames[forPrecompile]),
                             TRACE_EVENT_SCOPE_THREAD,
                             "key", key.hash(),
                             "pipelineID", (*entry)->id1());
    } else {
        ++fStats.fGraphicsCacheMisses;
        ++fStats.fBaz[forPrecompile];

        SkASSERT(futureID);
        if (futureID) {
            *futureID = next_id();
        }

        auto event = forPrecompile ? GlobalCache::SkLog::EventType2::kCacheMissForP
                                   : GlobalCache::SkLog::EventType2::kCacheMissForN;
        log->addEvent2(event, *futureID);

        static const char* kNames[2] = { "CacheMissForN", "CacheMissForP" };
        SkDebugf("{ %d, %lld, \"%s\", %u, -1, %u, UINT32_MAX },\n",
                 log->id(),
                 micros_since_0(),
                 kNames[forPrecompile],
                 key.hash(),
                 *futureID);
        TRACE_EVENT_INSTANT2("skia.gpu",
                             TRACE_STR_STATIC(kNames[forPrecompile]),
                             TRACE_EVENT_SCOPE_THREAD,
                             "key", key.hash(),
                             "pipelineID", *futureID);
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
        bool fromPrecompile = pipeline->fromPrecompile1();
        auto event = fromPrecompile ? GlobalCache::SkLog::EventType2::kAddedP
                                    : GlobalCache::SkLog::EventType2::kAddedN;
        SkASSERT(pipeline->numHits() == 0);
        SkASSERT(log->fNumHits == 0);
        uint32_t pipelineID = pipeline->id1();
        entry = fGraphicsPipelineCache.insert(key, std::move(pipeline));
        log->addEvent2(event, pipelineID);

        static const char* kNames[2] = { "AddedN", "AddedP" };
        SkDebugf("{ %d, %lld, \"%s\", %u, -1, %u, UINT32_MAX },\n",
                 log->id(),
                 micros_since_0(),
                 kNames[fromPrecompile],
                 key.hash(),
                 pipelineID);
        TRACE_EVENT_INSTANT2("skia.gpu",
                             TRACE_STR_STATIC(kNames[fromPrecompile]),
                             TRACE_EVENT_SCOPE_THREAD,
                             "key", key.hash(),
                             "pipelineID", pipelineID);
    } else {
#if defined(GPU_TEST_UTILS)
        // else there was a race creating the same pipeline and this thread lost, so return
        // the winner
        ++fStats.fTotalGraphicsRaces;
        ++fStats.fFoo[(*entry)->fromPrecompile1()][pipeline->fromPrecompile1()];

        static const GlobalCache::SkLog::EventType2 kCombos[4] = {
                GlobalCache::SkLog::EventType2::kNWonRaceOverN,
                GlobalCache::SkLog::EventType2::kNWonRaceOverP,
                GlobalCache::SkLog::EventType2::kPWonRaceOverN,
                GlobalCache::SkLog::EventType2::kPWonRaceOverP,
        };


        int race = (*entry)->fromPrecompile1() * 2 + pipeline->fromPrecompile1();
        log->addEvent2(kCombos[race], (*entry)->id1(), pipeline->id1());

        static const char* kNames[4] = {
                "NWonRaceOverN",
                "NWonRaceOverP",
                "PWonRaceOverN",
                "PWonRaceOverP"
        };
        SkDebugf("{ %d, %lld, \"%s\", %u, -1, %u, %u },\n",
                 log->id(),
                 micros_since_0(),
                 kNames[race],
                 key.hash(),
                 (*entry)->id1(),   // winner
                 pipeline->id1());  // loser
        TRACE_EVENT_INSTANT2("skia.gpu",
                             TRACE_STR_STATIC(kNames[race]),
                             TRACE_EVENT_SCOPE_THREAD,
                             "key", key.hash(),
                             /* "winningPipelineID", (*entry)->id1() */
                             "losingPipelineID", pipeline->id1());
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

void GlobalCache::SkLog::addEvent2(EventType2 type, uint32_t pipelineID1, uint32_t pipelineID2) {
    uint32_t curEvent = fCurEvent1.fetch_add(1, std::memory_order_relaxed);
    SkASSERT_RELEASE(curEvent < kMaxNumMsgs);

    fEvents[curEvent] = { type, micros_since_0(), (int) fNumHits, pipelineID1, pipelineID2 };
}

GlobalCache::SkLog* GlobalCache::gimme(const UniqueKey& key) {
    SkAutoSpinlock lock{fSpinLock};


    sk_sp<SkLog>* log = fLogHash.find(key);
    if (!log) {
        sk_sp<SkLog> tmp = sk_make_sp<SkLog>(key, fLogHash.count());
        log = fLogHash.set(std::move(tmp));
    }

    return (*log).get();
}

void GlobalCache::dump() {
    SkAutoSpinlock lock{fSpinLock};

    fLogHash.foreach([](sk_sp<SkLog>* log) {
        uint32_t numEvents = (*log)->fCurEvent1.fetch_add(0, std::memory_order_relaxed);
        numEvents = std::min(numEvents, SkLog::kMaxNumMsgs);

        SkDebugf("%2d: %10u ", (*log)->id(), (*log)->fKey.hash());
        for (uint32_t i = 0; i < numEvents; ++i) {
            SkDebugf("[%5lld %s",
                     (*log)->fEvents[i].fTime,
                     SkLog::Name((*log)->fEvents[i].fType));
            if ((*log)->fEvents[i].fType == SkLog::EventType2::kPurgedN ||
                (*log)->fEvents[i].fType == SkLog::EventType2::kPurgedP) {
                SkDebugf("-%d", (*log)->fEvents[i].fAncillary);
            }
            if ((*log)->fEvents[i].fPipelineKey1 != UINT32_MAX) {
                SkDebugf(" %u", (*log)->fEvents[i].fPipelineKey1);
            }
            if ((*log)->fEvents[i].fPipelineKey2 != UINT32_MAX) {
                SkDebugf(" %u", (*log)->fEvents[i].fPipelineKey2);
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
