---
name: skia/resolve-path-convexity-for-raw-views
description: When creating an SkPathRaw view for code that checks convexity via isKnownToBeConvex(), pass SkResolveConvexity::kYes instead of kNo
rubric: correctness
severity: should_fix
---

**Rule.** When creating an SkPathRaw view via SkPathPriv::Raw(...) or
SkPathData::raw(...), pass SkResolveConvexity::kYes if downstream processing
queries convexity (such as via raw.isKnownToBeConvex()). Do not pass
SkResolveConvexity::kNo when convexity state affects downstream algorithms or
optimization branches.

**Why.** SkResolveConvexity::kNo skips computing convexity for uncomputed paths,
leaving their convexity state as kUnknown. Any subsequent call to
raw.isKnownToBeConvex() evaluates to false for unresolved paths even when they
are convex, causing convexity-dependent optimizations or algorithms (such as
edge culling) to execute incorrect code paths.

## Examples

Bad:

```cpp
if (auto raw = SkPathPriv::Raw(path, SkResolveConvexity::kNo)) {
    // buildEdges branches on raw.isKnownToBeConvex()
    return buildEdges(*raw, shiftedClip);
}
```

Good:

```cpp
if (auto raw = SkPathPriv::Raw(path, SkResolveConvexity::kYes)) {
    return buildEdges(*raw, shiftedClip);
}
```
