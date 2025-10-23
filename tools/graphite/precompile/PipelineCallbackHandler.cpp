/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/precompile/PipelineCallbackHandler.h"

namespace skiatools::graphite {

void NewPipelineCallBackHandler::add(skgpu::graphite::ContextOptions::PipelineCacheOp op,
                                     const std::string& label,
                                     uint32_t uniqueKeyHash,
                                     bool fromPrecompile,
                                     sk_sp<SkData> androidStyleKey) {
    SkAutoSpinlock lock{ fSpinLock };

    std::unique_ptr<PipelineData>* foundData = fMap.find({ &label, uniqueKeyHash });
    if (foundData) {
        if (op == skgpu::graphite::ContextOptions::PipelineCacheOp::kPipelineFound) {
            (*foundData)->fUses++;
        }
    } else {
        SkASSERT(op == skgpu::graphite::ContextOptions::PipelineCacheOp::kAddingPipeline);

        std::unique_ptr<PipelineData> newData = std::make_unique<PipelineData>(
            label, uniqueKeyHash, fromPrecompile, std::move(androidStyleKey));

        fMap.set(std::move(newData));
    }
}

void NewPipelineCallBackHandler::report() {
    SkAutoSpinlock lock{ fSpinLock };

    fMap.foreach([](std::unique_ptr<PipelineData>* data) -> void{
        SkDebugf("%u %s\n", (*data)->fUses, (*data)->fLabel.c_str());
        if ((*data)->fFromPrecompile && !(*data)->fUses) {
            SkDebugf("!!!!!!!!!!!!!!!!!!!!! Needless precompile\n");
        }
    });

}

}  // namespace skiatools::graphite
