/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_task_TaskList_DEFINED
#define skgpu_graphite_task_TaskList_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/task/Task.h"

#include <functional>
#include <utility>

namespace skgpu::graphite {

class CommandBuffer;
class Context;
class ResourceProvider;
class RuntimeEffectDictionary;
class ScratchResourceManager;

class TaskList {
public:
    TaskList() = default;

    void add(TaskList&& tasks) { fTasks.move_back(tasks.fTasks); }
    void add(sk_sp<Task> task) { fTasks.emplace_back(std::move(task)); }
    void reset() { fTasks.clear(); }

    int size() const { return fTasks.size(); }
    bool hasTasks() const { return !fTasks.empty(); }

    // Returns kSuccess if no child task failed and at least one child didn't return kDiscard.
    // Returns kDiscard if all children were discarded.
    // Returns kFail if any child failed.
    // Automatically removes tasks from its list if they return kDiscard.
    Task::Status prepareResources(ResourceProvider*,
                                  ScratchResourceManager*,
                                  const RuntimeEffectDictionary*);
    Task::Status addCommands(Context*, CommandBuffer*, Task::ReplayTargetData);

    template<typename T>
    bool visit(const std::function<bool(const T*)>& visitor) {
        Task::Status status = this->visitTasks([&](Task* task) {
            return task->visitChildren<T>(visitor) ? Task::Status::kSuccess : Task::Status::kFail;
        });
        return status != Task::Status::kFail;
    }

private:
    // Needs to be in the header due to templating on visit
    template <typename Fn>
    Task::Status visitTasks(Fn fn) {
        int discardCount = 0;
        for (sk_sp<Task>& task: fTasks) {
            if (!task) {
                discardCount++;
                continue; // Skip over discarded tasks
            }

            Task::Status status = fn(task.get());
            if (status == Task::Status::kFail) {
                return Task::Status::kFail;
            } else if (status == Task::Status::kDiscard) {
                task.reset();
                discardCount++;
            }
        }

        return discardCount == fTasks.size() ? Task::Status::kDiscard : Task::Status::kSuccess;
    }

    skia_private::TArray<sk_sp<Task>> fTasks;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_task_TaskList_DEFINED
