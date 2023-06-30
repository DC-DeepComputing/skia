/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/GraphicsPipeline.h"

namespace skgpu::graphite {

GraphicsPipeline::GraphicsPipeline(const SharedContext* sharedContext, Shaders* pipelineShaders)
        : Resource(sharedContext, Ownership::kOwned, skgpu::Budgeted::kYes, /*gpuMemorySize=*/0) {
#if GRAPHITE_TEST_UTILS
    if (pipelineShaders) {
        fPipelineShaders = std::move(*pipelineShaders);
    }
#endif
}

GraphicsPipeline::~GraphicsPipeline() {
}

} // namespace skgpu::graphite
