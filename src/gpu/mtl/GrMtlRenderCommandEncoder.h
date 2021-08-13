/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlRenderCommandEncoder_DEFINED
#define GrMtlRenderCommandEncoder_DEFINED

#include <memory>
#include "src/gpu/GrSamplerState.h"
#include "src/gpu/mtl/GrMtlSampler.h"
#include "src/gpu/mtl/GrMtlUniformHandler.h"
#include "src/gpu/mtl/GrMtlUtil.h"

class GrMtlSampler;

#import <Metal/Metal.h>

/**
 * Wraps a MTLRenderCommandEncoder object and associated tracked state
 */
class GrMtlRenderCommandEncoder {
public:
    static std::unique_ptr<GrMtlRenderCommandEncoder> Make(id<MTLCommandBuffer> cmdBuffer,
                                                           MTLRenderPassDescriptor* desc) {
        // We retain here because the encoder is set to autorelease (not alloc or copy)
        auto encoder = sk_ret_cfp<id<MTLRenderCommandEncoder>>(
                [cmdBuffer renderCommandEncoderWithDescriptor:desc]);
        return std::unique_ptr<GrMtlRenderCommandEncoder>(
                new GrMtlRenderCommandEncoder(std::move(encoder)));
    }

    ~GrMtlRenderCommandEncoder() {
        fCommandEncoder.reset();
    }

    void setLabel(NSString* label) {
        [*fCommandEncoder setLabel:label];
    }

    void pushDebugGroup(NSString* string) {
        [*fCommandEncoder pushDebugGroup:string];
    }
    void popDebugGroup() {
        [*fCommandEncoder popDebugGroup];
    }
    void insertDebugSignpost(NSString* string) {
        [*fCommandEncoder insertDebugSignpost:string];
    }

    void setRenderPipelineState(id<MTLRenderPipelineState> pso) {
        if (fCurrentRenderPipelineState != pso) {
            [*fCommandEncoder setRenderPipelineState:pso];
            fCurrentRenderPipelineState = pso;
        }
    }

    void setTriangleFillMode(MTLTriangleFillMode fillMode) {
        if (fCurrentTriangleFillMode != fillMode) {
            [*fCommandEncoder setTriangleFillMode:fillMode];
            fCurrentTriangleFillMode = fillMode;
        }
    }

    void setFrontFacingWinding(MTLWinding winding) {
        [*fCommandEncoder setFrontFacingWinding:winding];
    }

    void setViewport(const MTLViewport& viewport) {
        [*fCommandEncoder setViewport:viewport];
    }

    void setVertexBuffer(id<MTLBuffer> buffer, NSUInteger offset, NSUInteger index) {
        if (@available(macOS 10.11, iOS 8.3, *)) {
            if (fCurrentVertexBuffer[index] == buffer) {
                this->setVertexBufferOffset(offset, index);
                return;
            }
        }
        if (fCurrentVertexBuffer[index] != buffer || fCurrentVertexOffset[index] != offset) {
            [*fCommandEncoder setVertexBuffer:buffer
                                       offset:offset
                                      atIndex:index];
            fCurrentVertexBuffer[index] = buffer;
            fCurrentVertexOffset[index] = offset;
        }
    }
    void setVertexBufferOffset(NSUInteger offset, NSUInteger index)
            SK_API_AVAILABLE(macos(10.11), ios(8.3)) {
        if (fCurrentVertexOffset[index] != offset) {
            [*fCommandEncoder setVertexBufferOffset:offset
                                            atIndex:index];
            fCurrentVertexOffset[index] = offset;
        }
    }

