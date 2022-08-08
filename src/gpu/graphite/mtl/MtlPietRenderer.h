/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlPietRenderer_DEFINED
#define skgpu_graphite_MtlPietRenderer_DEFINED

#include "src/gpu/graphite/PietRenderer.h"
#include "src/gpu/piet/Renderer.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {

class MtlPietRenderer final : public PietRenderer {
public:
    MtlPietRenderer(id<MTLDevice>, id<MTLCommandQueue>);

    void encodeSceneRender(const skgpu::piet::Scene& scene,
                           const Texture* target,
                           CommandBuffer* cmdBuffer) const override;

private:
    skgpu::piet::MtlRenderer fRenderer;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_MtlPietRenderer_DEFINED
