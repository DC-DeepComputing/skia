---
name: skia/gpu/document-non-hierarchy-task-classes
description: When declaring classes with a Task suffix in Graphite that do not derive from skgpu::graphite::Task, document that they represent threaded work rather than TaskList tasks
rubric: readability
severity: should_fix
---

**Rule.** When declaring a class in Graphite that uses the *Task suffix but does
not derive from skgpu::graphite::Task (or participate in TaskLists), document in
its header comment that it is not part of the Task DAG hierarchy. Explain that
it represents an independent unit of threaded or asynchronous work managed by a
subsystem coordinator rather than a command-recording task.

**Why.** In Graphite, Task is the fundamental DAG abstraction for sequencing GPU
resource preparation and command recording into TaskLists. Reusing the *Task
suffix for unrelated asynchronous worker objects without clarification confuses
maintainers who expect task-graph scheduling, visitor traversal, and
command-buffer execution semantics.

## Examples

Bad:

```cpp
// Once completed, the PipelineTask will lock the created Pipeline in the cache (via 'fPipeline')
// until the PipelineTask is deleted.
class PipelineCreationTask : public SkRefCnt {
```

Good:

```cpp
// Once completed, the PipelineTask will lock the created Pipeline in the cache (via 'fPipeline')
// until the PipelineTask is deleted.
// Note that this is not a task in the sense of the Task class. It is a task in the sense that
// it is a unit of work possibly delegated to a thread. PipelineCreationTasks are also only
// known and handled by the PipelineManager vs being added to TaskLists (as Task-derived classes
// are).
class PipelineCreationTask : public SkRefCnt {
```
