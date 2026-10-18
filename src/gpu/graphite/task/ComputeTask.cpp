/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/task/ComputeTask.h"

#include "include/private/SkAssert.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ComputePipeline.h"  // IWYU pragma: keep
#include "src/gpu/graphite/Sampler.h"  // IWYU pragma: keep
#include "src/gpu/graphite/Texture.h"  // IWYU pragma: keep
#include "src/gpu/graphite/TextureProxy.h"  // IWYU pragma: keep
#include "src/gpu/graphite/compute/DispatchGroup.h"

#include <cstddef>
#include <utility>

namespace skgpu::graphite {

ComputeTask::OutputListener::OutputListener(sk_sp<TextureProxy> proxy) : fProxy(std::move(proxy)) {
    SkASSERT(fProxy);
}

ComputeTask::OutputListener::~OutputListener() = default;

bool ComputeTask::OutputListener::prepare(ResourceProvider* provider,
                                          ScratchResourceManager* scratchManager) {
    if (fProxy->isInstantiated()) {
        // The output was given a regular texture after it was recorded (e.g. when its image was
        // made non-budgeted), so it's never returned to the scratch pool.
        SkASSERT(!fIsScratch);
        return true;
    }
    // Outputs are created by the compute work that writes them, so they're never lazy.
    SkASSERT(!fProxy->isLazy());

    // Same policy as RenderPassTask::prepareResources(): an output without pending reads might be
    // read by a later Recording, so it must get a regular non-shareable texture that's never
    // returned to the scratch pool.
    fIsScratch = fScratchAllowed && scratchManager->pendingReadCount(fProxy.get()) > 0;
    return fIsScratch ? TextureProxy::InstantiateIfNotLazy(scratchManager, fProxy.get())
                      : TextureProxy::InstantiateIfNotLazy(provider, fProxy.get());
}

void ComputeTask::OutputListener::onUseCompleted(ScratchResourceManager* scratchManager) {
    // A consumer has finished reading the output. If that was the last pending read, the texture
    // can be reused by tasks that are prepared after the consumer.
    SkASSERT(fProxy->isInstantiated());
    SkASSERT(scratchManager->pendingReadCount(fProxy.get()) > 0);
    if (scratchManager->removePendingRead(fProxy.get()) && fIsScratch) {
        scratchManager->returnTexture(fProxy->refTexture());
    }
}

sk_sp<ComputeTask> ComputeTask::Make(DispatchGroupList dispatchGroups) {
    return Make(std::move(dispatchGroups), /*dependencies=*/{}, /*outputs=*/{});
}

sk_sp<ComputeTask> ComputeTask::Make(DispatchGroupList dispatchGroups,
                                     skia_private::TArray<sk_sp<Task>> dependencies,
                                     skia_private::TArray<sk_sp<TextureProxy>> outputs) {
    return sk_sp<ComputeTask>(new ComputeTask(std::move(dispatchGroups),
                                              std::move(dependencies),
                                              std::move(outputs)));
}

ComputeTask::ComputeTask(DispatchGroupList dispatchGroups,
                         skia_private::TArray<sk_sp<Task>> dependencies,
                         skia_private::TArray<sk_sp<TextureProxy>> outputs)
        : fDispatchGroups(std::move(dispatchGroups))
        , fChildTasks(fDispatchGroups.size())
        , fDependencies(std::move(dependencies)) {
    for (auto& group : fDispatchGroups) {
        fChildTasks.push_back(group->snapChildTask());
    }
    fOutputs.reserve_exact(outputs.size());
    for (sk_sp<TextureProxy>& output : outputs) {
        fOutputs.emplace_back(std::move(output));
    }
}

ComputeTask::~ComputeTask() = default;

void ComputeTask::markOutputInUse(ScratchResourceManager* scratchManager, int index) {
    SkASSERT(index >= 0 && index < fOutputs.size());
    // The output must have been instantiated by this task's prepareResources().
    SkASSERT(fOutputs[index].proxy()->isInstantiated());
    scratchManager->markResourceInUse(&fOutputs[index]);
}

void ComputeTask::disableScratchOutput(int index) {
    SkASSERT(index >= 0 && index < fOutputs.size());
    fOutputs[index].disableScratch();
}

Task::Status ComputeTask::prepareResources(ResourceProvider* provider,
                                           ScratchResourceManager* scratchManager,
                                           sk_sp<const RuntimeEffectDictionary> rtd) {
    // Like the child TaskList of a DrawTask, this scope ensures that only the reads marked by this
    // task's dependencies are consumed by its notifyResourcesConsumed() below.
    scratchManager->pushScope();
    auto fail = [scratchManager]() {
        scratchManager->popScope();
        return Status::kFail;
    };

    for (sk_sp<Task>& dependency : fDependencies) {
        if (dependency) {
            Status status = dependency->prepareResources(provider, scratchManager, rtd);
            if (status == Status::kFail) {
                return fail();
            } else if (status == Status::kDiscard) {
                dependency.reset();
            }
        }
    }

    // Instantiate the outputs while the sources are still unavailable to the scratch manager, so
    // that an output never aliases a texture that the dispatches read.
    for (OutputListener& output : fOutputs) {
        if (!output.prepare(provider, scratchManager)) {
            SKIA_LOG_W("Failed to instantiate ComputeTask output. Dropping ComputeTask!");
            return fail();
        }
    }

    for (auto& child : fChildTasks) {
        if (child) {
            Status status = child->prepareResources(provider, scratchManager, rtd);
            if (status == Status::kFail) {
                return fail();
            } else if (status == Status::kDiscard) {
                child.reset();
            }
        }
    }
    for (const auto& group : fDispatchGroups) {
        // Textures that were already instantiated (e.g. the outputs above) are skipped.
        if (!group->prepareResources(provider)) {
            return fail();
        }
    }

    // At the equivalent point in addCommands(), the dispatches will have read any scratch textures
    // whose pending reads were marked by the dependencies, so they can be returned for reuse.
    scratchManager->notifyResourcesConsumed();
    scratchManager->popScope();
    return Status::kSuccess;
}

Task::Status ComputeTask::addCommands(Context* ctx,
                                      CommandBuffer* commandBuffer,
                                      ReplayTargetData rtd) {
    // Dependencies that weren't discarded during prepareResources() are executed by this task.
    bool hasDependencies = false;
    for (sk_sp<Task>& dependency : fDependencies) {
        if (dependency) {
            Status status = dependency->addCommands(ctx, commandBuffer, rtd);
            if (status == Status::kFail) {
                return Status::kFail;
            } else if (status == Status::kDiscard) {
                dependency.reset();
            } else {
                hasDependencies = true;
            }
        }
    }

    if (fDispatchGroups.empty()) {
        return hasDependencies ? Status::kSuccess : Status::kDiscard;
    }
    SkASSERT(fDispatchGroups.size() == fChildTasks.size());
    const std::unique_ptr<DispatchGroup>* currentSpanPtr = &fDispatchGroups[0];
    size_t currentSpanSize = 0u;
    for (int i = 0; i < fDispatchGroups.size(); ++i) {
        // If the next DispatchGroup has a dependent task, then encode the accumulated span as a
        // compute pass now. CommandBuffer encodes each compute pass with a separate encoder, so
        // the dependent task can use a non-compute encoder if needed.
        Task* child = fChildTasks[i].get();
        if (child) {
            if (currentSpanSize > 0u) {
                if (!commandBuffer->addComputePass({currentSpanPtr, currentSpanSize})) {
                    return Status::kFail;
                }
                currentSpanPtr = &fDispatchGroups[i];
                currentSpanSize = 0u;
            }

            Status status = child->addCommands(ctx, commandBuffer, rtd);
            if (status == Status::kFail) {
                return Status::kFail;
            } else if (status == Status::kDiscard) {
                fChildTasks[i].reset();
            }
        }
        currentSpanSize++;
    }
    return (currentSpanSize == 0u ||
            commandBuffer->addComputePass({currentSpanPtr, currentSpanSize})) ? Status::kSuccess
                                                                              : Status::kFail;
}

bool ComputeTask::visitProxies(const std::function<bool(const TextureProxy*)>& visitor,
                               bool readsOnly) {
    for (const sk_sp<Task>& dependency : fDependencies) {
        if (dependency && !dependency->visitProxies(visitor, readsOnly)) {
            return false;
        }
    }
    for (const sk_sp<Task>& child : fChildTasks) {
        if (child && !child->visitProxies(visitor, readsOnly)) {
            return false;
        }
    }
    // DispatchGroup doesn't record which of its textures are only read, so every bound texture is
    // visited even if `readsOnly` is true. Reporting a written texture as read is conservative: at
    // worst it causes an extra flush in DrawContext::readsTexture()'s callers.
    for (const auto& group : fDispatchGroups) {
        for (const sk_sp<TextureProxy>& texture : group->textureProxies()) {
            if (!visitor(texture.get())) {
                return false;
            }
        }
    }
    return true;
}

}  // namespace skgpu::graphite
