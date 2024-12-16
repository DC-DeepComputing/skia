/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/GlobalCache.h"

#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/Resource.h"

namespace {

uint32_t next_compilation_id() {
    static std::atomic<uint32_t> nextId{0};
    // Not worried about overflow since we don't expect that many GraphicsPipelines.
    // Even if it wraps around to 0, this is solely for debug logging.
    return nextId.fetch_add(1, std::memory_order_relaxed);
}

} // anonymous namespce

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
        : fGraphicsPipelineCache(/*256*/ 2, &fLogHash)  // TODO: find a good value for these limits
        , fComputePipelineCache(256)
        , fLogHash() {
}

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

void GlobalCache::LogPurge(const UniqueKey& key, sk_sp<GraphicsPipeline>* p, void* data) {
    LogHash* hash = reinterpret_cast<LogHash *>(data);
    sk_sp<SkLog>* log = hash->find(key);
    bool fromPrecompile = (*p)->fromPrecompile();
    auto event = fromPrecompile ? ((*p)->wasUsed() ? SkLog::EventType2::kGoodPurgedP : SkLog::EventType2::kBadPurgedP)
                                : ((*p)->wasUsed() ? SkLog::EventType2::kGoodPurgedN : SkLog::EventType2::kBadPurgedN);
    (*log)->addEvent2(event, (*p)->id1());
    if ((*log)->fNumHits == 0 && (*p)->fromPrecompile()) {
        SkDebugf("------------------ Wasted!!!\n");
    }
    (*log)->fNumHits = 0;

#if defined(SK_PIPELINE_LIFETIME_LOGGING)
    if (!(*p)->wasUsed() && (*p)->fromPrecompile()) {
        // Add UMA stat for wasted precompile!
    }

    // A "Bad" Purge is one where the Pipeline was never retrieved from the Cache (i.e., unused
    // overgeneration).
    static const char* kNames[2][2] = { { "BadPurgedN", "BadPurgedP" },
                                        { "PurgedN",    "PurgedP"} };
    SkDebugf("{ %d, %lld, \"%s\", %u, -1, %u, UINT32_MAX },\n",
             (*log)->id(),
             micros_since_0(),
             kNames[(*p)->wasUsed()][fromPrecompile],
             key.hash(),
             (*p)->id1());
    TRACE_EVENT_INSTANT2("skia.gpu",
                         TRACE_STR_STATIC(kNames[(*p)->wasUsed()][(*p)->fromPrecompile()]),
                         TRACE_EVENT_SCOPE_THREAD,
                         "key", key.hash(),
                         "compilationID", (*p)->getPipelineInfo().fCompilationID);
#endif
}

