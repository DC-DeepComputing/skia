---
name: skia/api/document-task-queue-priority-conventions
description: When declaring multi-queue or prioritized task execution APIs accepting integer priority levels or queue indices, document priority ordering and scheduling semantics in header comments
rubric: readability
severity: should_fix
---

**Rule.** When declaring multi-queue, multi-worklist, or prioritized task
execution APIs that accept integer priority levels, queue counts, or queue
indices (such as numWorkLists or workList), document the priority ordering
conventions and scheduling semantics in the header comments. Explicitly clarify
whether index 0 represents the highest or lowest priority, how tasks are
prioritized across queues, and whether lower-priority or active tasks are
subject to preemption.

**Why.** Integer priority indices and queue IDs are inherently ambiguous across
libraries and frameworks (some treat 0 as the highest priority, while others
treat higher integers as higher priority). Documenting the priority indexing
convention and dispatch semantics directly on public interfaces prevents caller
misconfiguration and ensures tasks are scheduled with the intended urgency.

## Examples

Bad:

```cpp
// Missing documentation explaining queue priority ordering and dispatch semantics
static std::unique_ptr<SkExecutor> MakeMultiListFIFOThreadPool(int numWorkLists,
                                                               int threads = 0,
                                                               bool allowBorrowing = true);
static std::unique_ptr<SkExecutor> MakeMultiListLIFOThreadPool(int numWorkLists,
                                                               int threads = 0,
                                                               bool allowBorrowing = true);
```

Good:

```cpp
// Having multiple work lists allows for prioritization with work being pulled
// from the lower indexed work lists first - with work list '0' being the highest priority.
static std::unique_ptr<SkExecutor> MakeMultiListFIFOThreadPool(int numWorkLists,
                                                               int threads = 0,
                                                               bool allowBorrowing = true);
static std::unique_ptr<SkExecutor> MakeMultiListLIFOThreadPool(int numWorkLists,
                                                               int threads = 0,
                                                               bool allowBorrowing = true);
```
