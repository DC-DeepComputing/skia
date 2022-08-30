/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlMemoryAllocatorImpl.h"

namespace skgpu {

sk_sp<MetalMemoryAllocator> MetalMemoryAllocatorImpl::Make(id<MTLDevice> device) {
    return sk_sp<MetalMemoryAllocator>(new MetalMemoryAllocatorImpl(device));
}

id<MTLBuffer> MetalMemoryAllocatorImpl::newBufferWithLength(NSUInteger length,
                                                            MTLResourceOptions options,
                                                            sk_sp<MetalAlloc>* allocation) {
    // TODO: suballocate and fill in Alloc
    allocation->reset(new Alloc());
    return [fDevice newBufferWithLength:length options:options];
}

id<MTLTexture> MetalMemoryAllocatorImpl::newTextureWithDescriptor(MTLTextureDescriptor* texDesc,
                                                                  sk_sp<MetalAlloc>* allocation) {
    // TODO: suballocate and fill in Alloc
    allocation->reset(new Alloc());
    return [fDevice newTextureWithDescriptor:texDesc];
}

} // namespace skgpu
