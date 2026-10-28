---
name: skia/reciprocal-sync-comments-for-mirrored-code
description: When maintaining mirrored code or constants across decoupled files or language boundaries, include reciprocal sync comments explaining invariants and explicitly qualify counterpart identifiers
rubric: readability
severity: nit
---

**Rule.** When duplicate or mirrored implementations, algorithms, or constants
across separate files, modules, or language boundaries (such as SkSL and host
C++) must be kept in sync, add reciprocal synchronization comments in both
locations explaining why they must match rather than relying on a one-sided
comment. Explicitly qualify counterpart identifiers (e.g.,
ClassName::kConstantName) or specify source files rather than using ambiguous or
unqualified references (e.g., 'C++'s kConstantName').

**Why.** A one-sided or unexplained 'KEEP IN SYNC' comment leaves the primary
implementation unprotected and fails to alert maintainers why code must match.
Furthermore, unqualified or informal references across files or languages become
ambiguous as code is refactored, preventing maintainers from locating and
keeping coupled values synchronized.

## Examples

Bad:

```cpp
// In modules/skcms/src/Transform_inl.h:
// KEEP IN SYNC with skvx::from_half
SI F F_from_Half(U16 half) {
    ...
}

// In src/core/SkVx.h:
// (No reciprocal synchronization comment pointing back to skcms)
SIN Vec<N,float> from_half(const Vec<N,uint16_t>& x) {
    ...
}

// In src/sksl/sksl_graphite_vert.sksl:
const int kCornerVertexCount = 9; // KEEP IN SYNC WITH C++'s kCornerVertexCount
```

Good:

```cpp
// In modules/skcms/src/Transform_inl.h:
// KEEP IN SYNC with skvx::from_half to ensure that f16 colors are computed consistently in both
// skcms and skvx.
SI F F_from_Half(U16 half) {
    ...
}

// In src/core/SkVx.h:
// KEEP IN SYNC with skcms' F_from_Half to ensure that f16 colors are computed consistently in both
// skcms and skvx.
SIN Vec<N,float> from_half(const Vec<N,uint16_t>& x) {
    ...
}

// In src/sksl/sksl_graphite_vert.sksl:
const int kCornerVertexCount = 9; // KEEP IN SYNC WITH AnalyticRRectRenderStep::kCornerVertexCount
```
