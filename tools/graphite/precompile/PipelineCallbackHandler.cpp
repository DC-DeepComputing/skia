/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/precompile/PipelineCallbackHandler.h"

namespace skiatools::graphite {

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

void PipelineCallBackHandler::add(skgpu::graphite::ContextOptions::PipelineCacheOp op,
                                  const std::string& label,
                                  uint32_t uniqueKeyHash,
                                  uint32_t compilationID,
                                  bool fromPrecompile,
                                  bool wasUsed) {

    foo** pF = fMap2.find({ label, uniqueKeyHash });
    if (!pF) {
        foo* tmp = new foo;
        tmp->fKey.fLabel = label;
        tmp->fKey.fUniqueKeyHash = uniqueKeyHash;

        tmp->fCompilationID = compilationID;
        tmp->fFromPrecompile = fromPrecompile;
        tmp->fUses = fromPrecompile ? 0 : 1;

        fMap2.set(tmp);
    }

}

}  // namespace skiatools::graphite
