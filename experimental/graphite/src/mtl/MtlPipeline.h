/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlPipeline_DEFINED
#define skgpu_MtlPipeline_DEFINED

#include "experimental/graphite/src/Pipeline.h"

#include "include/ports/SkCFObject.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

class RenderPipeline final : public skgpu::RenderPipeline {
public:
    static sk_sp<RenderPipeline> Make() {
        return sk_sp<RenderPipeline>(new RenderPipeline());
    }
    ~RenderPipeline() override {}

private:
    RenderPipeline();
};

// TODO: add ComputePipeline

} // namespace skgpu::mtl

#endif // skgpu_MtlPipeline_DEFINED
