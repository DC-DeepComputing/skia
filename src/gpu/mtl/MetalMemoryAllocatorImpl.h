/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlMemoryAllocatorImpl_DEFINED
#define GrMtlMemoryAllocatorImpl_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/mtl/MetalMemoryAllocator.h"

#import <Metal/Metal.h>

namespace skgpu {

class MetalMemoryAllocatorImpl : public MetalMemoryAllocator {
public:
    static sk_sp<MetalMemoryAllocator> Make(id<MTLDevice>);

    ~MetalMemoryAllocatorImpl() override {}

    id<MTLBuffer> newBufferWithLength(NSUInteger length, MTLResourceOptions options,
                                      sk_sp<skgpu::MetalAlloc>* allocation) override;
    id<MTLTexture> newTextureWithDescriptor(MTLTextureDescriptor* texDesc,
                                            sk_sp<skgpu::MetalAlloc>* allocation) override;

    class Alloc : public MetalAlloc {
    public:
        Alloc() {}
        ~Alloc() override {
            // TODO: free allocation
        }
    private:
        friend class MetalMemoryAllocatorImpl;
        // TODO: allocation data goes here
    };

private:
    MetalMemoryAllocatorImpl(id<MTLDevice> device) : fDevice(device) {}

    id<MTLDevice> fDevice;
};

} // namespace skgpu

#endif
