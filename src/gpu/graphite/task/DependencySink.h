/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_task_DependencySink_DEFINED
#define skgpu_graphite_task_DependencySink_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu::graphite {

class Task;

/**
 * A DependencySink collects the tasks that must execute before the work it's recording, such as a
 * second reference to the DrawTask of a scratch device whose texture is read by that work. The
 * sink is what an Image is notified with when it's read (see Image_Base::notifyInUse()), so the
 * reader can be something other than a DrawContext (e.g. a batch of compute dispatches).
 *
 * A sink that receives a task this way is responsible for adding it to the task graph *before* the
 * tasks of its own work, inside a scope of the ScratchResourceManager that then consumes the
 * reads marked by the dependency (see ComputeTask and DrawTask for examples).
 */
class DependencySink {
public:
    // Adds `task` to the set of tasks that execute before any work recorded after this call.
    virtual void recordDependency(sk_sp<Task> task) = 0;

protected:
    virtual ~DependencySink() = default;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_task_DependencySink_DEFINED
