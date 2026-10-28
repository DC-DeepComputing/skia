---
name: skia/prefer-addraw-between-path-builders
description: When appending geometry from another SkPathBuilder or raw buffer spans into SkPathBuilder, use SkPathRaw and addRaw instead of allocating an intermediate SkPath
rubric: safety
severity: should_fix
---

**Rule.** When appending geometry from another SkPathBuilder or raw buffer spans
into an SkPathBuilder, wrap or query an SkPathRaw view (using
SkPathPriv::Raw(srcBuilder, ...) or constructing SkPathRaw from point, verb, and
weight spans) and call destBuilder.addRaw(raw, reserve) instead of materializing
an intermediate SkPath to pass to addPath(). Choose Reserve::kExact when
appending a known size or completed contour, or Reserve::kGrow when accumulating
repeatedly.

**Why.** Constructing an intermediate SkPath (via detach(), snapshot(), or Make
from raw buffers) allocates and initializes an owning heap-backed
SkPathRef/SkPathData object, which is wasted overhead when the geometry is
immediately consumed by another builder. SkPathRaw provides a lightweight,
non-owning view that SkPathBuilder::addRaw() appends directly without temporary
object allocations or ref-count churn.

## Examples

Bad:

```cpp
// Bad: Materializing a temporary SkPath from another builder or raw spans.
fBuilder.addPath(fCurrent.detach());

SkPath path = MakePathFromVerbsPointsWeights(verbsPtr, numVerbs, ptsPtr, numPts, wtsPtr, numWts);
builder.addPath(path);
```

Good:

```cpp
// Good: Query or wrap in SkPathRaw and append directly via addRaw().
if (auto raw = SkPathPriv::Raw(fCurrent, SkResolveConvexity::kNo)) {
    fBuilder.addRaw(*raw, SkPathBuilder::Reserve::kExact);
}

SkPathRaw raw{
    SkSpan<const SkPoint>(pts, SkToSizeT(numPts)),
    SkSpan<const SkPathVerb>(verbs, SkToSizeT(numVerbs)),
    SkSpan<const float>(weights, SkToSizeT(numWts)),
};
builder.addRaw(raw, SkPathBuilder::Reserve::kExact);
```
