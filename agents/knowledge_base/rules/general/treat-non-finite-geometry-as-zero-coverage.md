---
name: skia/treat-non-finite-geometry-as-zero-coverage
description: When handling non-finite geometry or transforms in drawing and clipping pipelines, treat them as producing zero coverage rather than undefined behavior
rubric: correctness
severity: should_fix
---

**Rule.** Treat non-finite geometry (containing NaN or Infinity) and non-finite
transformation matrices as producing zero coverage rather than dismissing them
as undefined behavior. Drawing operations with non-finite geometry or
non-invertible/non-finite transforms must early-exit or no-op without rendering,
and clipping operations must behave as clipping against an empty path
(collapsing the clip on kIntersect or leaving the clip unaffected on
kDifference, inverted if the path is inverse-filled).

**Why.** Skia establishes a well-defined contract across CPU and GPU backends
where non-finite coordinates produce zero coverage rather than arbitrary
rendering artifacts, crashes, or unconstrained undefined behavior. Adhering to
this contract ensures consistent behavior across rendering pipelines and keeps
clipping invariants and unit tests predictable.

## Examples

Bad:

```cpp
// Bad: Dismissing non-finite geometry as undefined behavior
void MyDevice::onDrawPath(const SkPath& path, const SkPaint& paint) {
    // Drawing without checking finiteness, assuming callers never pass NaN
    this->drawShape(path, paint);
}
```

Good:

```cpp
// Good: Treat non-finite geometry as zero coverage (draws no-op, clips act as empty)
void MyDevice::onDrawPath(const SkPath& path, const SkPaint& paint) {
    if (!path.isFinite()) {
        return; // Non-finite geometry produces zero coverage; safe no-op
    }
    this->drawShape(path, paint);
}
```
