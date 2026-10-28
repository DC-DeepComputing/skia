---
name: skia/document-positive-macro-mappings-for-negative-flags
description: When defining or refactoring preprocessor configuration flags that map negative macros to positive internal flags, include explanatory comments justifying the indirection to avoid double negations
rubric: readability
severity: should_fix
---

**Rule.** When defining or refactoring preprocessor configuration flags,
preserve or add explanatory comments that justify non-obvious macro
mappings—especially when deriving positive internal macros (such as SK*_USE_*)
from negative external build flags (such as SK*_DISABLE_*). Explicitly document
that the positive internal flag is used to avoid confusing double-negation
checks across downstream code.

**Why.** External build systems and client configs frequently expose negative
toggles (e.g., SK*_DISABLE_*), but referencing them internally leads to
confusing double-negation expressions (such as !defined(SK*_DISABLE_*) or #if
!SK*_DISABLE_*). Mapping negative flags to positive internal macros clarifies
conditional logic, but without an explanatory comment, maintainers may view the
mapping as redundant indirection and remove it during refactoring.

## Examples

Bad:

```cpp
#if defined(SKVX_DISABLE_SIMD)
#define SKVX_USE_SIMD 0
#else
#define SKVX_USE_SIMD 1
#endif
```

Good:

```cpp
// Use SKVX_USE_SIMD internally to avoid confusing double negation.
#if defined(SKVX_DISABLE_SIMD)
#define SKVX_USE_SIMD 0
#else
#define SKVX_USE_SIMD 1
#endif
```
