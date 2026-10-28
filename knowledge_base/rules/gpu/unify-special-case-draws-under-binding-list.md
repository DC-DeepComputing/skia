---
name: skia/gpu/unify-special-case-draws-under-binding-list
description: When organizing draws in Graphite draw batching, represent specialized draws like depth-only steps uniformly under common binding lists rather than bespoke side-structures
rubric: design
severity: should_fix
---

**Rule.** In Graphite draw batching and layer-based recording (such as
DrawListLayer), do not introduce bespoke side-structures or separate container
types to track special-case draws (such as depth-only or stencil steps).
Represent all draws uniformly using the primary draw-grouping abstraction (such
as BindingList).

**Why.** Maintaining bespoke side-containers for specialized draws duplicates
container management, complicates layer traversal, and disrupts uniform arena
allocation lifecycles. Reusing the primary draw-grouping abstraction simplifies
sorting, batch traversal, and memory management across all draw types.

## Examples

Bad:

```cpp
struct DepthDrawInfo {
    Rect fBounds = Rect::InfiniteInverted();
    skia_private::TArray<DepthDraw*>* fDraws = nullptr;
    bool fStencil = false;
};

struct Layer {
    DepthDrawInfo* fDepthInfo = nullptr;
    SkTInternalLList<BindingList> fBindings;
};
```

Good:

```cpp
// Represent depth-only draws uniformly using the common BindingList abstraction
struct Layer {
    SkTInternalLList<BindingList> fBindings;
};
```
