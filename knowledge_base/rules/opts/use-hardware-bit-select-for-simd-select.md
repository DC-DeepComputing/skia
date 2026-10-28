---
name: skia/opts/use-hardware-bit-select-for-simd-select
description: When implementing SIMD if_then_else or vector conditional selection, use dedicated hardware bit-select or blend intrinsics instead of synthesized logical operations
rubric: safety
severity: should_fix
---

**Rule.** When implementing vector if_then_else or bitwise conditional selection
in SIMD routines, use dedicated architecture bit-select or blend intrinsics
(such as __lasx_xvbitsel_v/__lsx_vbitsel_v on LoongArch, vbslq_* on ARM NEON, or
_mm256_blendv_* on x86) rather than synthesizing the selection through multiple
bitwise operations ((c & t) | (~c & e) or AND/ANDN/OR). Consult SkVx.h for
established reference intrinsic mappings across architectures.

**Why.** Synthesizing vector conditionals via boolean operations requires three
separate vector instructions (AND, AND-NOT, OR) and temporary registers, whereas
modern SIMD architectures provide single hardware bit-select or blend
instructions that execute in fewer cycles with lower register pressure.

## Examples

Bad:

```cpp
SI F if_then_else(I32 c, F t, F e) {
    return (F)__lasx_xvor_v(__lasx_xvand_v(c, (I32)t), __lasx_xvandn_v(c, (I32)e));
}
```

Good:

```cpp
SI F if_then_else(I32 c, F t, F e) {
    return sk_bit_cast<Vec<8,float>>(__lasx_xvbitsel_v(sk_bit_cast<__m256i>(e),
                                                       sk_bit_cast<__m256i>(t),
                                                       sk_bit_cast<__m256i>(c)));
}
```
