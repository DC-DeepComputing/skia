---
name: skia/opts/prefer-fmadd-negated-multiplicand-over-fnmadd
description: When implementing SIMD mad, nmad, or rcp_precise, use hardware FMA intrinsics and use fmadd with negated multiplicand when fnmadd negates the sum
rubric: safety
severity: should_fix
---

**Rule.** In SIMD math implementations (such as SkRasterPipeline_opts.h),
implement mad and nmad using hardware fused multiply-add (FMA) intrinsics when
available, and express Newton-Raphson reciprocal refinement in rcp_precise using
nmad(v, e, 2.0f) instead of separate multiplication and subtraction. When
implementing nmad (a - f * m) on architectures where fnmadd negates the entire
accumulated sum (-(f * m + a)), use fmadd(-f, m, a) with a negated multiplicand
rather than passing a negated accumulator into fnmadd(f, m, -a).

**Why.** Hardware FMA instructions execute multiply-accumulate in a single cycle
with a single rounding step, improving both throughput and precision in raster
pipeline stages. In Newton-Raphson reciprocal refinement, routing through nmad
ensures compilers emit fused instructions, while avoiding fnmadd on
architectures that negate the accumulated sum prevents confusing double-negation
conventions.

## Examples

Bad:

```cpp
SI F mad(F f, F m, F a)   { return a + f * m; }
SI F nmad(F f, F m, F a)  { return __lasx_xvfnmadd_s(f, m, -a); }
SI F rcp_precise(F v)     { F e = rcp_approx(v); return e * (2.0f - v * e); }
```

Good:

```cpp
SI F mad(F f, F m, F a)   { return __lasx_xvfmadd_s(f, m, a); }
SI F nmad(F f, F m, F a)  { return __lasx_xvfmadd_s(-f, m, a); }
SI F rcp_precise(F v)     { F e = rcp_approx(v); return e * nmad(v, e, F() + 2.0f); }
```
