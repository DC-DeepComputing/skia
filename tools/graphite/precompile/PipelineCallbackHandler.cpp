/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/precompile/PipelineCallbackHandler.h"

namespace skiatools::graphite {

#if 0
void PipelineCallBackHandler::add(sk_sp<SkData> payload) {
    SkAutoSpinlock lock{ fSpinLock };

    const sk_sp<SkData>* data = fMap.find({ payload.get() });
    if (!data) {
        fMap.set(std::move(payload));
    }
}

void PipelineCallBackHandler::retrieve(
        std::vector<sk_sp<SkData>>* result) {
    SkAutoSpinlock lock{ fSpinLock };

    result->reserve(fMap.count());

    fMap.foreach([result](sk_sp<SkData>* data) {
        result->push_back(*data);
    });
}

void PipelineCallBackHandler::reset() {
    SkAutoSpinlock lock{ fSpinLock };

    fMap.reset();
}
#endif

void PipelineCallBackHandler::add(skgpu::graphite::ContextOptions::PipelineCacheOp op,
                                  const std::string& label,
                                  uint32_t uniqueKeyHash,
                                  bool fromPrecompile) {
    SkAutoSpinlock lock{ fSpinLock };

    PipelineData** foundData = fPipelineDataMap.find({ label, uniqueKeyHash });
    if (foundData) {
        if (op == skgpu::graphite::ContextOptions::PipelineCacheOp::kPipelineFound) {
            (*foundData)->fUses++;
        }
    } else {
        SkASSERT(op == skgpu::graphite::ContextOptions::PipelineCacheOp::kAddingPipeline);

        PipelineData* newData = new PipelineData;
        newData->fKey.fLabel = label;
        newData->fKey.fUniqueKeyHash = uniqueKeyHash;

        newData->fFromPrecompile = fromPrecompile;
        newData->fUses = fromPrecompile ? 0 : 1;

        fPipelineDataMap.set(newData);
    }
}

void PipelineCallBackHandler::report() {
    SkAutoSpinlock lock{ fSpinLock };

    fPipelineDataMap.foreach([](PipelineData** data) -> void{
        SkDebugf("%u %s\n", (*data)->fUses, (*data)->fKey.fLabel.c_str());
        if ((*data)->fFromPrecompile && !(*data)->fUses) {
            SkDebugf("!!!!!!!!!!!!!!!!!!!!! Needless precompile\n");
        }
    });

}

}  // namespace skiatools::graphite
