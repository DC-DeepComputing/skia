---
name: skia/gpu/bound-path-complexity-tightly-against-oom
description: When defining path complexity limits to prevent fuzzer OOMs in triangulators or tessellators, enforce tight upper bounds rather than overly generous thresholds
rubric: safety
severity: should_fix
---

**Rule.** When adding complexity limits on vector path geometry (such as contour
counts in GrTriangulator or path tessellators) to prevent fuzzer-induced
out-of-memory crashes, choose tight upper bounds (e.g., 1024 / 1 << 10) rather
than generous limits (such as 1 << 13). Because legitimate vector paths rarely
contain thousands of contours, keep limits conservative to guard against
pathological intermediate allocations and quadratic sweep-line operations.

**Why.** Path triangulation and tessellation algorithms construct intermediate
vertex lists, polygon meshes, and edge structures whose memory allocations and
sweep operations scale poorly with contour counts. Generous thresholds (such as
8192 contours) leave ample headroom for fuzzer-crafted inputs to exhaust memory
before tripping bounds checks, whereas tight bounds accommodate realistic paths
while safely rejecting pathological payloads.

## Examples

Bad:

```cpp
// Bad: Overly generous contour limit leaves room for fuzzer OOMs.
constexpr static int kMaxContourCount = 1 << 13;
if (contourCnt > kMaxContourCount) {
    return {};
}
```

Good:

```cpp
// Good: Tight bound accommodates valid paths while preventing memory exhaustion.
constexpr static int kMaxContourCount = 1 << 10;
if (contourCnt > kMaxContourCount) {
    return {};
}
```
