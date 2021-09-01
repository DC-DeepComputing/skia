/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlMSAALoadManager.h"

#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/mtl/GrMtlGpu.h"

#include "src/gpu/mtl/GrMtlBuffer.h"
#include "src/gpu/mtl/GrMtlCommandBuffer.h"
#include "src/gpu/mtl/GrMtlRenderCommandEncoder.h"

GrMtlMSAALoadManager::GrMtlMSAALoadManager() {}

GrMtlMSAALoadManager::~GrMtlMSAALoadManager() {}

bool GrMtlMSAALoadManager::loadMSAAFromResolve(GrMtlGpu* gpu,
                                               GrAttachment* dst,
                                               GrMtlAttachment* src,
                                               const SkIRect& rect) {
    if (!dst) {
        return false;
    }
    if (!src || src->framebufferOnly()) {
        return false;
    }

    GrMtlAttachment* mtlDst = static_cast<GrMtlAttachment*>(dst);

    auto renderPipeline = gpu->resourceProvider().findOrCreateMSAALoadPipeline(mtlDst->mtlFormat(),
                                                                               dst->numSamples());

    // Set up rendercommandencoder
    auto renderPassDesc = [MTLRenderPassDescriptor new];
    auto colorAttachment = renderPassDesc.colorAttachments[0];
    colorAttachment.texture = mtlDst->mtlTexture();
    colorAttachment.loadAction = MTLLoadActionDontCare;
    colorAttachment.storeAction = MTLStoreActionMultisampleResolve;
    colorAttachment.resolveTexture = src->mtlTexture();

    auto renderCmdEncoder =
                gpu->commandBuffer()->getRenderCommandEncoder(renderPassDesc, nullptr, nullptr);

    // Bind pipeline
    renderCmdEncoder->setRenderPipelineState(renderPipeline->mtlPipelineState());
    gpu->commandBuffer()->addResource(sk_ref_sp(renderPipeline));

    // Bind src as input texture
    renderCmdEncoder->setFragmentTexture(src->mtlTexture(), 0);
    // No sampler needed
    gpu->commandBuffer()->addGrSurface(sk_ref_sp<GrSurface>(src));

    // Scissor and viewport should default to size of color attachment

    // Update and bind uniform data
    int w = rect.width();
    int h = rect.height();

    // dst rect edges in NDC (-1 to 1)
    int dw = dst->width();
    int dh = dst->height();
    float dx0 = 2.f * rect.fLeft / dw - 1.f;
    float dx1 = 2.f * (rect.fLeft + w) / dw - 1.f;
    float dy0 = 2.f * rect.fTop / dh - 1.f;
    float dy1 = 2.f * (rect.fTop + h) / dh - 1.f;

    struct {
        float posXform[4];
        int textureSize[2];
        int pad[2];
    } uniData = {{dx1 - dx0, dy1 - dy0, dx0, dy0}, {dw, dh}, {0, 0}};

    constexpr size_t uniformSize = 32;
    if (@available(macOS 10.11, iOS 8.3, *)) {
        SkASSERT(uniformSize <= gpu->caps()->maxPushConstantsSize());
        renderCmdEncoder->setVertexBytes(&uniData, uniformSize, 0);
    } else {
        // upload the data
        GrRingBuffer::Slice slice = gpu->uniformsRingBuffer()->suballocate(uniformSize);
        GrMtlBuffer* buffer = (GrMtlBuffer*) slice.fBuffer;
        char* destPtr = static_cast<char*>(slice.fBuffer->map()) + slice.fOffset;
        memcpy(destPtr, &uniData, uniformSize);

        renderCmdEncoder->setVertexBuffer(buffer->mtlBuffer(), slice.fOffset, 0);
    }

    renderCmdEncoder->drawPrimitives(MTLPrimitiveTypeTriangleStrip, 0, 4, 1, 0);

    return true;
}

