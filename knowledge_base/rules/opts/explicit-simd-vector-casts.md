---
name: skia/opts/explicit-simd-vector-casts
description: When wrapping architecture-specific SIMD intrinsics with generic vector types, explicitly cast vector arguments and returns to prevent cross-compiler build failures
rubric: correctness
severity: blocking
---

**Rule.** When implementing SIMD operations that wrap architecture-specific
intrinsics (such as SSE, AVX, NEON, or LSX/LASX), explicitly cast generic vector
arguments and return values (e.g. (F), (__m128)a, or sk_bit_cast<...>) rather
than relying on implicit conversions. Do not assume compilers permit implicit
conversion between generic vectors (__attribute__((vector_size(...)))) and
target-specific intrinsic vector types.

**Why.** Compilers such as GCC and strict Clang toolchains reject implicit
conversions between generic vector types and architecture-specific intrinsic
vector types, leading to build breaks on platform-specific targets. Explicit
casts ensure portable compilation across all supported compilers and CPU
architectures.

## Examples

Bad:

```cpp
SI F   min(F a, F b)        { return __lsx_vfmin_s(a, b);     }
SI I32 min(I32 a, I32 b)    { return __lsx_vmin_w(a, b);      }
```

Good:

```cpp
SI F   min(F a, F b)        { return (F)__lsx_vfmin_s((__m128)a, (__m128)b);     }
SI I32 min(I32 a, I32 b)    { return (I32)__lsx_vmin_w((__m128i)a, (__m128i)b);  }
```
