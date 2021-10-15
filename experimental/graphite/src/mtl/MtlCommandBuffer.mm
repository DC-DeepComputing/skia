/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlCommandBuffer.h"

#include "experimental/graphite/src/mtl/MtlBuffer.h"
#include "experimental/graphite/src/mtl/MtlGpu.h"
#include "experimental/graphite/src/mtl/MtlTexture.h"
#include "include/core/SkRect.h"

namespace skgpu::mtl {

sk_sp<CommandBuffer> CommandBuffer::Make(const Gpu* gpu) {
    sk_cfp<id<MTLCommandBuffer>> cmdBuffer;
    id<MTLCommandQueue> queue = gpu->queue();
    if (@available(macOS 11.0, iOS 14.0, tvOS 14.0, *)) {
        sk_cfp<MTLCommandBufferDescriptor*> desc([[MTLCommandBufferDescriptor alloc] init]);
        (*desc).retainedReferences = NO;
#ifdef SK_ENABLE_MTL_DEBUG_INFO
        (*desc).errorOptions = MTLCommandBufferErrorOptionEncoderExecutionStatus;
#endif
        // We add a retain here because the command buffer is set to autorelease (not alloc or copy)
        cmdBuffer.reset([[queue commandBufferWithDescriptor:desc.get()] retain]);
    } else {
        // We add a retain here because the command buffer is set to autorelease (not alloc or copy)
        cmdBuffer.reset([[queue commandBufferWithUnretainedReferences] retain]);
    }
    if (cmdBuffer == nil) {
        return nullptr;
    }

#ifdef SK_ENABLE_MTL_DEBUG_INFO
     (*cmdBuffer).label = @"CommandBuffer::Make";
#endif

    return sk_sp<CommandBuffer>(new CommandBuffer(gpu, std::move(cmdBuffer)));
}

bool CommandBuffer::commit() {
    // TODO: end any encoding
    [(*fCommandBuffer) commit];

    // TODO: better error reporting
    if ((*fCommandBuffer).status == MTLCommandBufferStatusError) {
        NSString* description = (*fCommandBuffer).error.localizedDescription;
        const char* errorString = [description UTF8String];
        SkDebugf("Error submitting command buffer: %s\n", errorString);
    }

    return ((*fCommandBuffer).status != MTLCommandBufferStatusError);
}

static bool check_max_blit_width(int widthInPixels) {
    if (widthInPixels > 32767) {
        SkASSERT(false); // surfaces should not be this wide anyway
        return false;
    }
    return true;
}

id<MTLBlitCommandEncoder> CommandBuffer::getBlitCommandEncoder() {
    if (fActiveBlitCommandEncoder) {
        return fActiveBlitCommandEncoder.get();
    }

    // We retain here because the encoder is set to autorelease (not alloc or copy)
    fActiveBlitCommandEncoder =
            sk_ret_cfp<id<MTLBlitCommandEncoder>>([*fCommandBuffer blitCommandEncoder]);

    return fActiveBlitCommandEncoder.get();
}

void CommandBuffer::copyTextureToBuffer(sk_sp<skgpu::Texture> texture,
                                        SkIRect srcRect,
                                        sk_sp<skgpu::Buffer> buffer,
                                        size_t bufferOffset,
                                        size_t bufferRowBytes) {
    SkASSERT(!fInRenderPass);

    if (!check_max_blit_width(srcRect.width())) {
        return;
    }

    id<MTLTexture> mtlTexture = static_cast<Texture*>(texture.get())->mtlTexture();
    id<MTLBuffer> mtlBuffer = static_cast<Buffer*>(buffer.get())->mtlBuffer();

    id<MTLBlitCommandEncoder> blitCmdEncoder = this->getBlitCommandEncoder();

#ifdef SK_ENABLE_MTL_DEBUG_INFO
    [blitCmdEncoder pushDebugGroup:@"readOrTransferPixels"];
#endif
    [blitCmdEncoder copyFromTexture: mtlTexture
                        sourceSlice: 0
                        sourceLevel: 0
                       sourceOrigin: MTLOriginMake(srcRect.left(), srcRect.top(), 0)
                         sourceSize: MTLSizeMake(srcRect.width(), srcRect.height(), 1)
                           toBuffer: mtlBuffer
                  destinationOffset: bufferOffset
             destinationBytesPerRow: bufferRowBytes
           destinationBytesPerImage: bufferRowBytes * srcRect.height()];

#ifdef SK_BUILD_FOR_MAC
    if (fGpu->mtlCaps().isMac()) {
        // Sync GPU data back to the CPU
        [blitCmdEncoder synchronizeResource: mtlBuffer];
    }
#endif
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    [blitCmdEncoder popDebugGroup];
#endif

    // TODO: Save ref the Texture, Buffer, and any mtl objects on the command buffer

    fHasWork = true;
}

} // namespace skgpu::mtl
