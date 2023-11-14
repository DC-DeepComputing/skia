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

void* Buffer::map() {
    SkASSERT(this->isMapped() || this->hasPendingAsyncMap() ||
             !this->sharedContext()->caps()->bufferMapsAreAsync());
    if (!this->isMapped()) {
        this->onMap();
    }
    return fMapPtr;
}

void Buffer::asyncMap(GpuFinishedProc proc, GpuFinishedContext ctx) {
    SkASSERT(this->sharedContext()->caps()->bufferMapsAreAsync());
    if (this->isMapped()) {
        if (proc) {
            proc(ctx, CallbackResult::kSuccess);
        }
        return;
    }
    if (proc) {
        fAsyncMapCallbacks.push_back(RefCntedCallback::Make(proc, ctx));
    }
    if (!this->hasPendingAsyncMap()) {
        this->onAsyncMap();
    }
}

void Buffer::unmap() {
    SkASSERT(this->isMapped() || this->hasPendingAsyncMap());
    this->onUnmap();
    fMapPtr = nullptr;
}

void Buffer::callAsyncMapCallbacks(CallbackResult result) {
    if (result == CallbackResult::kFailed) {
        for (auto& cb : fAsyncMapCallbacks) {
            cb->setFailureResult();
        }
    }
    fAsyncMapCallbacks.clear();
}

} // namespace skgpu::graphite

