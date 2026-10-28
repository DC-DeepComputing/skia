---
name: skia/qualify-curve-tolerance-constants-by-degree
description: When defining tolerance or threshold constants in curve flattening and tessellation algorithms, qualify names by curve degree or stage rather than generic kTol
rubric: readability
severity: should_fix
---

**Rule.** In curve flattening, tessellation, and geometry algorithms, qualify
tolerance and threshold constants with the specific curve degree (e.g.,
kQuadTolerance / kQuadErrTolerance, kCubicTolerance, kConicTolerance) or
approximation stage (e.g., kConicToCubicErrTolerance, kQuadToLineTolFromCubic).
Do not use bare, generic names like kTol or kTolerance when algorithms process
or convert between multiple curve types.

**Why.** Curve flattening and tessellation pipelines often process multiple
curve types (quadratics, cubics, conics) or chained approximation stages, each
with its own error threshold and budget. Generic names like kTol or kTolerance
obscure which curve degree or error phase is controlled, making error budget
tuning and maintenance error-prone.

## Examples

Bad:

```cpp
// Bad: Generic tolerance names obscure which curve degree or stage is controlled.
static constexpr SkScalar kTolerance = Flatten::kTol;
static constexpr SkScalar kToleranceSqd = Flatten::kTol2;
```

Good:

```cpp
// Good: Qualify tolerance constants by curve degree and approximation stage.
static constexpr SkScalar kTolerance = Flatten::kQuadErrTolerance;
static constexpr SkScalar kToleranceSqd = Flatten::kQuadTolerance2;
```
