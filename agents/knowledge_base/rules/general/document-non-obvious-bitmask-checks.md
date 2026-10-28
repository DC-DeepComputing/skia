---
name: skia/document-non-obvious-bitmask-checks
description: When testing bitmasks using non-obvious expressions—such as negated subset checks or non-zero cleared-state comparisons—add an explanatory comment or descriptive helper
rubric: readability
severity: nit
---

**Rule.** When testing bitmask flags or packed state using non-obvious
expressions, add an explanatory inline comment or extract the condition into an
intention-revealing helper. This includes negated mask expressions testing flag
subsets (such as `0 == (mask & ~(kFlagA | kFlagB))` or `!(mask &
~kAllowedMask)`) as well as checks that test for cleared state or empty
reference counts by comparing against a non-zero sentinel or flag mask instead
of zero. Clarify the active state combinations or why the non-zero comparison is
required so reviewers and maintainers do not mistake the condition for a defect.

**Why.** Negated bitmask expressions require mental boolean inversion, and
comparing masked values against non-zero sentinels to verify cleared state is
counter-intuitive—especially when adjacent checks compare to zero. An
explanatory comment or dedicated predicate conveys algorithmic intent, prevents
maintainers from suspecting bugs, and avoids accidental mis-refactoring.

## Examples

Bad:

```cpp
// Variant 1: Negated subset check without explanation
TypeMask mask = this->getType();
if (0 == (mask & ~(kScale_Mask | kTranslate_Mask))) {
    if (mask & kScale_Mask) { ... }
}

// Variant 2: Masked reference count compared to non-zero sentinel to check empty state
return {(origRefs & fReusableRefMask) == RefMask(RefType::kReturnQueue),
        (origRefs & PurgeableMask()) == 0};
```

Good:

```cpp
// Variant 1: Inline comment clarifies permitted states (or extract a helper)
TypeMask mask = this->getType();
// Optimized invert for only scale and/or translation matrices.
if (0 == (mask & ~(kScale_Mask | kTranslate_Mask))) {
    if (mask & kScale_Mask) { ... }
}

// Variant 2: Document why non-zero comparison detects cleared reusable refs
// `fReusableRefMask` includes the ReturnQueue bit; compare against it to detect
// when the actual reusable refs are all zero, whereas PurgeableMask() compares to 0.
return {(origRefs & fReusableRefMask) == RefMask(RefType::kReturnQueue),
        (origRefs & PurgeableMask()) == 0};
```
