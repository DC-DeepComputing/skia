/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlComputeCommandEncoder_DEFINED
#define skgpu_graphite_MtlComputeCommandEncoder_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/ports/SkCFObject.h"
#include "src/gpu/graphite/Resource.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {

/**
 * Wraps a MTLComputeCommandEncoder object and associated tracked state
 */
class MtlComputeCommandEncoder : public Resource {
public:
    static sk_sp<MtlComputeCommandEncoder> Make(const Gpu* gpu,
                                                id<MTLCommandBuffer> commandBuffer) {
        // Adding a retain here to keep our own ref separate from the autorelease pool
        sk_cfp<id<MTLComputeCommandEncoder>> encoder =
                 sk_ret_cfp([commandBuffer computeCommandEncoder]);
        return sk_sp<MtlComputeCommandEncoder>(new MtlComputeCommandEncoder(gpu,
                                                                            std::move(encoder)));
    }

    void setLabel(NSString* label) {
        [(*fCommandEncoder) setLabel:label];
    }

    void pushDebugGroup(NSString* string) {
        [(*fCommandEncoder) pushDebugGroup:string];
    }

    void popDebugGroup() {
        [(*fCommandEncoder) popDebugGroup];
    }

    void insertDebugSignpost(NSString* string) {
        [(*fCommandEncoder) insertDebugSignpost:string];
    }

    void setComputePipelineState(id<MTLComputePipelineState> pso) {
        if (fCurrentComputePipelineState != pso) {
            [(*fCommandEncoder) setComputePipelineState:pso];
            fCurrentComputePipelineState = pso;
        }
    }

    void setBuffer(id<MTLBuffer> buffer, int offset, int index) {
        printf("setbuffer: %d, %d\n", offset, index);
        [(*fCommandEncoder) setBuffer:buffer offset:offset atIndex:index];
    }

    void execute(int threadGroupsX, int threadGroupsY, int threadGroupsZ,
                 int threadsX,      int threadsY,      int threadsZ) {

        printf("execute %d\n", threadsX);
        [(*fCommandEncoder)
                dispatchThreadgroups:MTLSizeMake(threadGroupsX, threadGroupsY, threadGroupsZ)
                threadsPerThreadgroup:MTLSizeMake(threadsX, threadsY, threadsZ)];
    }

    void endEncoding() {
        printf("end!\n");
        [(*fCommandEncoder) endEncoding];
    }

private:
    MtlComputeCommandEncoder(const Gpu* gpu, sk_cfp<id<MTLComputeCommandEncoder>> encoder)
        : Resource(gpu, Ownership::kOwned, SkBudgeted::kYes)
        , fCommandEncoder(std::move(encoder)) {}

    void freeGpuData() override {
        fCommandEncoder.reset();
    }

    sk_cfp<id<MTLComputeCommandEncoder>> fCommandEncoder = nil;
    id<MTLComputePipelineState> fCurrentComputePipelineState = nil;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlRenderCommandEncoder_DEFINED
