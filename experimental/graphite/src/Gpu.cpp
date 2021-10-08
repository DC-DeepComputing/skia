/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Gpu.h"

#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/ResourceProvider.h"

namespace skgpu {

Gpu::Gpu(sk_sp<const Caps> caps)
    : fCaps(std::move(caps)) {
    // subclasses create their own subclassed resource provider
}

Gpu::~Gpu() {
    // TODO: add disconnect and destroyResources?

    // TODO: how do we handle command buffers that haven't been submitted yet?


    // TODO: destroyResources instead?
    fResourceProvider.reset();
}

bool Gpu::submit(CommandBuffer* commandBuffer, SyncQueue sync) {
    if (!commandBuffer) {
        return false;
    }

    if (!commandBuffer->hasWork()) {
        if (sync == SyncQueue::kForce) {
            this->resourceProvider()->forceSyncCommandBuffers();
        }
        // TODO: call finished procs?
        this->resourceProvider()->checkCommandBuffers();
        return true;
    }

    SkASSERT(commandBuffer);
    if (!this->onSubmit(commandBuffer, sync)) {
        return false;
    }

    this->resourceProvider()->checkCommandBuffers();

    return true;
}

} // namespace skgpu
