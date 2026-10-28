---
name: skia/opts/use-si-macro-for-internal-simd-helpers
description: When declaring internal helper functions in src/opts or skcms SIMD files, use the SI macro instead of bare static or inline
rubric: readability
severity: should_fix
---

**Rule.** In src/opts SIMD/vector files (such as SkSwizzler_opts.inc and
SkRasterPipeline_opts.h) and modules/skcms, declare internal helper functions
using the subsystem SI macro instead of bare static, inline, or static inline.
Do not use raw compiler attributes or leave helper functions unannotated.

**Why.** In SIMD and vector processing files, helper functions must be reliably
inlined (__attribute__((always_inline)) static inline) to eliminate
function-call and register-spilling overhead inside tight pixel-processing
loops. Using the subsystem SI macro standardizes always-inline static
declarations portably across compilers and avoids inconsistent linkage.

## Examples

Bad:

```cpp
static __m128i scale(__m128i x, __m128i y) {
    return _mm_mulhi_epu16(x, y);
}
```

Good:

```cpp
SI __m128i scale(__m128i x, __m128i y) {
    return _mm_mulhi_epu16(x, y);
}
```
