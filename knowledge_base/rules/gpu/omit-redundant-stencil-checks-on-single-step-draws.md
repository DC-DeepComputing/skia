---
name: skia/gpu/omit-redundant-stencil-checks-on-single-step-draws
description: When filtering, batching, or merging single-renderstep draws (numRenderSteps() == 1) in Graphite, omit redundant !isStencil or stencil flag checks
rubric: readability
severity: should_fix
---

**Rule.** When filtering, batching, or merging draws in Graphite that are
already constrained to single render steps (`numRenderSteps() == 1`), do not add
redundant `!isStencil` checks or test for `DepthStencilFlags::kStencil`. All
stencil-using `Renderer`s in Graphite inherently require multiple render steps.

**Why.** In Graphite's rendering architecture, stencil operations (such as
stencil-and-cover or stencil tessellation) always execute across multiple render
steps. Checking for stencil exclusion on draws already restricted to single
render steps is dead code that misleadingly implies single-step stencil
renderers exist.

## Examples

Bad:

```cpp
// Redundant check: all stencil-using renderers in Graphite require multiple render steps.
if (renderer->numRenderSteps() == 1) {
    if (!isStencil && !target->fBounds.intersects(drawBounds)) {
        forwardMerge = target;
    }
}
```

Good:

```cpp
// Invariant: single-step renderers inherently exclude stencil draws.
if (renderer->numRenderSteps() == 1) {
    if (!target->fBounds.intersects(drawBounds)) {
        forwardMerge = target;
    }
}
```
