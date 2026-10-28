---
name: skia/opts/match-intrinsic-headers-to-cpu-feature-levels
description: When conditionally including architecture-specific intrinsic headers under CPU feature-level guards, match each header to the exact instruction set level being guarded
rubric: correctness
severity: blocking
---

**Rule.** When including architecture-specific intrinsic headers within CPU
feature-level preprocessor blocks (such as SK_CPU_LSX_LEVEL or
SK_CPU_X64_LEVEL), verify that each header precisely corresponds to the
instruction set extension guarded by that conditional block (e.g.,
<lasxintrin.h> under SK_CPU_LSX_LEVEL_LASX and <lsxintrin.h> under
SK_CPU_LSX_LEVEL_LSX). Do not invert or cross-include lower-tier and higher-tier
intrinsic headers across differing CPU level branches.

**Why.** Inverted or mismatched intrinsic headers cause compilation failures on
systems that only support the baseline instruction set extension (where
higher-tier headers may be absent or rejected by compiler flags) and leave
higher-tier SIMD functions with undefined intrinsic symbols.

## Examples

Bad:

```cpp
#if SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
    #include <lsxintrin.h>

    static inline __m256i SkPMSrcOver_LASX(...) { ... }
#endif

#if SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
    #include <lasxintrin.h>

    static inline __m128i SkPMSrcOver_LSX(...) { ... }
#endif
```

Good:

```cpp
#if SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
    #include <lasxintrin.h>

    static inline __m256i SkPMSrcOver_LASX(...) { ... }
#endif

#if SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
    #include <lsxintrin.h>

    static inline __m128i SkPMSrcOver_LSX(...) { ... }
#endif
```
