---
name: skia/document-scalar-fallback-in-simd-pipeline
description: When invoking scalar routines or falling back to scalar execution in a SIMD pipeline, document the empirical performance or workload rationale in an inline comment
rubric: readability
severity: nit
---

**Rule.** When invoking a scalar helper or falling back to scalar execution
within an ostensibly vectorized or SIMD pipeline, document the non-obvious
performance or workload rationale in an inline comment. Explain the empirical
benchmarking results or workload characteristics (such as typical input sizes or
low element counts making vector setup overhead inefficient) so maintainers
understand the scalar choice is deliberate and not an oversight.

**Why.** In performance-critical SIMD code, calling a scalar implementation when
a vectorized alternative exists or is expected looks like an accidental omission
or copy-paste bug. Explicit inline documentation prevents future maintainers
from refactoring the call to SIMD without benchmarking and records the empirical
tradeoffs for future optimizations.

## Examples

Bad:

```cpp
if (skvx::all(X < 0.0f) ||
    is_within_dist_sq(pts[1], pts[0], pts[2], kQuadSubdivThreshold)) {
    polyline->appendPoint(pts[2]);
} else {
    this->flattenQuadScalar(pts, polyline);
}
```

Good:

```cpp
if (skvx::all(X < 0.0f) ||
    is_within_dist_sq(pts[1], pts[0], pts[2], kQuadSubdivThreshold)) {
    polyline->appendPoint(pts[2]);
} else {
    // Note: testing shows scalar is slightly faster here, likely because
    // typical quads don't produce enough segments to offset SIMD overhead.
    this->flattenQuadScalar(pts, polyline);
}
```
