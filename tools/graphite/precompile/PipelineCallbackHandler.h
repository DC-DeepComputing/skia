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
class OldPipelineCallBackHandler {
public:
    static void CallBack(void* data, sk_sp<SkData> androidStyleKey) {
        OldPipelineCallBackHandler* handler = reinterpret_cast<OldPipelineCallBackHandler*>(data);

        handler->add(std::move(androidStyleKey));
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
};


// This is intended to be an example of a Precompilation Callback handler. For DM it collects
// all the Android-style keys that are used by a given source (e.g., gm, or skp) and uses
// them in resetAndRecreatePipelines to recreate the Pipelines.
class NewPipelineCallBackHandler {
public:
    static void CallBack(void* context,
                          skgpu::graphite::ContextOptions::PipelineCacheOp op,
                          const std::string& label,
                          uint32_t uniqueKeyHash,
                          bool fromPrecompile,
                          sk_sp<SkData> androidStyleKey) {
        NewPipelineCallBackHandler* handler = reinterpret_cast<NewPipelineCallBackHandler*>(context);

        handler->add(op, label, uniqueKeyHash, fromPrecompile);
    }

    void add(skgpu::graphite::ContextOptions::PipelineCacheOp op,
             const std::string& label,
             uint32_t uniqueKeyHash,
             bool fromPrecompile) SK_EXCLUDES(fSpinLock);

    void report() SK_EXCLUDES(fSpinLock);

private:
    mutable SkSpinlock fSpinLock;

    struct PipelineKey {
        std::string fLabel;
        uint32_t fUniqueKeyHash;

        bool operator==(const PipelineKey& other) const {
            return fUniqueKeyHash == other.fUniqueKeyHash && fLabel == other.fLabel;
        }
    };

    struct PipelineData {
        PipelineKey fKey;
        bool fFromPrecompile;
        sk_sp<SkData> fData;

        uint32_t fUses = 0;
    };

    struct PipelineDataTraits {
        static const PipelineKey& GetKey(PipelineData*& e) { return e->fKey; }
        static uint32_t Hash(const PipelineKey& k) { return k.fUniqueKeyHash; }
    };

    skia_private::THashTable<PipelineData*, PipelineKey, PipelineDataTraits> fMap SK_GUARDED_BY(fSpinLock);
};

}  // namespace skiatools::graphite

#endif  // PipelineCallbackHandler_DEFINED
