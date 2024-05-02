/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/task/DrawTask.h"

#include "src/gpu/graphite/ScratchResourceManager.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

DrawTask::DrawTask(sk_sp<TextureProxy> target) : fTarget(std::move(target)) {
    SkDebugf("creating task %p with proxy %p %d\n", this, fTarget.get(), !fTarget->isInstantiated() && !fTarget->isLazy());
}

DrawTask::~DrawTask() {
    SkDebugf("deleting %p\n", this);
}

Task::Status DrawTask::prepareResources(ResourceProvider* resourceProvider,
                                        ScratchResourceManager* scratchManager,
                                        const RuntimeEffectDictionary* rteDict) {
    const int pendingReadCount = scratchManager->pendingReadCount(fTarget.get());
    SkDebugf("prepare resources %p target= %p, read count =%d\n", this, fTarget.get(), pendingReadCount);
    if (pendingReadCount) {
        // This DrawTask defines the content of a scratch device that has incremented the pending
        // read count before snap() was called. The target may have already been instantiated if
        // we've processed this task's children before.
        SkASSERT(!fTarget->isLazy());
        // Even though we may discard the task, we always want to mark it as in-use to track the
        // pending reads to know when to return the texture.;
        SkDebugf("mark %p as read (listener = %p)\n", fTarget.get(), this);
        scratchManager->markResourceInUse(this);

    /**
     * FIXME: If we actually have multiple drawtasks mapping to the same proxy, we can get into the
     * situation where the later drawtask is only visited once
    */

        if (fTarget->isInstantiated()) {
            // Return kDiscard so that this reference to the task is removed and the original
            // encounter in the graph will be the only time addCommands() is invoked.
            // scratchManager->markResourceInUse(this);
            return Status::kDiscard;
        } else {
            SkDebugf("first time encountering %p (should be root task)\n", fTarget.get());
        }
    } else {
        SkDebugf("no pending reads for %p (%d %d)\n", fTarget.get(), fTarget->isInstantiated(), fTarget->isFullyLazy());
        if (!fTarget->isInstantiated() && !fTarget->isFullyLazy()) {
            SkDebugf("possibly wasted work\n"); // can't be discarded if this was a root flush that needs to be done for valid follow-up Recording
        }
        // SkASSERT(fTarget->isInstantiated() || fTarget->isFullyLazy());
    }

    // NOTE: This prepareResources() pushes a new scope for scratch resource management, which is
    // what we want since the child tasks are what will actually instantiate any scratch device and
    // trigger returns of any grand-child resources. The above markResourceInUse() should happen
    // above this so that pending returns are handled in caller's scope.
    SkDebugf("visiting children for proxy: %p\n", fTarget.get());
    Status s = fChildTasks.prepareResources(resourceProvider, scratchManager, rteDict);
    SkDebugf("done visiting tasks for %p\n", fTarget.get());
    return s;
}

void DrawTask::onUseCompleted(ScratchResourceManager* scratchManager) {
    // Now that the render task has completed, actually decrement the read count of the target proxy
    // If the count hits zero, this was the last pending read that needed to use the DrawTask's
    // results so we can return the texture to the ScratchResourceManager for reuse.
    SkASSERT(!fTarget->isLazy() && fTarget->isInstantiated());
    SkASSERT(scratchManager->pendingReadCount(fTarget.get()) > 0);
    if (scratchManager->removePendingRead(fTarget.get())) {
        SkDebugf("returning texture associated with %p\n", fTarget.get());
        scratchManager->returnResource(fTarget->refTexture());
    }
}

Task::Status DrawTask::addCommands(Context* ctx,
                                   CommandBuffer* commandBuffer,
                                   ReplayTargetData replayTarget) {
    SkASSERT(fTarget->isInstantiated());
    return fChildTasks.addCommands(ctx, commandBuffer, replayTarget);
}

} // namespace skgpu::graphite
