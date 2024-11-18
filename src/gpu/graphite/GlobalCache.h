/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_GlobalCache_DEFINED
#define skgpu_graphite_GlobalCache_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkSpinlock.h"
#include "src/core/SkLRUCache.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/GraphicsPipeline.h"

#include <functional>

namespace skgpu::graphite {

class ComputePipeline;
class GraphicsPipeline;
class Resource;
class ShaderCodeDictionary;

/**
 * GlobalCache holds GPU resources that should be shared by every Recorder. The common requirement
 * of these resources are they are static/read-only, have long lifetimes, and are likely to be used
 * by multiple Recorders. The canonical example of this are pipelines.
 *
 * GlobalCache is thread safe, but intentionally splits queries and storing operations so that they
 * are not atomic. The pattern is to query for a resource, which has a high likelihood of a cache
 * hit. If it's not found, the Recorder creates the resource on its own, without locking the
 * GlobalCache. After the resource is created, it is added to the GlobalCache, atomically returning
 * the winning Resource in the event of a race between Recorders for the same UniqueKey.
 */
class GlobalCache {
public:
    GlobalCache();
    ~GlobalCache();

    void deleteResources();

    // Find a cached GraphicsPipeline that matches the associated key.
    sk_sp<GraphicsPipeline> findGraphicsPipeline(const UniqueKey&, bool forPrecompile) SK_EXCLUDES(fSpinLock);

    // Associate the given pipeline with the key. If the key has already had a separate pipeline
    // associated with the key, that pipeline is returned and the passed-in pipeline is discarded.
    // Otherwise, the passed-in pipeline is held by the GlobalCache and also returned back.
    sk_sp<GraphicsPipeline> addGraphicsPipeline(const UniqueKey&,
                                                sk_sp<GraphicsPipeline>) SK_EXCLUDES(fSpinLock);

#if defined(GPU_TEST_UTILS)
    int numGraphicsPipelines() const SK_EXCLUDES(fSpinLock);
    void resetGraphicsPipelines() SK_EXCLUDES(fSpinLock);
    void forEachGraphicsPipeline(
            const std::function<void(const UniqueKey&, const GraphicsPipeline*)>& fn)
            SK_EXCLUDES(fSpinLock);

    class SkLog : public SkRefCnt {
    public:
        SkLog(const UniqueKey& k) : fKey(k) {}

        enum class EventType2 {
            kUnset,
            kCacheHit,
            kCacheMiss,
            kNWonRaceOverN,
            kNWonRaceOverP,
            kPWonRaceOverN,
            kPWonRaceOverP,
            kAdded,
            kPurged,
            kWaitBegin,
            kWaitEnd,
        };

        enum class Type {
            kUnset,
            kPrecompile,
            kNormal,
        };

        static const char* Name(EventType2 type, Type entryType) {
            switch (type) {
                case EventType2::kUnset: return "unset";
                case EventType2::kCacheHit:
                    if (entryType == Type::kPrecompile) {
                        return "hit P for";
                    } else if (entryType == Type::kNormal) {
                        return "hit N for";
                    } else {
//                        SkASSERT(0);
                        return "hit U for";
                    }
                case EventType2::kCacheMiss: return "miss";
                case EventType2::kPWonRaceOverP: return "P won-race-over P";
                case EventType2::kPWonRaceOverN: return "P won-race-over N";
                case EventType2::kNWonRaceOverP: return "N won-race-over P";
                case EventType2::kNWonRaceOverN: return "N won-race-over N";
                case EventType2::kAdded:     return "added";
                case EventType2::kPurged:   return "purged";
                case EventType2::kWaitBegin:   return "wait-begin";
                case EventType2::kWaitEnd:   return "wait-end";
            }

            return "unkw";
        }

        static char Name2(Type type) {
            switch (type) {
                case Type::kUnset: return 'U';
                case Type::kPrecompile: return 'P';
                case Type::kNormal: return 'N';
            }

            return 'U';
        }

        void addEvent2(EventType2 type, bool forPrecompile);
        void setType(Type type) {
//            SkASSERT(fType == Type::kUnset);
            fType = type;
        }

