---
name: skia/opts/mark-simd-helpers-with-si
description: When defining helper functions in SIMD optimization files like SkSwizzler or SkRasterPipeline, mark them with SI instead of plain static
rubric: safety
severity: should_fix
---

**Rule.** In SIMD and raster pipeline optimization files (such as
src/opts/SkRasterPipeline_opts.h and src/opts/SkSwizzler_opts.inc), mark helper
functions called in tight per-pixel loops with the SI macro rather than plain
static or static inline. Guarantee that all fine-grained SIMD math, packing, and
conversion helpers are forced inline across all compiler optimization modes.

**Why.** Under size-optimization compiler flags such as -Oz, compilers may
decline to inline small helper functions unless explicitly forced. In hot SIMD
routines, inlining is essential to avoid call overhead and register spilling in
inner pixel loops.

## Examples

Bad:

```cpp
static __m256i scale(__m256i x, __m256i y) {
    const __m256i _128 = __lasx_xvreplgr2vr_h(128);
    const __m256i _257 = __lasx_xvreplgr2vr_h(257);
    return __lasx_xvmuh_hu(__lasx_xvadd_h(__lasx_xvmul_h(x, y), _128), _257);
}
```

Good:

```cpp
SI __m256i scale(__m256i x, __m256i y) {
    const __m256i _128 = __lasx_xvreplgr2vr_h(128);
    const __m256i _257 = __lasx_xvreplgr2vr_h(257);
    return __lasx_xvmuh_hu(__lasx_xvadd_h(__lasx_xvmul_h(x, y), _128), _257);
}
```
