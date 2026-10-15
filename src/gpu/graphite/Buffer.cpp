/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/Buffer.h"

#include "include/gpu/GpuTypes.h"
#include "include/private/SkAssert.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/SharedContext.h"

namespace skgpu::graphite {

void* Buffer::map() {
#ifdef SK_DEBUG
    SkASSERT(isUnmappable() || !this->Resource::sharedContext()->caps()->bufferMapsAreAsync());
    SkASSERT(isProtected() == Protected::kNo);
#endif
    if (!isMapped()) {
        onMap();
    }
    return fMapPtr;
}

void Buffer::asyncMap(GpuFinishedProc proc, GpuFinishedContext ctx) {
    SkASSERT(this->Resource::sharedContext()->caps()->bufferMapsAreAsync());
    SkASSERT(isProtected() == Protected::kNo);
    onAsyncMap(proc, ctx);
}

void Buffer::unmap() {
    SkASSERT(isUnmappable());
    onUnmap();
    fMapPtr = nullptr;
}

bool Buffer::isUnmappable() const { return isMapped(); }

void Buffer::onAsyncMap(skgpu::graphite::GpuFinishedProc, skgpu::graphite::GpuFinishedContext) {
    SkASSERT(!sharedContext()->caps()->bufferMapsAreAsync());
    SK_ABORT("Async buffer mapping not supported");
}

} // namespace skgpu::graphite

