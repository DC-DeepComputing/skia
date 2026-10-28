---
name: skia/match-boundary-constant-type-to-parameter
description: When defining range, min, or max boundary constants for continuous floating-point parameters, declare them using the parameter's type rather than int
rubric: readability
severity: should_fix
---

**Rule.** When declaring range, minimum, or maximum boundary constants (such as
kMin... or kMax...) that bound continuous floating-point properties or API
parameters (such as float, double, or SkScalar), declare the constants using the
matching floating-point type rather than int. Do not define bounds of continuous
parameters as integer constants, even when the boundary values are whole numbers
like 0, 1, or 4.

**Why.** Declaring boundary constants for continuous parameters as int
misleadingly implies to callers that only discrete whole numbers are valid (for
example, suggesting contrast can only be 0 or 1). Furthermore, integer types
cause unnecessary type friction, requiring explicit casts or conversion helpers
(such as SkIntToScalar) and risking conversion or comparison warnings at
clamping and validation sites.

## Examples

Bad:

```cpp
// Bad: Declaring boundary constants for continuous float/SkScalar parameters as int
static constexpr int kMinContrastInclusive = 0;
static constexpr int kMaxContrastInclusive = 1;
static constexpr int kMinGammaInclusive = 0;
static constexpr int kMaxGammaExclusive = 4;

void setContrast(SkScalar contrast) {
    SkASSERT(contrast >= SkIntToScalar(kMinContrastInclusive) &&
             contrast <= SkIntToScalar(kMaxContrastInclusive));
}
```

Good:

```cpp
// Good: Matching boundary constant types to the continuous parameter type
static constexpr SkScalar kMinContrastInclusive = 0;
static constexpr SkScalar kMaxContrastInclusive = 1;
static constexpr SkScalar kMinGammaInclusive = 0;
static constexpr SkScalar kMaxGammaExclusive = 4;

void setContrast(SkScalar contrast) {
    SkASSERT(contrast >= kMinContrastInclusive &&
             contrast <= kMaxContrastInclusive);
}
```
