---
name: skia/sksl/document-shader-division-guard-threshold-limits
description: When guarding or omitting division guards in shaders, document cutoff threshold overflow limits or why zero denominators safely propagate
rubric: readability
severity: nit
---

**Rule.** When introducing fixed cutoff thresholds, minimum-normal constants, or
epsilons to guard shader division against zero denominators, document the input
assumptions, limitations, and numerator magnitude bounds under which overflow
can still occur. Additionally, when intentionally omitting a divide-by-zero
guard or branch in shader or numerical calculations, document why the zero
denominator is geometrically or numerically improbable and how floating-point
values (+/-Inf or NaN) safely propagate through downstream checks without
causing visual artifacts.

**Why.** In GPU shaders, dynamic magnitude checks like isfinite() are often
unsupported or unreliable across drivers, while explicit branches introduce
pipeline divergence. Fixed denominator cutoff constants only prevent denormals
and zero denominators but still overflow on large numerators, whereas unguarded
divisions risk visual defects unless downstream logic handles IEEE 754 float
propagation. Documenting overflow bounds or safe propagation invariants
clarifies numerical intent and prevents review churn.

## Examples

Bad:

```cpp
// 1) Fixed cutoff without documenting numerator overflow bounds:
// Denormal half floats will compare not-equal to 0 but cause overflow to infinity.
const half $kMinNormalHalf = 1.0 / (1 << 14);

// 2) Unguarded division without documenting float propagation:
if (a == 0) {
    // Linear case, both circles intersect at exactly one point.
    t = c / negB;
}
```

Good:

```cpp
// 1) Document numerator magnitude bounds and approximation limits:
// Denormal half floats can cause overflow to infinity; values x > ~3.998 will
// overflow when divided by $kMinNormalHalf. Denominator checks like `denom < x/F16_MAX`
// are ideal, but $kMinNormalHalf is safe for typical wide-gamut inputs.
const half $kMinNormalHalf = 1.0 / (1 << 14);

// 2) Document improbable zero and downstream float propagation:
if (a == 0) {
    // Linear case, both circles intersect at exactly one point.
    // Unlikely b and c are 0 (NaN). If negB == 0 due to translation, +/-Inf
    // propagates sign to isValid, preserving expected pixel rendering.
    t = c / negB;
}
```
