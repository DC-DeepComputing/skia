/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlCommandBuffer_DEFINED
#define skgpu_MtlCommandBuffer_DEFINED

#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/GpuWorkSubmission.h"
// TODO: get rid of this?
#include "experimental/graphite/src/mtl/MtlRenderCommandEncoder.h"

#include <memory>

#include "include/core/SkTypes.h"
#include "include/ports/SkCFObject.h"
#include "include/private/SkTArray.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {
class Gpu;
class RenderCommandEncoder;

class CommandBuffer final : public skgpu::CommandBuffer {
public:
    static sk_sp<CommandBuffer> Make(const Gpu*);
    ~CommandBuffer() override {}

    bool isFinished() {
        return (*fCommandBuffer).status == MTLCommandBufferStatusCompleted ||
               (*fCommandBuffer).status == MTLCommandBufferStatusError;

    }
    void waitUntilFinished() {
        // TODO: it's not clear what do to if status is Enqueued. Commit and then wait?
        if ((*fCommandBuffer).status == MTLCommandBufferStatusCommitted) {
            [(*fCommandBuffer) waitUntilCompleted];
        }
    }
    bool commit();

private:
    CommandBuffer(sk_cfp<id<MTLCommandBuffer>> cmdBuffer)
        : fCommandBuffer(std::move(cmdBuffer)) {}

    void beginRenderPass(const RenderPassDesc&) override;
    void endRenderPass() override;

    void setRenderPipeline(const RenderPipeline*) override;
    void drawPrimitives(PrimitiveType, size_t vertexOffset, unsigned int vertexCount) override;

    sk_cfp<id<MTLCommandBuffer>> fCommandBuffer;
    sk_sp<RenderCommandEncoder> fActiveRenderCommandEncoder;

};

} // namespace skgpu::mtl

#endif // skgpu_MtlCommandBuffer_DEFINED
