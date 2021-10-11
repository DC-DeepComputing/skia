/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Gpu.h"

#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/GpuWorkSubmission.h"
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

bool Gpu::submit(sk_sp<CommandBuffer> commandBuffer, SyncToCpu sync) {
    if (!commandBuffer) {
        return false;
    }

    if (!commandBuffer->hasWork()) {
        if (sync == SyncToCpu::kYes) {
            for (int i = fActiveGpuWork.count()-1; fActiveGpuWork.count() && i >= 0; --i) {
                std::unique_ptr<GpuWorkSubmission>& workSubmission = fActiveGpuWork[i];
                workSubmission->waitUntilFinished(this);
            }
        }
        // TODO: call finished procs?
        this->checkForFinishedWork();
        return true;
    }

    SkASSERT(commandBuffer);
    if (!this->onSubmit(commandBuffer, sync)) {
        return false;
    }

    this->checkForFinishedWork();

    return true;
}

void Gpu::checkForFinishedWork() {
    for (int i = fActiveGpuWork.count()-1; fActiveGpuWork.count() && i >= 0; --i) {
        std::unique_ptr<GpuWorkSubmission>& workSubmission = fActiveGpuWork[i];
        if (workSubmission->isFinished()) {
            // TODO: call finished procs?
            fActiveGpuWork.removeShuffle(i);
            // TODO: recycle if possible
        }
    }
}

} // namespace skgpu
