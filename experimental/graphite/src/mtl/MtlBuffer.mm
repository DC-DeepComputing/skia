/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlBuffer.h"

#include "experimental/graphite/src/mtl/MtlGpu.h"

namespace skgpu::mtl {

#ifdef SK_ENABLE_MTL_DEBUG_INFO
NSString* kBufferTypeNames[kBufferTypeCount] = {
    @"Vertex",
    @"Index",
    @"Xfer CPU to GPU",
    @"Xfer GPU to CPU",
    @"Uniform",
};
#endif

sk_sp<Buffer> Buffer::Make(const Gpu* gpu,
                           size_t size,
                           BufferType type,
                           TransientGpuBuffer transient) {
    if (size <= 0) {
        return nullptr;
    }

    NSUInteger options = 0;
    if (@available(macOS 10.11, iOS 9.0, *)) {
        if (transient == TransientGpuBuffer::kYes) {
            if (gpu->mtlCaps().isMac()) {
                options |= MTLResourceStorageModeManaged;
            } else {
                SkASSERT(gpu->mtlCaps().isApple());
                options |= MTLResourceStorageModeShared;
            }
        } else {
            options |= MTLResourceStorageModePrivate;
        }
    }
#ifdef SK_BUILD_FOR_MAC
    // Mac requires 4-byte alignment for copies so we need to ensure we have space for the extra
    // data.
    // TODO: Confirm if this is needed for M1 or not
    size = SkAlign4(size);
#endif
    sk_cfp<id<MTLBuffer>> buffer([gpu->device() newBufferWithLength: size options: options]);
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    (*buffer).label = kBufferTypeNames[(int)type];
#endif

    return sk_sp<Buffer>(new Buffer(size, type, transient, std::move(buffer)));
}

Buffer::Buffer(size_t size,
               BufferType type,
               TransientGpuBuffer transient,
               sk_cfp<id<MTLBuffer>> buffer)
        : skgpu::Buffer(size, type, transient)
        , fBuffer(std::move(buffer)) {}

void Buffer::onMap() {
    SkASSERT(fBuffer);
    SkASSERT(!this->isMapped());

    if ((*fBuffer).storageMode == MTLStorageModePrivate) {
        return;
    }

    fMapPtr = static_cast<char*>((*fBuffer).contents);
}

void Buffer::onUnmap() {
    SkASSERT(fBuffer);
    SkASSERT(this->isMapped());
    if ((*fBuffer).storageMode == MTLStorageModeManaged) {
        [*fBuffer didModifyRange: NSMakeRange(0, this->size())];
    }
    fMapPtr = nullptr;
}

} // namespace skgpu::mtl