    void setFragmentBuffer(id<MTLBuffer> buffer, NSUInteger offset, NSUInteger index) {
        if (@available(macOS 10.11, iOS 8.3, *)) {
            if (fCurrentFragmentBuffer[index] == buffer) {
                this->setFragmentBufferOffset(offset, index);
                return;
            }
        }
        if (fCurrentFragmentBuffer[index] != buffer || fCurrentFragmentOffset[index] != offset) {
            [*fCommandEncoder setFragmentBuffer:buffer
                                         offset:offset
                                        atIndex:index];
            fCurrentFragmentBuffer[index] = buffer;
            fCurrentFragmentOffset[index] = offset;
        }
    }
    void setFragmentBufferOffset(NSUInteger offset, NSUInteger index)
            SK_API_AVAILABLE(macos(10.11), ios(8.3)) {
        if (fCurrentFragmentOffset[index] != offset) {
            [*fCommandEncoder setFragmentBufferOffset:offset
                                              atIndex:index];
            fCurrentFragmentOffset[index] = offset;
        }
    }

    void setVertexBytes(const void* bytes, NSUInteger length, NSUInteger index)
            SK_API_AVAILABLE(macos(10.11), ios(8.3)) {
        [*fCommandEncoder setVertexBytes:bytes
                                  length:length
                                 atIndex:index];
    }
    void setFragmentBytes(const void* bytes, NSUInteger length, NSUInteger index)
            SK_API_AVAILABLE(macos(10.11), ios(8.3)) {
        [*fCommandEncoder setFragmentBytes:bytes
                                    length:length
                                   atIndex:index];
    }

    void setFragmentTexture(id<MTLTexture> texture, NSUInteger index) {
        SkASSERT(index < 16);
        if (fCurrentTexture[index] != texture) {
            [*fCommandEncoder setFragmentTexture:texture
                                         atIndex:index];
            fCurrentTexture[index] = texture;
        }
    }
    void setFragmentSamplerState(GrMtlSampler* sampler, NSUInteger index) {
        if (fCurrentSampler[index] != sampler) {
            [*fCommandEncoder setFragmentSamplerState: sampler->mtlSampler()
                                              atIndex: index];
            fCurrentSampler[index] = sampler;
        }
    }

    void setBlendColor(SkPMColor4f blendConst) {
        [*fCommandEncoder setBlendColorRed: blendConst.fR
                                    green: blendConst.fG
                                     blue: blendConst.fB
                                    alpha: blendConst.fA];
    }

    void setStencilFrontBackReferenceValues(uint32_t frontReferenceValue,
                                            uint32_t backReferenceValue)
            SK_API_AVAILABLE(macos(10.11), ios(9.0)) {
        [*fCommandEncoder
                setStencilFrontReferenceValue:frontReferenceValue
                           backReferenceValue:backReferenceValue];
    }
    void setStencilReferenceValue(uint32_t referenceValue) {
        [*fCommandEncoder setStencilReferenceValue:referenceValue];
    }
    void setDepthStencilState(id<MTLDepthStencilState> depthStencilState) {
        if (depthStencilState != fCurrentDepthStencilState) {
            [*fCommandEncoder setDepthStencilState:depthStencilState];
            fCurrentDepthStencilState = depthStencilState;
        }
    }

    void setScissorRect(const MTLScissorRect& scissorRect) {
        if (fCurrentScissorRect.x != scissorRect.x ||
            fCurrentScissorRect.y != scissorRect.y ||
            fCurrentScissorRect.width != scissorRect.width ||
            fCurrentScissorRect.height != scissorRect.height) {
            [*fCommandEncoder setScissorRect:scissorRect];
            fCurrentScissorRect = scissorRect;
        }
    }

    void drawPrimitives(MTLPrimitiveType primitiveType, NSUInteger vertexStart,
                        NSUInteger vertexCount) {
        [*fCommandEncoder drawPrimitives:primitiveType
                             vertexStart:vertexStart
                             vertexCount:vertexCount];
    }
    void drawPrimitives(MTLPrimitiveType primitiveType, NSUInteger vertexStart,
                        NSUInteger vertexCount, NSUInteger instanceCount,
                        NSUInteger baseInstance) SK_API_AVAILABLE(macos(10.11), ios(9.0)) {
        [*fCommandEncoder drawPrimitives:primitiveType
                             vertexStart:vertexStart
                             vertexCount:vertexCount
                           instanceCount:instanceCount
                            baseInstance:baseInstance];
    }
    void drawPrimitives(MTLPrimitiveType primitiveType, id<MTLBuffer> indirectBuffer,
                        NSUInteger indirectBufferOffset) SK_API_AVAILABLE(macos(10.11), ios(9.0)) {
        [*fCommandEncoder drawPrimitives:primitiveType
                          indirectBuffer:indirectBuffer
                    indirectBufferOffset:indirectBufferOffset];
    }

