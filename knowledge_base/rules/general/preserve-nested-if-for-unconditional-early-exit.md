---
name: skia/preserve-nested-if-for-unconditional-early-exit
description: When guarding early exits or item exclusion, keep inner checks separate and assert with SkASSERT on fallthrough that excluded items never bypass bailout
rubric: correctness
severity: should_fix
---

**Rule.** When an outer condition or exclusion flag requires an early exit
(`return`, `break`), do not collapse inner conditions or downstream exceptions
into the outer guard with `&&`, which allows excluded items to bypass early exit
and fall through. Keep nested conditions separate or explicitly negate
downstream exceptions in the bailout guard, and verify on subsequent fallback
paths with `SkASSERT` that excluded items never fall through to regular
processing.

**Why.** Collapsing inner checks into an outer early-exit guard or improperly
restricting a bailout condition allows items that satisfy outer exclusion
criteria to bypass early termination and execute normal code. Keeping conditions
distinct prevents unintended fallthrough, while asserting invariants on fallback
paths ensures accidental bypasses fail immediately in debug builds.

## Examples

Bad:

```cpp
// Variant 1: Collapsing inner checks allows dst-dependent draws to bypass early exit
if (dependsOnDst && canForwardMerge && result.second) {
    forwardMerge = result.second;
    return true;
} else {
    return false;
}

// Variant 2: Overly restrictive bailout allows specialized params to fall through
if (paramIsSpecialized && param->type().isSampler() && !fUseTextureSamplerPairs) {
    return NA;
}
SpvId tmpVar;  // Incorrectly processes specialized parameters
```

Good:

```cpp
// Variant 1: Separate nested condition ensures all dst-dependent draws early-exit
if (dependsOnDst) {
    if (canForwardMerge && result.second) {
        forwardMerge = result.second;
    }
    return true;
} else {
    return false;
}

// Variant 2: Negate exception and assert on fallback path that specialized params never fall through
if (paramIsSpecialized && !(param->type().isSampler() && fUseTextureSamplerPairs)) {
    return NA;
}
SkASSERT(!paramIsSpecialized);
SpvId tmpVar;
```
