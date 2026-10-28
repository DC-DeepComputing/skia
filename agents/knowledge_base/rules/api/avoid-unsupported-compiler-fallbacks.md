---
name: skia/api/avoid-unsupported-compiler-fallbacks
description: When writing compiler feature or attribute preprocessor guards, do not add fallback branches for toolchains older than Skia's supported compiler baseline
rubric: design
severity: should_fix
---

**Rule.** Do not add compatibility workarounds or fallback preprocessor branches
for compiler versions older than Skia's minimum supported toolchain baseline.
When checking when a compiler attribute or feature became available, verify the
initial major release where it was introduced (such as GCC 8.0) rather than
relying on point releases shown in online documentation (such as GCC 8.5).

**Why.** Carrying workaround code and fallback branches for compiler versions
older than Skia's supported baseline adds unnecessary code complexity and
maintenance burden. Relying on online compiler documentation indices can mislead
developers into assuming a feature was added in a late point release when it was
actually supported since the initial series release.

## Examples

Bad:

```cpp
#if __has_attribute(no_sanitize)
  // GCC documentation lists 8.5, but 8.0 introduced no_sanitize
  #define SK_NO_SANITIZE(A) SK_ATTRIBUTE(no_sanitize(A))
#elif __has_attribute(no_sanitize_undefined)
  // Fallback for GCC <= 7.5, which is older than Skia's supported baseline
  #define SK_NO_SANITIZE(A) SK_ATTRIBUTE(no_sanitize_undefined)
#else
  #define SK_NO_SANITIZE(A)
#endif
```

Good:

```cpp
#if __has_attribute(no_sanitize)
  // GCC >= 8.0 and Clang support no_sanitize; no fallback needed for older GCC
  #define SK_NO_SANITIZE(A) SK_ATTRIBUTE(no_sanitize(A))
#else
  #define SK_NO_SANITIZE(A)
#endif
```
