---
name: skia/gpu/avoid-coeff-blend-checks-in-advanced-blends
description: When handling advanced blend modes guarded by mode > SkBlendMode::kLastCoeffMode, do not check or assert coefficient blend modes like SkBlendMode::kSrcOver
rubric: correctness
severity: should_fix
---

**Rule.** Do not include checks or assertions for Porter-Duff coefficient blend
modes (such as SkBlendMode::kSrcOver, where mode <= SkBlendMode::kLastCoeffMode)
inside code paths guarded by mode > SkBlendMode::kLastCoeffMode. In advanced
blend mode branches, only check invariants applicable to advanced blending (such
as single-channel coverage requirements).

**Why.** SkBlendMode is partitioned into coefficient blend modes (enum values up
to kLastCoeffMode) and advanced blend modes (values strictly greater than
kLastCoeffMode). Checking or asserting for a coefficient mode like kSrcOver
inside an advanced blend branch is dead logic that contradicts the enclosing
guard and misleads readers about supported blend modes.

## Examples

Bad:

```cpp
if (finalBlendMode > SkBlendMode::kLastCoeffMode) {
    SkASSERT(finalCoverage == Coverage::kSingleChannel ||
             finalBlendMode == SkBlendMode::kSrcOver);
    fBlendInfo = gBlendTable[static_cast<int>(*finalBlendMode)];
}
```

Good:

```cpp
if (finalBlendMode > SkBlendMode::kLastCoeffMode) {
    SkASSERT(finalCoverage == Coverage::kSingleChannel);
    fBlendInfo = gBlendTable[static_cast<int>(*finalBlendMode)];
}
```
