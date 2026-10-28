---
name: skia/check-matrix-min-max-scale-for-negative
description: When computing scale factors or dimensions with SkMatrix::getMinScale() or getMaxScale(), check for negative return values before calculations or divisions
rubric: correctness
severity: should_fix
---

**Rule.** When using SkMatrix::getMinScale() or getMaxScale() to compute scale
factors, offsets, or stroke dimensions, check the return value for negative
values (< 0 or -1) before using it in calculations or divisions. Provide an
appropriate fallback value or early exit when the matrix contains perspective or
overflows.

**Why.** SkMatrix::getMinScale() and getMaxScale() return -1 if the matrix
contains perspective or if scale factor calculations overflow. Using the
returned scale directly without checking for negative values leads to inverted
offsets, division by negative numbers, or invalid stroke geometry.

## Examples

Bad:

```cpp
const float max_abs_offset = kMaxDevOffset / ctm.getMinScale();
```

Good:

```cpp
const float min_scale = ctm.getMinScale();
const float max_abs_offset = min_scale < 0 ? kMaxDevOffset : kMaxDevOffset / min_scale;
```
