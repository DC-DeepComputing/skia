/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_task_DrawTask_DEFINED
#define skgpu_graphite_task_DrawTask_DEFINED

#include "src/gpu/graphite/task/Task.h"
#include "src/gpu/graphite/task/TaskList.h"

namespace skgpu::graphite {

class TextureProxy;

/**
 * DrawTask is a collection of subtasks that are executed in order to produce some intended
 * image in the DrawTask's target. As such, at least one of its subtasks will either be a
 * RenderPassTask, ComputeTask or CopyXToTextureTask that directly modify the target.
*/
class DrawTask final : public Task {
public:
    explicit DrawTask(sk_sp<TextureProxy> target);
    ~DrawTask() override;

    bool prepareResources(ResourceProvider*, const RuntimeEffectDictionary*) override;

    bool addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

private:
    friend class DrawContext; // for "addTask"

    // DrawTask is modified directly by DrawContext for efficiency, but its task list will be
    // fixed once DrawContext snaps the task.
    void addTask(sk_sp<Task> task) { fChildTasks.add(std::move(task)); }
    bool hasTasks() const { return fChildTasks.hasTasks(); }

    sk_sp<TextureProxy> fTarget;
    TaskList fChildTasks;
};

// RefDrawTask serves as a temporary placeholder for a child DrawTask in a parent's task list.
// For now, all DrawTasks are added at most once to the root task list. Scratch device tasks are
// then linked into a graph using RefDrawTasks to determine the read/write intervals of the targets.
// Eventually, scratch devices will not add their DrawTasks to the root task list and will instead
// be appended directly to the parent's task list. The read/write interval tracking performed by
// this task will be merged into DrawTask's functionality.
class RefDrawTask final : public Task {
public:
    explicit RefDrawTask(sk_sp<DrawTask> task) : fRealTask(std::move(task)) {}

    bool prepareResources(ResourceProvider*, const RuntimeEffectDictionary*) override;

    bool addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

private:
    sk_sp<DrawTask> fRealTask;
}

} // namespace skgpu::graphite

#endif // skgpu_graphite_task_DrawTask_DEFINED
