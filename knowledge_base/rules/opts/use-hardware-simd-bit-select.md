---
name: skia/opts/use-hardware-simd-bit-select
description: When implementing vector conditional selection in SIMD routines, use dedicated hardware bit-select or blend intrinsics instead of multi-instruction bitwise logic
rubric: safety
severity: should_fix
---

**Rule.** When implementing vector conditional selection (such as
`if_then_else`) in SIMD stages or vector routines, use the target architecture's
dedicated hardware bit-select or blend intrinsics (such as
`__lsx_vbitsel_v`/`__lasx_xvbitsel_v` on LoongArch, `vbslq_*` on ARM NEON, or
`_mm_blendv_*`/`_mm256_blendv_*` on x86) instead of synthesizing selection with
multiple bitwise logic operations (AND, AND-NOT, OR). Consult canonical vector
primitives in `src/core/SkVx.h` for existing, verified intrinsic patterns across
supported architectures.

**Why.** Synthesizing conditional selection via separate bitwise instructions
requires three instructions and extra register pressure, whereas modern vector
ISAs provide single-instruction bit-select or blend hardware operations. Reusing
established vector intrinsics from SkVx.h ensures optimal instruction selection
and avoids duplicate, sub-optimal logic in hot raster pipeline stages.

## Examples

Bad:

```cpp
SI F if_then_else(I32 c, F t, F e) {
    return (F)__lsx_vor_v(__lsx_vand_v(c, (I32)t), __lsx_vandn_v(c, (I32)e));
}
```

Good:

```cpp
SI F if_then_else(I32 c, F t, F e) {
    return sk_bit_cast<Vec<4, float>>(__lsx_vbitsel_v(sk_bit_cast<__m128i>(e),
                                                      sk_bit_cast<__m128i>(t),
                                                      sk_bit_cast<__m128i>(c)));
}
```
