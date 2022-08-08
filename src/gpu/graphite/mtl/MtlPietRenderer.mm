/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlPietRenderer.h"

#include "src/gpu/graphite/mtl/MtlCommandBuffer.h"
#include "src/gpu/graphite/mtl/MtlTexture.h"

namespace skgpu::graphite {

// The piet library internally retains `device` and `queue`, hence we can operate over raw id
// types without needing to store sk_cfp's.
MtlPietRenderer::MtlPietRenderer(id<MTLDevice> device, id<MTLCommandQueue> queue)
        : fRenderer(device, queue) {
    SkASSERT(device);
    SkASSERT(queue);
}

void MtlPietRenderer::encodeSceneRender(const skgpu::piet::Scene& scene,
                                        const Texture* target,
                                        CommandBuffer* cmdBuffer) const {
    id<MTLTexture> mtlTexture = static_cast<const MtlTexture*>(target)->mtlTexture();
    id<MTLCommandBuffer> mtlCmdBuffer =
            static_cast<MtlCommandBuffer*>(cmdBuffer)->getCommandBufferForDirectEncoding();
    fRenderer.render(scene, mtlTexture, mtlCmdBuffer);
}

}  // namespace skgpu::graphite
