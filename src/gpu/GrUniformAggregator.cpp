/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrUniformAggregator.h"

void GrUniformAggregator::addUniforms(const GrProcessor& p, const char* mangleSuffix) {
    if (!mangleSuffix) {
        mangleSuffix = "";
    }
    size_t i = 0;
    bool first = true;
    for (const auto& u : p.uniforms()) {
        if (!u.isInitialized()) {
            continue;
        }
        SkString name = SkStringPrintf("%s%s", u.name(), mangleSuffix);
        Record record{GrProcessor::Uniform{name.c_str(), u.type(), u.visibility()},
                      i++,
                      std::move(name)};
        fMap[fProcessorIndex].push_back(std::move(record));
        if (first) {
            SkASSERT(fProcessorToIndexMap.find(&p) == fProcessorToIndexMap.end());
            fProcessorToIndexMap[&p] = fProcessorIndex;
            first = false;
        }
        ++fTotalUniformCount;
    }
    ++fProcessorIndex;
}

GrProcessor::Uniform GrUniformAggregator::findUniform(const GrProcessor* p, size_t index) const {
    if (auto indexIter = fProcessorToIndexMap.find(p); indexIter != fProcessorToIndexMap.end()) {
        int processorIndex = indexIter->second;
        auto recordIter = fMap.find(processorIndex);
        SkASSERT(recordIter != fMap.end());
        return recordIter->second[index].uniform;
    }
    return {};
}

GrUniformAggregator::ProcessorUniformMap GrUniformAggregator::snapUniforms() {
    fTotalUniformCount = 0;
    fProcessorToIndexMap.clear();
    return std::move(fMap);
}