        static constexpr uint32_t kMaxNumMsgs = 20;

        const UniqueKey fKey;
        struct {
            EventType2 fType = EventType2::kUnset;
            int64_t fTime = 0;
            bool fForPrecompile = false;
            int fAncillary = 0;
        } fEvents[kMaxNumMsgs];
        std::atomic<uint32_t> fCurEvent1{0};
        Type fType = Type::kUnset;
        uint32_t fNumHits = 0;
    };

    SkLog* gimme(const UniqueKey&) SK_GUARDED_BY(fSpinLock);
    void dump();

    struct PipelineStats {
        int fGraphicsCacheHits = 0;
        int fGraphicsCacheMisses = 0;
        int fGraphicsCacheAdditions = 0;
        int fTotalGraphicsRaces = 0;
        int fFoo[2][2] = { { 0, 0 }, { 0, 0 } }; // [winner][loser]
        int fBar[2][2] = { { 0, 0 }, { 0, 0 } }; // [winner][loser]
        int fBaz[2] = { 0, 0 };
    };

    PipelineStats getStats() const SK_EXCLUDES(fSpinLock);
#endif

    // Find and add operations for ComputePipelines, with the same pattern as GraphicsPipelines.
    sk_sp<ComputePipeline> findComputePipeline(const UniqueKey&) SK_EXCLUDES(fSpinLock);
    sk_sp<ComputePipeline> addComputePipeline(const UniqueKey&,
                                              sk_sp<ComputePipeline>) SK_EXCLUDES(fSpinLock);

    // The GlobalCache holds a ref on the given Resource until the cache is destroyed, keeping it
    // alive for the lifetime of the SharedContext. This should be used only for Resources that are
    // immutable after initialization so that anyone can use the resource without synchronization
    // or reference tracking.
    void addStaticResource(sk_sp<Resource>) SK_EXCLUDES(fSpinLock);

private:
    struct KeyHash {
        uint32_t operator()(const UniqueKey& key) const { return key.hash(); }
    };
    struct PurgeCB {
        void operator()(const UniqueKey& key, void* data) const {
            skia_private::THashTable<sk_sp<SkLog>, UniqueKey, Traits2>* map = (skia_private::THashTable<sk_sp<SkLog>, UniqueKey, Traits2>*) data;
            sk_sp<SkLog>* log = map->find(key);
            (*log)->addEvent2(SkLog::EventType2::kPurged, /* forPrecompile= */ false);
        }
    };

    using GraphicsPipelineCache = SkLRUCache2<UniqueKey, sk_sp<GraphicsPipeline>, KeyHash, PurgeCB>;
    using ComputePipelineCache  = SkLRUCache<UniqueKey, sk_sp<ComputePipeline>,  KeyHash>;

    // TODO: can we do something better given this should have write-seldom/read-often behavior?
    mutable SkSpinlock fSpinLock;

    // GraphicsPipelines and ComputePipelines are expensive to create, likely to be used by multiple
    // Recorders, and are ideally pre-compiled on process startup so thread write-contention is
    // expected to be low. For these reasons we store pipelines globally instead of per-Recorder.
    GraphicsPipelineCache fGraphicsPipelineCache SK_GUARDED_BY(fSpinLock);
    ComputePipelineCache  fComputePipelineCache  SK_GUARDED_BY(fSpinLock);

    struct Traits2 {
        static const UniqueKey& GetKey(sk_sp<SkLog> e) {
            return (e)->fKey;
        }

        static uint32_t Hash(const UniqueKey& k) {
            return KeyHash()(k);
        }
    };
    skia_private::THashTable<sk_sp<SkLog>, UniqueKey, Traits2> fMap SK_GUARDED_BY(fSpinLock);

    skia_private::TArray<sk_sp<Resource>> fStaticResource SK_GUARDED_BY(fSpinLock);

#if defined(GPU_TEST_UTILS)
    PipelineStats fStats SK_GUARDED_BY(fSpinLock);
#endif
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_GlobalCache_DEFINED
