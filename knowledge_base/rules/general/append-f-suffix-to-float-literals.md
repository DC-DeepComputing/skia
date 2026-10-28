---
name: skia/append-f-suffix-to-float-literals
description: When assigning, passing, returning, or comparing floating-point values against literals, append the 'f' suffix (e.g., 1.0f) to prevent double conversions and MSVC warnings
rubric: correctness
severity: should_fix
---

**Rule.** Append the 'f' suffix to floating-point numeric literals (e.g., 1.0f
instead of 1.0) when comparing against float or SkScalar values, passing them as
arguments, assigning them to float variables, or returning float results. Do not
use unsuffixed floating-point literals in expressions involving 32-bit floats.

**Why.** In C++, unsuffixed floating-point literals (like 1.0 or 0.5) have type
double. Passing, assigning, or comparing them with 32-bit float or SkScalar
operands causes implicit double-to-float narrowing or unwanted double-precision
promotion, triggering compiler warnings on MSVC and potentially breaking
warning-clean Windows builds.

## Examples

Bad:

```cpp
// Unsuffixed double literals cause narrowing conversions or double promotion
SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1.0, false);
SkRRect::MakeRectXY(rect, 3.1, 1.5);

float scale = 0.5;
SkPoint pt = {1.0, 2.0};

if (curr == 1.0 && fLastStopIsImplicit) {
    fLastStopIsImplicit = false;
}
```

Good:

```cpp
// Explicit 'f' suffix avoids narrowing conversions and MSVC comparison warnings
SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1.0f, false);
SkRRect::MakeRectXY(rect, 3.1f, 1.5f);

float scale = 0.5f;
SkPoint pt = {1.0f, 2.0f};

if (curr == 1.0f && fLastStopIsImplicit) {
    fLastStopIsImplicit = false;
}
```
