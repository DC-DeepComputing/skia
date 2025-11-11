/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/PipelineCallbackHandler.h"

#include <algorithm>

namespace skiatools::graphite {

void PipelineCallBackHandler::add(skgpu::graphite::ContextOptions::PipelineCacheOp op,
                                  const std::string& label,
                                  uint32_t uniqueKeyHash,
                                  bool fromPrecompile,
                                  sk_sp<SkData> androidStyleKey) {
    SkAutoSpinlock lock{ fSpinLock };

    auto iter = fMap.find({ &label, uniqueKeyHash });
    if (iter != fMap.end()) {
        if (op == skgpu::graphite::ContextOptions::PipelineCacheOp::kPipelineFound) {
            iter->second->fUses++;
        }
    } else {
        SkASSERT(op == skgpu::graphite::ContextOptions::PipelineCacheOp::kAddingPipeline);

        std::unique_ptr<PipelineData> newData = std::make_unique<PipelineData>(
            label, fromPrecompile, std::move(androidStyleKey));

        fMap.insert({ PipelineKey(&newData->fLabel, uniqueKeyHash),
                      std::move(newData) });
    }
}

void PipelineCallBackHandler::report() {
    // The assumption is that we're just doing this once at the end so we just lock for the
    // entire method.
    SkAutoSpinlock lock{ fSpinLock };

    std::vector<const PipelineData*> tmp;

    tmp.reserve(fMap.size());

    for (const auto& [key, value] : fMap) {
        tmp.push_back(value.get());
    }

    std::sort(tmp.begin(), tmp.end(), [](const PipelineData* a, const PipelineData* b) {
                                            if (a->fUses != b->fUses) {
                                                return a->fUses > b->fUses;
                                            }
                                            return a->fLabel < b->fLabel;
                                        });

    for (const PipelineData* data : tmp) {
        if (data->fFromPrecompile && !data->fUses) {
            SkDebugf("!! ");   // A needless precompiled Pipeline
        }
        SkDebugf("%u %s\n", data->fUses, data->fLabel.c_str());
    }
}

}  // namespace skiatools::graphite
