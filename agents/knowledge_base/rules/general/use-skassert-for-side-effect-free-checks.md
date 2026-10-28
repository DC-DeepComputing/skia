---
name: skia/use-skassert-for-side-effect-free-checks
description: Use SkASSERT for pure side-effect-free checks so they compile away, and reserve SkAssertResult for mutating operations that must execute and succeed
rubric: readability
severity: should_fix
---

**Rule.** Do not use SkAssertResult for pure, side-effect-free expressions (such
as checking std::optional::has_value, pointer validity, or relational
comparisons); use SkASSERT instead. Conversely, when asserting expressions with
required runtime side effects or mutating operations that return a boolean
status expected to always succeed (such as SkIRect::intersect or
SkMatrix::invert), wrap them in SkAssertResult rather than SkASSERT or ignoring
the return value.

**Why.** In release builds (!SK_DEBUG), SkASSERT expands to
static_cast<void>(0), eliding the expression entirely. Placing mutating
operations in SkASSERT causes them to be skipped in release builds, while
ignoring their boolean status misses debug invariant verification. Conversely,
SkAssertResult unconditionally evaluates its condition (expanding to if (cond)
{}), which executes necessary side effects in release builds while asserting
success in debug builds, but incurs wasted runtime work if applied to pure
checks.

## Examples

Bad:

```cpp
// Bad: pure check in SkAssertResult runs needlessly in release builds
auto lastPt = contour.getLastPt();
SkAssertResult(lastPt.has_value());
next = *lastPt;

// Bad: mutating operation in SkASSERT is skipped in release builds
SkIRect msaaArea = renderPassBounds;
SkASSERT(msaaArea.intersect(SkIRect::MakeSize(colorTexture->dimensions())));
```

Good:

```cpp
// Good: pure check elided in release builds
auto lastPt = contour.getLastPt();
SkASSERT(lastPt.has_value());
next = *lastPt;

// Good: mutating operation executed in all builds and asserted in debug
SkIRect msaaArea = renderPassBounds;
SkAssertResult(msaaArea.intersect(SkIRect::MakeSize(colorTexture->dimensions())));
```
