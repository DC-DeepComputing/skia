---
name: skia/gpu/distinguish-draw-pass-from-draw-task
description: When naming variables, parameters, or fields tracking draw- or pass-level requirements in Graphite, name them after DrawPass rather than the aggregate DrawTask
rubric: readability
severity: nit
---

**Rule.** When naming variables, parameters, or member fields that track draw-
or pass-specific requirements (such as destination texture sampling or dst-read
strategy) in Graphite, name them after DrawPass rather than DrawTask. Reserve
DrawTask naming strictly for the aggregate container task that manages flushes,
uploads, compute tasks, and render passes.

**Why.** In Graphite, a DrawTask is an aggregate task container that sequences
multiple distinct operations, including texture uploads, compute tasks, and
RenderPassTasks. Conflating DrawTask with DrawPass in identifier names is
misleading because draw-specific execution requirements belong to the individual
DrawPass and its RenderPassTask, not the enclosing DrawTask.

## Examples

Bad:

```cpp
bool drawTaskUsesDstSampler = fPendingDraws->usesDstSampler();
DstReadStrategy drawTaskDstReadStrategy = fPendingDraws->drawsReadDst()
        ? DstReadStrategy::kTextureCopy : DstReadStrategy::kNone;
```

Good:

```cpp
bool drawPassUsesDstSampler = fPendingDraws->usesDstSampler();
DstReadStrategy drawPassDstReadStrategy = fPendingDraws->drawsReadDst()
        ? DstReadStrategy::kTextureCopy : DstReadStrategy::kNone;
```
