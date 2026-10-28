---
name: skia/core/transform-mutable-builder-in-place
description: When implementing mutating transformations on mutable geometry builders like SkPathBuilder, transform underlying data buffers in place rather than round-tripping through snapshot()
rubric: safety
severity: should_fix
---

**Rule.** When implementing mutating transformation or adjustment methods (such
as transform or offset) on mutable builder classes like SkPathBuilder, modify
the builder's internal point and verb buffers directly in place. Do not
materialize an immutable object via snapshot() and reassign the result to *this.

**Why.** Invoking snapshot() allocates and constructs an immutable intermediate
object (such as SkPath and its underlying data structures), and reassigning to
*this deep-copies points and verbs back into the builder. In-place buffer
transformations (such as calling matrix.mapPoints(fPts) or offsetting points
directly) eliminate temporary allocations and redundant buffer copies.

## Examples

Bad:

```cpp
SkPathBuilder& SkPathBuilder::transform(const SkMatrix& matrix, SkApplyPerspectiveClip pc) {
    // Bad: snapshot() allocates an intermediate SkPath, and reassigning copies all points.
    *this = this->snapshot().transform(matrix, pc);
    return *this;
}
```

Good:

```cpp
SkPathBuilder& SkPathBuilder::transform(const SkMatrix& matrix, SkApplyPerspectiveClip pc) {
    // Good: Transform underlying point buffers directly in place without allocations.
    matrix.mapPoints(fPts);
    return *this;
}
```
