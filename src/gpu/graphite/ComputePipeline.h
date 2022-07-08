/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ComputePipeline_DEFINED
#define skgpu_graphite_ComputePipeline_DEFINED

#include "src/gpu/graphite/Resource.h"

namespace skgpu::graphite {

class ComputePipeline : public Resource {
public:
    ~ComputePipeline() override {}

protected:
    ComputePipeline(const Gpu* gpu)
        : Resource(gpu, Ownership::kOwned, SkBudgeted::kYes) {}
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ComputePipeline_DEFINED
