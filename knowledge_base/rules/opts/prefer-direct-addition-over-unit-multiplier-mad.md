---
name: skia/opts/prefer-direct-addition-over-unit-multiplier-mad
description: When computing additions in SIMD stages or vector routines, use direct addition (v + t) instead of invoking multiply-add operations with a unit multiplier
rubric: readability
severity: should_fix
---

**Rule.** When adding terms in SIMD stages or vector math helpers, use direct
vector addition (v + t) instead of invoking multiply-add helpers (such as mad(v,
1, t) or mad(1, v, t)) with a multiplier of one.

**Why.** Calling a multiply-accumulate function or intrinsic when multiplying by
one adds cognitive overhead and can emit unnecessary multiply-accumulate
operations when direct vector addition communicates intent clearly and
efficiently.

## Examples

Bad:

```cpp
SI U32 iround(F v) {
    F t = F(0.5);
    return __lsx_vftintrz_w_s(mad(v, 1, t));
}
```

Good:

```cpp
SI U32 iround(F v) {
    F t = F(0.5);
    return __lsx_vftintrz_w_s(v + t);
}
```
