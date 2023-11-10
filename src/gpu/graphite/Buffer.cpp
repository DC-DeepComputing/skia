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
    if (!this->isMapped()) {
        this->onMap();
    }
    return fMapPtr;
}

void Buffer::asyncMap() {
    SkASSERT(this->sharedContext()->caps()->canAsyncMapBuffer());
    this->onAsyncMap();
}

void Buffer::unmap() {
    SkASSERT(this->isMapped() || this->hasPendingAsyncMap());
    this->onUnmap();
    fMapPtr = nullptr;
}

} // namespace skgpu::graphite

