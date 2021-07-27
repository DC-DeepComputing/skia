/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/mtl/GrMtlBuffer.h"
#include "src/gpu/mtl/GrMtlCommandBuffer.h"
#include "src/gpu/mtl/GrMtlGpu.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

#ifdef SK_DEBUG
#define VALIDATE() this->validate()
#else
#define VALIDATE() do {} while(false)
#endif

GR_NORETAIN_BEGIN

#ifdef SK_ENABLE_MTL_DEBUG_INFO
NSString* kBufferTypeNames[kGrGpuBufferTypeCount] = {
    @"Vertex",
    @"Index",
    @"Indirect",
    @"Xfer CPU to GPU",
    @"Xfer GPU to CPU",
    @"Uniform",
};
#endif

sk_sp<GrMtlBuffer> GrMtlBuffer::Make(GrMtlGpu* gpu, size_t size, GrGpuBufferType intendedType,
                                     GrAccessPattern accessPattern, const void* data) {
    sk_sp<GrMtlBuffer> buffer(new GrMtlBuffer(gpu, size, intendedType, accessPattern));
    if (data && !buffer->onUpdateData(data, size)) {
        return nullptr;
    }
    return buffer;
}

GrMtlBuffer::GrMtlBuffer(GrMtlGpu* gpu, size_t size, GrGpuBufferType intendedType,
                         GrAccessPattern accessPattern)
        : INHERITED(gpu, size, intendedType, accessPattern)
        , fIsDynamic(accessPattern != kStatic_GrAccessPattern) {
    NSUInteger options = 0;
    if (@available(macOS 10.11, iOS 9.0, *)) {
        if (fIsDynamic) {
#ifdef SK_BUILD_FOR_MAC
            options |= MTLResourceStorageModeManaged;
#else
            options |= MTLResourceStorageModeShared;
#endif
        } else {
            options |= MTLResourceStorageModePrivate;
        }
    }
#ifdef SK_BUILD_FOR_MAC
    // Mac requires 4-byte alignment for copies so we need
    // to ensure we have space for the extra data
    size = SkAlign4(size);
#endif
    fMtlBuffer = size == 0 ? nil :
            [gpu->device() newBufferWithLength: size
                                       options: options];
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    fMtlBuffer.label = kBufferTypeNames[(int)intendedType];
#endif
    this->registerWithCache(SkBudgeted::kYes);
    VALIDATE();
}

GrMtlBuffer::~GrMtlBuffer() {
    SkASSERT(fMtlBuffer == nil);
    SkASSERT(fStagingSlice.fBuffer == nullptr);
    SkASSERT(fMapPtr == nullptr);
}

bool GrMtlBuffer::onUpdateData(const void* src, size_t srcInBytes) {
    if (!fIsDynamic) {
        if (fMtlBuffer == nil) {
            return false;
        }
        if (srcInBytes > fMtlBuffer.length) {
            return false;
        }
    }
    VALIDATE();

    this->internalMap(srcInBytes);
    if (fMapPtr == nil) {
        return false;
    }
    SkASSERT(fStagingSlice.fBuffer);
    memcpy(fMapPtr, src, srcInBytes);
    this->internalUnmap(srcInBytes);

    VALIDATE();
    return true;
}

inline GrMtlGpu* GrMtlBuffer::mtlGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrMtlGpu*>(this->getGpu());
}

void GrMtlBuffer::onAbandon() {
    fMtlBuffer = nil;
    fStagingSlice.fBuffer = nullptr;
    fMapPtr = nullptr;
    VALIDATE();
    INHERITED::onAbandon();
}

void GrMtlBuffer::onRelease() {
    if (!this->wasDestroyed()) {
        VALIDATE();
        fMtlBuffer = nil;
        fStagingSlice.fBuffer = nullptr;
        fMapPtr = nullptr;
        VALIDATE();
    }
    INHERITED::onRelease();
}

void GrMtlBuffer::internalMap(size_t sizeInBytes) {
    if (this->wasDestroyed()) {
        return;
    }
    VALIDATE();
    SkASSERT(!this->isMapped());
    if (fIsDynamic) {
        fMapPtr = static_cast<char*>(fMtlBuffer.contents);
    } else {
        SkASSERT(fMtlBuffer);
        SkASSERT(fStagingSlice.fBuffer == nullptr);

#ifdef SK_BUILD_FOR_MAC
        // Mac requires 4-byte alignment for copies
        // TODO: true for Apple Silicon?
        static const size_t kMinAlignment = 4;
#else
        static const size_t kMinAlignment = 1;
#endif
        fStagingSlice = this->mtlGpu()->stagingBufferManager()->allocateStagingBufferSlice(
                sizeInBytes, kMinAlignment);
        SkASSERT(fStagingSlice.fBuffer);
        fMapPtr = fStagingSlice.fOffsetMapPtr;
    }
    VALIDATE();
}

void GrMtlBuffer::internalUnmap(size_t sizeInBytes) {
    SkASSERT(fMtlBuffer);
    if (this->wasDestroyed()) {
        return;
    }
    VALIDATE();
    SkASSERT(this->isMapped());
    if (fMtlBuffer == nil) {
        fMapPtr = nullptr;
        return;
    }

    if (fIsDynamic) {
#ifdef SK_BUILD_FOR_MAC
        [fMtlBuffer didModifyRange: NSMakeRange(0, sizeInBytes)];
#endif
    } else {
        GrMtlCommandBuffer* cmdBuffer = this->mtlGpu()->commandBuffer();
        id<MTLBlitCommandEncoder> GR_NORETAIN blitCmdEncoder = cmdBuffer->getBlitCommandEncoder();
        GrMtlBuffer* mtlBuffer = static_cast<GrMtlBuffer*>(fStagingSlice.fBuffer);
        id<MTLBuffer> transferBuffer = mtlBuffer->mtlBuffer();
        [blitCmdEncoder copyFromBuffer: transferBuffer
                          sourceOffset: fStagingSlice.fOffset
                              toBuffer: fMtlBuffer
                     destinationOffset: 0
                                  size: sizeInBytes];
    }
    fStagingSlice.fBuffer = nullptr;
    fMapPtr = nullptr;
}

void GrMtlBuffer::onMap() {
    this->internalMap(this->size());
}

void GrMtlBuffer::onUnmap() {
    this->internalUnmap(this->size());
}

#ifdef SK_DEBUG
void GrMtlBuffer::validate() const {
    SkASSERT(fMtlBuffer == nil ||
             this->intendedType() == GrGpuBufferType::kVertex ||
             this->intendedType() == GrGpuBufferType::kIndex ||
             this->intendedType() == GrGpuBufferType::kXferCpuToGpu ||
             this->intendedType() == GrGpuBufferType::kXferGpuToCpu ||
             this->intendedType() == GrGpuBufferType::kDrawIndirect ||
             this->intendedType() == GrGpuBufferType::kUniform);
}
#endif

GR_NORETAIN_END
