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
#include "include/gpu/graphite/ContextOptions.h"
#include "src/base/SkSpinlock.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkTHash.h"

#include <vector>
#include <unordered_map>

class SkData;

namespace skiatools::graphite {

// This is intended to be an example of a Precompilation Callback handler.
// For the gr*testprecompile configs it is used to:
//   collect all the Android-style keys that are used by a given source (e.g., gm, or skp)
//   recreate the Pipelines from the collected keys in `resetAndRecreatePipelines`
// For the gr*testtracking configs it is used to:
//   collect all the Pipeline labels for a given source
//   print out a list of the labels at the end using `report`
class PipelineCallBackHandler {
public:
    static void CallBack(void* context,
                         skgpu::graphite::ContextOptions::PipelineCacheOp op,
                         const std::string& label,
                         uint32_t uniqueKeyHash,
                         bool fromPrecompile,
                         sk_sp<SkData> androidStyleKey) {
        PipelineCallBackHandler* handler = reinterpret_cast<PipelineCallBackHandler*>(context);

        handler->add(op, label, uniqueKeyHash, fromPrecompile, std::move(androidStyleKey));
    }

    void add(skgpu::graphite::ContextOptions::PipelineCacheOp op,
             const std::string& label,
             uint32_t uniqueKeyHash,
             bool fromPrecompile,
             sk_sp<SkData> androidStyleKey) SK_EXCLUDES(fSpinLock);

    void retrieveKeys(std::vector<sk_sp<SkData>>* result) SK_EXCLUDES(fSpinLock) {
        SkAutoSpinlock lock{ fSpinLock };

        result->reserve(fMap.size());

        for (const auto& [key, value] : fMap) {
            // Because not all Pipelines are serializable we need to check for nulls here
            if (value->fAndroidStyleKey) {
                result->push_back(value->fAndroidStyleKey);
            }
        }
    }

    void reset() SK_EXCLUDES(fSpinLock) {
        SkAutoSpinlock lock{ fSpinLock };

        fMap.clear();
    }

    void report() SK_EXCLUDES(fSpinLock);

private:
    mutable SkSpinlock fSpinLock;

    struct PipelineData {
        PipelineData(const std::string& label,
                     bool fromPrecompile,
                     sk_sp<SkData> androidStyleKey) :
               fLabel(label),
               fAndroidStyleKey(std::move(androidStyleKey)),
               fUses(fromPrecompile ? 0 : 1),
               fFromPrecompile(fromPrecompile) {
        }

        const std::string   fLabel;
        const sk_sp<SkData> fAndroidStyleKey;
        uint32_t            fUses;
        const bool          fFromPrecompile;
    };

    struct PipelineKey {
        PipelineKey(const std::string* label, uint32_t uniqueKeyHash)
            : fLabel(label)
            , fUniqueKeyHash(uniqueKeyHash) {}
        PipelineKey() = default;

        std::size_t operator()(const PipelineKey& k) const { return k.fUniqueKeyHash; }

        bool operator==(const PipelineKey& other) const {
            return fUniqueKeyHash == other.fUniqueKeyHash && *fLabel == *other.fLabel;
        }

        const std::string* fLabel = nullptr;
        uint32_t fUniqueKeyHash = 0;
    };

    std::unordered_map<PipelineKey, std::unique_ptr<PipelineData>, PipelineKey> fMap;
};

}  // namespace skiatools::graphite

#endif  // PipelineCallbackHandler_DEFINED
