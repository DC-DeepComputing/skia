/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/piet/PietRenderer.h"

#include "src/gpu/graphite/mtl/MtlCommandBuffer.h"
#include "src/gpu/graphite/mtl/MtlGpu.h"
#include "src/gpu/graphite/mtl/MtlTexture.h"
#include "src/gpu/piet/Renderer.h"

namespace skgpu::graphite {
namespace {

class MtlPietRenderer final : public PietRenderer {
public:
    explicit MtlPietRenderer(const MtlGpu* gpu) : fRenderer(gpu->device(), gpu->queue()) {}

    void encodeSceneRender(const skgpu::piet::Scene& scene,
                           const Texture* target,
                           CommandBuffer* cmdBuffer) const override {
        id<MTLTexture> mtlTexture = static_cast<const MtlTexture*>(target)->mtlTexture();
        id<MTLCommandBuffer> mtlCmdBuffer =
                static_cast<MtlCommandBuffer*>(cmdBuffer)->getCommandBufferForDirectEncoding();
        fRenderer.render(scene, mtlTexture, mtlCmdBuffer);
    }

private:
    skgpu::piet::MtlRenderer fRenderer;
};

}  // namespace

std::unique_ptr<PietRenderer> PietRenderer::MakeMetal(const Gpu* gpu) {
    return std::make_unique<MtlPietRenderer>(static_cast<const MtlGpu*>(gpu));
}

}  // namespace skgpu::graphite
