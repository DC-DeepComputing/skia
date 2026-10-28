---
name: skia/gpu/consistent-coordinate-snapping-terminology
description: When documenting bounds or coordinate adjustments in snap helpers or call sites, describe operations as snapping rather than rounding, aligning, or quantizing
rubric: readability
severity: nit
---

**Rule.** Keep terminology in code comments consistent with surrounding helper
names and subsystem conventions. When documenting bounds, rects, or coordinate
adjustments in snap helpers or call sites (such as invoking snap_* helpers or
storing snapped bounds), describe the operation as 'snapping' rather than mixing
terms like 'aligning', 'rounding', or 'quantizing'.

**Why.** In graphics pipelines, coordinate alignment, quantizing, rounding, and
snapping carry distinct mathematical and algorithmic meanings (e.g., rounding
typically converts floating-point coordinates to integer pixel boundaries like
roundOut(), whereas snapping adjusts to a grid). Inconsistent terminology in
comments creates ambiguity for reviewers and maintainers about whether a
separate geometric operation or custom quantization grid is being introduced.

## Examples

Bad:

```cpp
// 1. In snap helper definitions:
// Aligning or quantizing to 4-pixel boundaries reduces scissor churn:
Rect snap_scissor(const Rect& a, const Rect& deviceBounds) {
    float l = std::floor(a.fLeft / 4.f) * 4.f;
    ...
}

// 2. At call sites:
// Always record rounded draw bounds to avoid scissor thrashing
Rect snappedDrawBounds = snap_scissor(drawBounds, deviceBounds);
```

Good:

```cpp
// 1. In snap helper definitions:
// Snapping to 4-pixel boundaries reduces scissor churn:
Rect snap_scissor(const Rect& a, const Rect& deviceBounds) {
    float l = std::floor(a.fLeft / 4.f) * 4.f;
    ...
}

// 2. At call sites:
// Always record snapped draw bounds to avoid scissor thrashing
Rect snappedDrawBounds = snap_scissor(drawBounds, deviceBounds);
```