    void drawIndexedPrimitives(MTLPrimitiveType primitiveType, NSUInteger indexCount,
                               MTLIndexType indexType, id<MTLBuffer> indexBuffer,
                               NSUInteger indexBufferOffset) {
        [*fCommandEncoder drawIndexedPrimitives:primitiveType
                                     indexCount:indexCount
                                      indexType:indexType
                                    indexBuffer:indexBuffer
                              indexBufferOffset:indexBufferOffset];
    }
    void drawIndexedPrimitives(MTLPrimitiveType primitiveType, NSUInteger indexCount,
                               MTLIndexType indexType, id<MTLBuffer> indexBuffer,
                               NSUInteger indexBufferOffset,
                               NSUInteger instanceCount,
                               NSInteger baseVertex,
                               NSUInteger baseInstance) SK_API_AVAILABLE(macos(10.11), ios(9.0)) {
        [*fCommandEncoder drawIndexedPrimitives:primitiveType
                                     indexCount:indexCount
                                      indexType:indexType
                                    indexBuffer:indexBuffer
                              indexBufferOffset:indexBufferOffset
                                  instanceCount:instanceCount
                                     baseVertex:baseVertex
                                   baseInstance:baseInstance];
    }
    void drawIndexedPrimitives(MTLPrimitiveType primitiveType,
                               MTLIndexType indexType, id<MTLBuffer> indexBuffer,
                               NSUInteger indexBufferOffset, id<MTLBuffer> indirectBuffer,
                               NSUInteger indirectBufferOffset)
            SK_API_AVAILABLE(macos(10.11), ios(9.0)) {
        [*fCommandEncoder drawIndexedPrimitives:primitiveType
                                      indexType:indexType
                                    indexBuffer:indexBuffer
                              indexBufferOffset:indexBufferOffset
                                 indirectBuffer:indirectBuffer
                           indirectBufferOffset:indirectBufferOffset];
    }

    void endEncoding() {
        [*fCommandEncoder endEncoding];
    }

private:
    GrMtlRenderCommandEncoder(sk_cfp<id<MTLRenderCommandEncoder>> encoder)
        : fCommandEncoder(std::move(encoder)) {}

    sk_cfp<id<MTLRenderCommandEncoder>> fCommandEncoder;

    id<MTLRenderPipelineState> fCurrentRenderPipelineState = nil;
    id<MTLDepthStencilState> fCurrentDepthStencilState = nil;
    id<MTLBuffer> fCurrentVertexBuffer[2 + GrMtlUniformHandler::kUniformBindingCount] = { nil };
    NSUInteger fCurrentVertexOffset[2 + GrMtlUniformHandler::kUniformBindingCount];
    id<MTLBuffer> fCurrentFragmentBuffer[GrMtlUniformHandler::kUniformBindingCount] = { nil };
    NSUInteger fCurrentFragmentOffset[2 + GrMtlUniformHandler::kUniformBindingCount];
    id<MTLTexture> fCurrentTexture[GrSamplerState::kNumUniqueSamplers] = { nil };
    GrMtlSampler* fCurrentSampler[GrSamplerState::kNumUniqueSamplers] = { 0 };
    MTLScissorRect fCurrentScissorRect = { 0, 0, 0, 0 };
    MTLTriangleFillMode fCurrentTriangleFillMode = (MTLTriangleFillMode)-1;
};

#endif
