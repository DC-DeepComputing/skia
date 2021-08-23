/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/glsl/GrUniformAggregator.h"

void GrUniformAggregator::addUniforms(const GrProcessor* p, const char* mangleSuffix) {
    if (!mangleSuffix) {
        mangleSuffix = "";
    }
    size_t i = 0;
    for (const auto& u : p->uniforms()) {
        if (!u.isInitialized()) {
            continue;
        }
        SkString name = SkStringPrintf("%s%s", u.name(), mangleSuffix);
        fMap[Handle{p, i}] = fRecords.size();
        Record record {
            {name.c_str(), u.type(), u.visibility()},
            fProcessorIndex,
            i,
            std::move(name)
        };
        fRecords.push_back(record);
        i++;
    }
    fProcessorIndex++;
}

GrProcessor::Uniform GrUniformAggregator::get(const GrProcessor* p, size_t index) const {
    if (auto iter = fMap.find(Handle{p, index}); iter != fMap.end()) {
        return fRecords[iter->second].uniform;
    }
    return {};
}

GrUniformAggregator::UniformRecords GrUniformAggregator::snapUniformRecords() {
    fMap.clear();
    fRecords.shrink_to_fit();
    return std::move(fRecords);
}
