---
name: skia/default-optimization-flags-to-conservative-state
description: When declaring member variables controlling optimizations, layer elision, or fast paths, initialize them to conservative safe defaults rather than optimistic unvalidated states
rubric: correctness
severity: should_fix
---

**Rule.** Initialize member variables and flags that govern optimizations, layer
elisions, caching, or fast paths (such as fRequiresIsolation or dirty markers)
to their conservative, safe default state rather than assuming the optimized or
bypassed state. Defer enabling the optimization until revalidation or layout
passes explicitly verify that preconditions are met.

**Why.** Defaulting optimization or elision flags optimistically risks bypassing
necessary isolation, synchronization, clipping, or drawing passes if an object
is evaluated, rendered, or traversed before validation completes. A conservative
default guarantees correct fallback rendering and behavior even under partial or
unvalidated states.

## Examples

Bad:

```cpp
class Group : public RenderNode {
    ...
    // Optimistic default: assumes isolation is unnecessary before children are validated
    bool fRequiresIsolation = false;
};
```

Good:

```cpp
class Group : public RenderNode {
    ...
    // Conservative default: assumes isolation is required until validation proves otherwise
    bool fRequiresIsolation = true;
};
```
