---
name: skia/prefer-enum-kdefault-sentinel
description: When passing or assigning enum values where default behavior is intended, use the enum's kDefault sentinel rather than underlying enumerators like kWinding
rubric: readability
severity: nit
---

**Rule.** When an enum defines a kDefault sentinel (such as
SkPathFillType::kDefault or SkPathDirection::kDefault), pass or assign
Enum::kDefault whenever default behavior is intended. Do not specify the
underlying default enumerator (such as kWinding or kCW) unless the call site
specifically and intentionally requires that particular mode regardless of
defaults.

**Why.** Using kDefault explicitly documents the caller's intent to rely on
standard default semantics. It decouples the call site from the underlying
enumerator value, ensuring that changes to the default enum mapping propagate
cleanly without code churn or silent semantic drift.

## Examples

Bad:

```cpp
// Passing concrete enumerator when default behavior is intended
return SkPath::Raw(ptsSpan, verbsSpan, weightsSpan, SkPathFillType::kWinding);

SkPath path = SkPath::Rect(rect, SkPathDirection::kCW);
```

Good:

```cpp
// Use kDefault sentinel to explicitly convey default behavior
return SkPath::Raw(ptsSpan, verbsSpan, weightsSpan, SkPathFillType::kDefault);

SkPath path = SkPath::Rect(rect, SkPathDirection::kDefault);
```
