---
name: skia/gpu/traverse-backwards-for-dst-dependent-draws
description: When recording dependency-constrained or destination-reading draws in draw lists, traverse backwards stopping at the dependency boundary instead of creating separate forward-traversal methods
rubric: design
severity: should_fix
---

**Rule.** Do not introduce separate forward-traversal recording methods (such as
recordForwards or recordClipped) for draws that depend on prior operations or
layers (such as clip depth draws or destination-reading draws). Enforce
dependency ordering by traversing backwards from the list tail and stopping at a
designated boundary (stopLayer or stop order), ensuring destination-reading
draws (DstUsage::kDependsOnDst) and clipped draws reuse the standard batching
path while preserving painter's order against intermediate overlaps.

**Why.** Maintaining separate forward-recording paths duplicates batching logic
and complicates layer sorting and barrier handling. Furthermore, forward
traversal from an earlier layer cannot guarantee that subsequent layers do not
contain conflicting overlaps without an exhaustive search to the tail; backwards
traversal naturally stops at intermediate overlaps for destination reads while
preventing dependent draws from preceding their stop boundary.

## Examples

Bad:

```cpp
// Separate forward-recording path bypasses standard backwards batching
// and misses conflicting overlaps for destination-reading draws:
void DrawListLayer::recordDraw(...) {
    if (captureLayer && !dependsOnDst) {
        this->recordForwards(stepIndex, isStencil, captureLayer);
    } else {
        this->recordBackwards(stepIndex, isStencil, stopOrder);
    }
}
```

Good:

```cpp
// Backwards traversal naturally enforces ordering by stopping at the boundary:
void DrawListLayer::recordDraw(...) {
    CompressedPaintersOrder stop = captureLayer ? captureLayer->fOrder
                                                : DrawOrder::kNoIntersection;
    this->searchBackwards(step, key, testMask, drawParams, stop);
}
```
