/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PipelineCallbackHandler_DEFINED
#define PipelineCallbackHandler_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "src/base/SkSpinlock.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkTHash.h"

#include "include/gpu/graphite/ContextOptions.h"

#include <vector>

class SkData;

namespace skiatools::graphite {


// This is intended to be an example of a Precompilation Callback handler. For DM it collects
// all the Android-style keys that are used by a given source (e.g., gm, or skp) and uses
// them in resetAndRecreatePipelines to recreate the Pipelines.
class PipelineCallBackHandler {
public:
    static void CallBack(void* data, sk_sp<SkData> androidStyleKey) {
        PipelineCallBackHandler* handler = reinterpret_cast<PipelineCallBackHandler*>(data);

        handler->add(std::move(androidStyleKey));
    }

    static void CallBack2(void* data,
                          skgpu::graphite::ContextOptions::PipelineCacheOp op,
                          const std::string& label,
                          uint32_t uniqueKeyHash,
                          uint32_t compilationID,
                          bool fromPrecompile,
                          bool wasUsed) {
        PipelineCallBackHandler* handler = reinterpret_cast<PipelineCallBackHandler*>(data);

        handler->add(op, label, uniqueKeyHash, compilationID, fromPrecompile, wasUsed);
    }

    // Add an Android-style key to the map
    void add(sk_sp<SkData> androidStyleKey) SK_EXCLUDES(fSpinLock);

    // Retrieve all the unique collected keys
    void retrieve(std::vector<sk_sp<SkData>>*) SK_EXCLUDES(fSpinLock);

    void reset() SK_EXCLUDES(fSpinLock);

    int numKeys() const SK_EXCLUDES(fSpinLock) {
        SkAutoSpinlock lock{ fSpinLock };
        return fMap.count();
    }

    void add(skgpu::graphite::ContextOptions::PipelineCacheOp op,
             const std::string& label,
             uint32_t uniqueKeyHash,
             uint32_t compilationID,
             bool fromPrecompile,
             bool wasUsed);

private:
    mutable SkSpinlock fSpinLock;

    struct SkDataKey {
        static SkDataKey GetKey(sk_sp<SkData>& e) { return { e.get() }; }
        static uint32_t Hash(const SkDataKey& k) { return k.hash(); }

        bool operator==(const SkDataKey& other) const { return fData->equals(other.fData); }
        uint32_t hash() const { return SkChecksum::Hash32(fData->data(), fData->size()); }

        const SkData* fData;
    };

    skia_private::THashTable<sk_sp<SkData>, SkDataKey, SkDataKey> fMap SK_GUARDED_BY(fSpinLock);

    struct key2 {
        std::string fLabel;
        uint32_t fUniqueKeyHash;

        bool operator==(const key2& other) const {
            return fUniqueKeyHash == other.fUniqueKeyHash &&
                   fLabel == other.fLabel;
        }
    };

    struct foo {
        key2 fKey;
        uint32_t fCompilationID;
        bool fFromPrecompile;

        uint32_t fUses = 0;
    };

    struct Traits2 {
        static const key2& GetKey(foo*& e) { return e->fKey; }
        static uint32_t Hash(const key2& k) { return k.fUniqueKeyHash; }

        const SkData* fData;
    };

    skia_private::THashTable<foo*, key2, Traits2> fMap2;
};

}  // namespace skiatools::graphite

#endif  // PipelineCallbackHandler_DEFINED
