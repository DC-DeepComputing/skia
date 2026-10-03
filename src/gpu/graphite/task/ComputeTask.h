/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_task_ComputeTask_DEFINED
#define skgpu_graphite_task_ComputeTask_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkTArray.h"
#include "src/gpu/graphite/ScratchResourceManager.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/task/Task.h"

#include <functional>
#include <memory>

namespace skgpu::graphite {

class CommandBuffer;
class Context;
class ResourceProvider;
class RuntimeEffectDictionary;
class TextureProxy;

/**
 * ComputeTask handles preparing and recording DispatchGroups into a series of compute dispatches
 * within a command buffer. It is guaranteed that dispatches within a DispatchGroup will be executed
 * sequentially.
 *
 * A ComputeTask takes part in scratch resource management (see ScratchResourceManager) the same way
 * a DrawTask and its RenderPassTask do:
 *  - `dependencies` are prepared in the ComputeTask's own scope, and executed before any dispatch
 *    if they weren't discarded. A dependency that's a second reference to a scratch DrawTask (e.g.
 *    added by Device::notifyInUse()) marks its pending read in that scope, so its texture can be
 *    reused by tasks that are prepared after this one.
 *  - `outputs` are textures written by the dispatches. An output with pending reads is instantiated
 *    as a scratch texture, which is returned once every pending read has been consumed. Each
 *    consumer marks its read with markOutputInUse().
 */
class ComputeTask final : public Task {
public:
    using DispatchGroupList = skia_private::STArray<1, std::unique_ptr<DispatchGroup>>;

    static sk_sp<ComputeTask> Make(DispatchGroupList dispatchGroups);
    static sk_sp<ComputeTask> Make(DispatchGroupList dispatchGroups,
                                   skia_private::TArray<sk_sp<Task>> dependencies,
                                   skia_private::TArray<sk_sp<TextureProxy>> outputs);

    ~ComputeTask() override;

    Status prepareResources(ResourceProvider*,
                            ScratchResourceManager*,
                            sk_sp<const RuntimeEffectDictionary>) override;
    Status addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

    bool visitProxies(const std::function<bool(const TextureProxy*)>& visitor,
                      bool readsOnly) override;

    int outputCount() const { return fOutputs.size(); }
    const TextureProxy* output(int index) const { return fOutputs[index].proxy(); }

    // Marks one pending read of `output(index)` in the scratch manager's current scope. The read is
    // consumed by the next notifyResourcesConsumed() in that scope (e.g. by the RenderPassTask that
    // samples the output). It must be called once for each pending read that was added for the
    // output, from a task that's prepared after this ComputeTask.
    void markOutputInUse(ScratchResourceManager*, int index);

    // Stops `output(index)` from being instantiated as a scratch texture, so it's never returned to
    // the scratch pool. This is required when the output is read by a task whose read can't be
    // marked, such as a copy on the root task list. Must be called before prepareResources().
    void disableScratchOutput(int index);

    // TODO: Traverse child tasks too!
    SK_DUMP_TASKS_CODE(const char* getTaskName() const override { return "Compute Task"; })

private:
    // Tracks the pending reads of one output, mirroring DrawTask::onUseCompleted(). The listeners
    // are owned by the ComputeTask, which stays in the task graph while consumers prepare, so a
    // consumer can mark the read with a short-lived task that's discarded right after.
    class OutputListener final : public ScratchResourceManager::PendingUseListener {
    public:
        explicit OutputListener(sk_sp<TextureProxy> proxy);
        ~OutputListener() override;

        OutputListener(OutputListener&&) = default;

        TextureProxy* proxy() const { return fProxy.get(); }

        void disableScratch() { fScratchAllowed = false; }

        // Instantiates the output, unless it's already instantiated.
        bool prepare(ResourceProvider*, ScratchResourceManager*);

        void onUseCompleted(ScratchResourceManager*) override;

    private:
        sk_sp<TextureProxy> fProxy;
        bool fScratchAllowed = true;
        // Whether the output was instantiated as a scratch texture by prepare().
        bool fIsScratch = false;
    };

    ComputeTask(DispatchGroupList dispatchGroups,
                skia_private::TArray<sk_sp<Task>> dependencies,
                skia_private::TArray<sk_sp<TextureProxy>> outputs);

    DispatchGroupList fDispatchGroups;

    // Every element of this list is a task that must execute before the DispatchGroup stored at the
    // same array index. Child tasks are allowed to be a nullptr to represent NOP (i.e. the
    // corresponding DispatchGroup doesn't have any pre-tasks).
    skia_private::TArray<sk_sp<Task>> fChildTasks;

    // Tasks that must be prepared and executed before any of the dispatch groups. These aren't held
    // in a TaskList because that pushes its own scope, which would drop the pending reads marked by
    // the dependencies before this task's dispatches could consume them.
    skia_private::TArray<sk_sp<Task>> fDependencies;

    // Fixed at construction, since the scratch manager holds pointers to the listeners.
    skia_private::TArray<OutputListener> fOutputs;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_task_ComputeTask_DEFINED
