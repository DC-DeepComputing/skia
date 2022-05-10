/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlComputeCommandEncoder_DEFINED
#define GrMtlComputeCommandEncoder_DEFINED

#include <memory>
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/mtl/GrMtlSampler.h"
#include "src/gpu/ganesh/mtl/GrMtlTexture.h"
#include "src/gpu/ganesh/mtl/GrMtlUniformHandler.h"
#include "src/gpu/ganesh/mtl/GrMtlUtil.h"

class GrMtlSampler;

#import <Metal/Metal.h>

GR_NORETAIN_BEGIN

/**
 * Wraps a MTLComputeCommandEncoder object and associated tracked state
 */
class GrMtlComputeCommandEncoder {
public:
    static std::unique_ptr<GrMtlComputeCommandEncoder> Make(id<MTLComputeCommandEncoder> encoder) {
        return std::unique_ptr<GrMtlComputeCommandEncoder>(new GrMtlComputeCommandEncoder(encoder));
    }

    void setLabel(NSString* label) {
        [fCommandEncoder setLabel:label];
    }

    void pushDebugGroup(NSString* string) {
        [fCommandEncoder pushDebugGroup:string];
    }
    void popDebugGroup() {
        [fCommandEncoder popDebugGroup];
    }
    void insertDebugSignpost(NSString* string) {
        [fCommandEncoder insertDebugSignpost:string];
    }

    void setComputePipelineState(id<MTLComputePipelineState> pso) {
        if (fCurrentComputePipelineState != pso) {
            [fCommandEncoder setComputePipelineState:pso];
            fCurrentComputePipelineState = pso;
        }
    }

    void setBuffer(id<MTLBuffer> buffer, int offset, int index) {
        [fCommandEncoder setBuffer:buffer offset:offset atIndex:index];
    }

    void setTexture(const GrMtlTexture& texture, int index) {
        [fCommandEncoder setTexture:texture.mtlTexture() atIndex:index];
    }

    void execute(int threadGroupsX, int threadGroupsY, int threadGroupsZ,
                 int threadsX,      int threadsY,      int threadsZ) {
        [fCommandEncoder
                dispatchThreadgroups:MTLSizeMake(threadGroupsX, threadGroupsY, threadGroupsZ)
                threadsPerThreadgroup:MTLSizeMake(threadsX, threadsY, threadsZ)];
    }

    void endEncoding() {
        [fCommandEncoder endEncoding];
    }

private:
    GrMtlComputeCommandEncoder(id<MTLComputeCommandEncoder> encoder)
        : fCommandEncoder(encoder) {}

    id<MTLComputeCommandEncoder> fCommandEncoder = nil;

    // As of 2022-03-09 All GPU families have the same value at:
    // https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
    static const int kMaxSamplers = 16;
    // This is a self-imposed limit and is less than any GPU family's value at
    // https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
    // Our implementation uses a 1:1 correspondence between textures and samplers.
    static const int kMaxTextures = kMaxSamplers;

    __weak id<MTLComputePipelineState> fCurrentComputePipelineState = nil;
};

GR_NORETAIN_END

#endif
