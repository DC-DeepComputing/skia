---
name: skia/use-skrectpriv-half-dimensions-to-prevent-overflow
description: When computing the half-width or half-height of an SkRect, use SkRectPriv::HalfWidth or HalfHeight rather than halving rect.width() or rect.height() to avoid float overflow
rubric: correctness
severity: should_fix
---

**Rule.** When computing the half-width or half-height of an SkRect whose
coordinates may be large or untrusted, call SkRectPriv::HalfWidth(rect) and
SkRectPriv::HalfHeight(rect). Do not compute half dimensions using expressions
like 0.5f * rect.width(), rect.width() * 0.5f, SK_ScalarHalf * rect.width(), or
rect.width() / 2.

**Why.** SkRect::width() and height() compute fRight - fLeft and fBottom - fTop
via 32-bit float subtraction. When bounds coordinates are large (such as in
fuzzer targets or extreme geometries), direct subtraction can overflow float to
infinity before halving, whereas SkRectPriv::HalfWidth and HalfHeight compute
the midpoint in double precision before narrowing to float.

## Examples

Bad:

```cpp
if (offset > std::min(SkTAbs(SK_ScalarHalf * bounds.width()),
                      SkTAbs(SK_ScalarHalf * bounds.height()))) {
    return false;
}
```

Good:

```cpp
if (offset > std::min(SkTAbs(SkRectPriv::HalfWidth(bounds)),
                      SkTAbs(SkRectPriv::HalfHeight(bounds)))) {
    return false;
}
```