sk_sp<GraphicsPipeline> GlobalCache::findGraphicsPipeline(
        const UniqueKey& key,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags,
        uint32_t *compilationID) {
    SkLog* log = this->gimme(key);

    [[maybe_unused]] bool forPrecompile =
            SkToBool(pipelineCreationFlags & PipelineCreationFlags::kForPrecompilation);

    SkAutoSpinlock lock{fSpinLock};

    sk_sp<GraphicsPipeline>* entry = fGraphicsPipelineCache.find(key);
    if (entry) {
#if defined(GPU_TEST_UTILS)
        ++fStats.fGraphicsCacheHits;
#endif

        ++fStats.fBar[(*entry)->fromPrecompile()][forPrecompile];

        auto event = forPrecompile ? SkLog::EventType2::kCacheHitForP
                                   : SkLog::EventType2::kCacheHitForN;

        log->fNumHits++;
        (*entry)->markUsed();

        log->addEvent2(event, (*entry)->id1());

#if defined(SK_PIPELINE_LIFETIME_LOGGING)
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
                             "compilationID", (*entry)->getPipelineInfo().fCompilationID);
#endif

        return *entry;
    } else {
#if defined(GPU_TEST_UTILS)
        ++fStats.fGraphicsCacheMisses;
#endif

        ++fStats.fBaz[forPrecompile];

        if (compilationID) {
            // This is a cache miss so we know the next step is going to be a Pipeline
            // creation. Create the compilationID here so we can use it in the "CacheMissFor"
            // trace event.
            *compilationID = next_compilation_id();

#if defined(SK_PIPELINE_LIFETIME_LOGGING)
            static const char* kNames[2] = { "CacheMissForN", "CacheMissForP" };
            SkDebugf("{ %d, %lld, \"%s\", %u, -1, %u, UINT32_MAX },\n",
                     log->id(),
                     micros_since_0(),
                     kNames[forPrecompile],
                     key.hash(),
                     *compilationID);
            TRACE_EVENT_INSTANT2("skia.gpu",
                                 TRACE_STR_STATIC(kNames[forPrecompile]),
                                 TRACE_EVENT_SCOPE_THREAD,
                                 "key", key.hash(),
                                 "compilationID", *compilationID);
#endif
            auto event = forPrecompile ? SkLog::EventType2::kCacheMissForP
                                       : SkLog::EventType2::kCacheMissForN;
            log->addEvent2(event, *compilationID);

        }

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
    SkLog* log = this->gimme(key);

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
        bool fromPrecompile = pipeline->fromPrecompile();
        auto event = fromPrecompile ? SkLog::EventType2::kAddedP
                                    : SkLog::EventType2::kAddedN;
        SkASSERT(!pipeline->wasUsed());
        SkASSERT(log->fNumHits == 0);
        uint32_t pipelineID = pipeline->id1();
        entry = fGraphicsPipelineCache.insert(key, std::move(pipeline));
        log->addEvent2(event, pipelineID);

        // Precompile Pipelines are only marked as used when they get a cache hit in
        // findGraphicsPipeline
        if (!fromPrecompile) {
            (*entry)->markUsed();
        }

        // Precompile Pipelines are only marked as used when they get a cache hit in
        // findGraphicsPipeline
        if (!(*entry)->fromPrecompile()) {
            (*entry)->markUsed();
        }

#if defined(SK_PIPELINE_LIFETIME_LOGGING)
        static const char* kNames[2] = { "AddedN", "AddedP" };
        SkDebugf("{ %d, %lld, \"%s\", %u, -1, %u, UINT32_MAX },\n",
                 log->id(),
                 micros_since_0(),
                 kNames[fromPrecompile],
                 key.hash(),
                 pipelineID);
        TRACE_EVENT_INSTANT2("skia.gpu",
                             TRACE_STR_STATIC(kNames[(*entry)->fromPrecompile()]),
                             TRACE_EVENT_SCOPE_THREAD,
                             "key", key.hash(),
                             "compilationID", (*entry)->getPipelineInfo().fCompilationID);
#endif
    } else {
        [[maybe_unused]] int race = (*entry)->fromPrecompile() * 2 + pipeline->fromPrecompile();

        // else there was a race creating the same pipeline and this thread lost, so return
        // the winner
        ++fStats.fTotalGraphicsRaces;
        ++fStats.fFoo[(*entry)->fromPrecompile()][pipeline->fromPrecompile()];

        static const SkLog::EventType2 kCombos[4] = {
                SkLog::EventType2::kNWonRaceOverN,
                SkLog::EventType2::kNWonRaceOverP,
                SkLog::EventType2::kPWonRaceOverN,
                SkLog::EventType2::kPWonRaceOverP,
        };


        log->addEvent2(kCombos[race], (*entry)->id1(), pipeline->id1());

#if defined(SK_PIPELINE_LIFETIME_LOGGING)
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
                             // The losing compilation
                             "compilationID", pipeline->getPipelineInfo().fCompilationID);
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

void SkLog::addEvent2(EventType2 type, uint32_t pipelineID1, uint32_t pipelineID2) {
    uint32_t curEvent = fCurEvent1.fetch_add(1, std::memory_order_relaxed);
    SkASSERT_RELEASE(curEvent < kMaxNumMsgs);

    fEvents[curEvent] = { type, micros_since_0(), (int) fNumHits, pipelineID1, pipelineID2 };
}

SkLog* GlobalCache::gimme2(uint32_t hash) const {
    SkAutoSpinlock lock{fSpinLock};

    sk_sp<SkLog>* log = fLogHash.find2(hash);
    if (!log) {
        SkASSERT(0);
        return nullptr;
    }

    return const_cast<SkLog*>((*log).get());
}

SkLog* GlobalCache::gimme(const UniqueKey& key) {
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
