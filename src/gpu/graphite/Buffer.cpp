/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/SharedContext.h"

namespace skgpu::graphite {

const void* Buffer::mapRead() {
    SkASSERT(this->isUnmappable() || !this->sharedContext()->caps()->bufferMapsAreAsync());
    if (!this->isMapped()) {
        this->onMap(/*mapForWrite=*/false);
    }
    SkASSERT(isMappedRead());
    return fConstMapPtr;
}

void* Buffer::mapWrite() {
    SkASSERT(this->isUnmappable() || !this->sharedContext()->caps()->bufferMapsAreAsync());
    if (!this->isMapped()) {
        this->onMap(/*mapForWrite=*/true);
    }
    SkASSERT(isMappedWrite());
    return fMapPtr;
}

void Buffer::asyncMapRead(GpuFinishedProc proc, GpuFinishedContext ctx) {
    SkASSERT(this->sharedContext()->caps()->bufferMapsAreAsync());
    this->onAsyncMap(proc, ctx, /*mapForWrite=*/false);
}

void Buffer::asyncMapWrite(GpuFinishedProc proc, GpuFinishedContext ctx) {
    SkASSERT(this->sharedContext()->caps()->bufferMapsAreAsync());
    this->onAsyncMap(proc, ctx, /*mapForWrite=*/true);
}

void Buffer::unmap() {
    SkASSERT(this->isUnmappable());
    this->onUnmap();
    fMapPtr = nullptr;
    fConstMapPtr = nullptr;
}

bool Buffer::isUnmappable() const { return isMapped(); }

void Buffer::onAsyncMap(skgpu::graphite::GpuFinishedProc,
                        skgpu::graphite::GpuFinishedContext,
                        bool mapForWrite) {
    SkASSERT(!this->sharedContext()->caps()->bufferMapsAreAsync());
    SK_ABORT("Async buffer mapping not supported");
}

} // namespace skgpu::graphite

