---
name: skia/opts/omit-redundant-architecture-preprocessor-branches
description: When implementing SIMD stages or helpers, omit redundant architecture preprocessor branches or target-specific intrinsics when generic fallbacks or portable vector primitives suffice
rubric: design
severity: should_fix
---

**Rule.** When defining architecture-specialized functions or SIMD stages, do
not keep redundant architecture preprocessor branches (#if / #elif defined(...))
that duplicate generic #else fallbacks, and do not introduce target-specific
intrinsics when portable vector primitives (such as cast<T>) already provide the
required behavior across targets. Fold architectures relying on generic code or
compiler auto-vectorization into the #else path, and avoid leaving experimental
or temporary intrinsic overrides in place unless documented with an explanatory
TODO.

**Why.** Explicit preprocessor branches for specific CPU architectures imply
specialized intrinsic implementations or tuning. Duplicating generic fallbacks
creates code clutter and confuses readers, while introducing target-specific
SIMD intrinsics where portable vector primitives already suffice adds
unnecessary maintenance burden and risks committing temporary benchmarking
experiments.

## Examples

Bad:

```cpp
// Variant 1: Redundant architecture branch duplicating generic fallback
#if defined(SKRP_CPU_ML4)
SI I32 max(I32 x, I32 y) { return if_then_else(x < y, y, x); }
#else
SI I32 max(I32 x, I32 y) { return if_then_else(x < y, y, x); }
#endif

// Variant 2: Architecture-specific intrinsics where portable primitives suffice
SI void store_8(uint8_t* ptr, U16 v) {
    v = min(v, 255);
#if defined(SKRP_CPU_LASX) || defined(JUMPER_IS_LASX)
    __m128i lo, hi;
    split(v, &lo, &hi);
    store(ptr, __lsx_vpickev_b(hi, lo));
#else
    store(ptr, cast<U8>(v));
#endif
}
```

Good:

```cpp
// Variant 1: Fold generic handling into #else, annotating needed investigations
#if defined(SKRP_CPU_AVX2)
SI I32 max(I32 x, I32 y) { return join<I32>(_mm256_max_epi32(x_lo, y_lo), ...); }
#elif defined(SKRP_CPU_NEON)
// TODO(author): make sure NEON code is handled well.
SI I32 max(I32 x, I32 y) { return if_then_else(x < y, y, x); }
#else
SI I32 max(I32 x, I32 y) { return if_then_else(x < y, y, x); }
#endif

// Variant 2: Use portable vector primitives directly across targets
SI void store_8(uint8_t* ptr, U16 v) {
    v = min(v, 255);
    store(ptr, cast<U8>(v));
}
```
