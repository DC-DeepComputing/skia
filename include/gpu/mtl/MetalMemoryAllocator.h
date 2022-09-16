/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlMemoryAllocator_DEFINED
#define GrMtlMemoryAllocator_DEFINED

#ifdef __APPLE__

#ifdef __OBJC__
#import <Metal/Metal.h>
#endif

namespace skgpu {

// interface classes for the GPU memory allocator
class MetalAlloc : public SkRefCnt {
public:
    ~MetalAlloc() override = default;
};

class MetalMemoryAllocator : public SkRefCnt {
public:
#ifdef __OBJC__
    virtual id<MTLBuffer> newBufferWithLength(NSUInteger length, MTLResourceOptions options,
                                              sk_sp<MetalAlloc>* allocation) = 0;
    virtual id<MTLTexture> newTextureWithDescriptor(MTLTextureDescriptor* texDesc,
                                                    sk_sp<MetalAlloc>* allocation) = 0;
#endif
};

}  // namespace skgpu

#endif // __APPLE__

#endif // GrMtlMemoryAllocator_DEFINED
