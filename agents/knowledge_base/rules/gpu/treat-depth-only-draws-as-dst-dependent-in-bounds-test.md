---
name: skia/gpu/treat-depth-only-draws-as-dst-dependent-in-bounds-test
description: When performing layer bounds testing for depth-only draws in Graphite draw batching, treat any bounding-box intersection with existing draws as an incompatible overlap
rubric: correctness
severity: should_fix
---

**Rule.** When performing layer overlap or bounds testing for depth-only draws
in draw-list batching, treat any bounding-box intersection with existing draws
as an incompatible overlap (such as by including BoundsFlags::kColor in testMask
or treating the draw as having destination dependencies). Do not allow
depth-only draws to share a layer with or reorder around overlapping shaded
draws.

**Why.** Depth-only draws modify or rely on depth/stencil state and must execute
strictly after any previously recorded shaded draw that they overlap. Allowing
depth-only draws to overlap with existing draws within the same batching layer
would reorder dependent operations and cause depth-testing and rendering
defects.

## Examples

Bad:

```cpp
// Skipping color bounds test or conditioning overlap on stencil state:
for (BindingWrapper* list : fBindings) {
    if ((disjointStencil == list->fContainsStencil) && list->fBounds.intersects(drawBounds)) {
        return BoundsTest::kIncompatibleOverlap;
    }
}
```

Good:

```cpp
// Treating any intersection with existing draws as an incompatible overlap:
for (BindingWrapper* list : fBindings) {
    if (list->fBounds.intersects(drawBounds)) {
        return BoundsTest::kIncompatibleOverlap;
    }
}

// In DrawListLayer::recordDraw, testing against color bounds for depth-only draws:
if (dependsOnDst || isDepthOnly) {
    testMask |= BoundsFlags::kColor;
}
```
