/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/precompile/PipelineCallbackHandler.h"

namespace skiatools::graphite {

void OldPipelineCallBackHandler::add(sk_sp<SkData> payload) {
    SkAutoSpinlock lock{ fSpinLock };

    const sk_sp<SkData>* data = fMap.find({ payload.get() });
    if (!data) {
        fMap.set(std::move(payload));
    }
}

void OldPipelineCallBackHandler::retrieve(
        std::vector<sk_sp<SkData>>* result) {
    SkAutoSpinlock lock{ fSpinLock };

    result->reserve(fMap.count());

    fMap.foreach([result](sk_sp<SkData>* data) {
        result->push_back(*data);
    });
}

void OldPipelineCallBackHandler::reset() {
    SkAutoSpinlock lock{ fSpinLock };

    fMap.reset();
}

void NewPipelineCallBackHandler::add(skgpu::graphite::ContextOptions::PipelineCacheOp op,
                                     const std::string& label,
                                     uint32_t uniqueKeyHash,
                                     bool fromPrecompile) {
    SkAutoSpinlock lock{ fSpinLock };

    PipelineData** foundData = fMap.find({ label, uniqueKeyHash });
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

        fMap.set(newData);
    }
}

void NewPipelineCallBackHandler::report() {
    SkAutoSpinlock lock{ fSpinLock };

    fMap.foreach([](PipelineData** data) -> void{
        SkDebugf("%u %s\n", (*data)->fUses, (*data)->fKey.fLabel.c_str());
        if ((*data)->fFromPrecompile && !(*data)->fUses) {
            SkDebugf("!!!!!!!!!!!!!!!!!!!!! Needless precompile\n");
        }
    });

}

}  // namespace skiatools::graphite
