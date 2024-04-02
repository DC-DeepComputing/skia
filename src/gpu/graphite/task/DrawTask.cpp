/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/task/DrawTask.h"

#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

DrawTask::DrawTask(sk_sp<TextureProxy> target) : fTarget(std::move(target)) {}

DrawTask::~DrawTask() = default;

bool DrawTask::prepareResources(ResourceProvider* resourceProvider,
                                const RuntimeEffectDictionary* rteDict) {
    return fChildTasks.prepareResources(resourceProvider, rteDict);
}

bool DrawTask::addCommands(Context* ctx,
                           CommandBuffer* commandBuffer,
                           ReplayTargetData replayTarget) {
    return fChildTasks.addCommands(ctx, commandBuffer, replayTarget);
}

bool RefDrawTask::prepareResources(ResourceProvider* resourceProvider,
                                const RuntimeEffectDictionary* rteDict) {
    return fChildTasks.prepareResources(resourceProvider, rteDict);
}

bool RefDrawTask::addCommands(Context*, CommandBuffer*, ReplayTargetData) {
    // When referring to a DrawTask later in the task list, there are no operations to perform.
    // The actual DrawTask must have been added to the root task list first, and the read/write
    // interval was determined in prepareResources().
    SkASSERT(fRealTask.)
    return fChildTasks.addCommands(ctx, commandBuffer, replayTarget);
}

} // namespace skgpu::graphite
