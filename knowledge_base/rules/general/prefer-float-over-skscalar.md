---
name: skia/prefer-float-over-skscalar
description: When declaring or converting floating-point variables, coordinates, spans, casts, or conversion helpers, use standard float instead of the legacy SkScalar typedef
rubric: design
severity: should_fix
---

**Rule.** Use standard float instead of the legacy SkScalar typedef (and
standard float constants like 1.0f instead of SK_Scalar1) when declaring
floating-point variables, coordinates, spans, interface parameters, casts, and
conversion helpers or macros. Do not introduce or retain new uses or casts of
SkScalar in newly authored, refactored, or modernized code.

**Why.** SkScalar was historically a typedef for float (and fixed-point before
that), but Skia is actively deprecating and eliminating SkScalar across the
codebase in favor of standard C++ float. Using float directly avoids
perpetuating deprecated legacy aliases in modern code.

## Examples

Bad:

```cpp
SkScalar w = -1.0f;
SkSpan<const SkScalar> fConics;
SkScalar radius = SK_Scalar1;
#define SkFDot6ToFloat(x) ((SkScalar)(x) * 0.015625f)
```

Good:

```cpp
float w = -1.0f;
SkSpan<const float> fConics;
float radius = 1.0f;
#define SkFDot6ToFloat(x) ((float)(x) * 0.015625f)
```
