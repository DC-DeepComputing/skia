---
name: skia/avoid-unnecessary-optional-return-type
description: When designing function signatures, do not wrap return types in std::optional<T> if the function never returns std::nullopt; return T directly
rubric: design
severity: should_fix
---

**Rule.** Do not wrap a function's return type in std::optional<T> when all
execution paths return a valid value or fallback representation, and
std::nullopt is never returned. Return T directly so callers are not burdened
with unnecessary unwrapping, null checks, or defensive error handling.

**Why.** Wrapping guaranteed return values in std::optional adds type
complexity, forces callers to inspect and unwrap the result (e.g., via *result
or .value()), and misleads readers into expecting a failure path where none
exists.

## Examples

Bad:

```cpp
// Bad: Function cannot fail, but wraps the result in std::optional.
std::optional<SkPathConvexity> SkPathPriv::ComputeConvexity(SkSpan<const SkPoint> pts,
                                                           SkSpan<const SkPathVerb> vbs,
                                                           SkSpan<const float> weights) {
    if (vbs.empty()) {
        return SkPathConvexity::kConvex_Degenerate;
    }
    return CalculateConvexity(pts, vbs, weights);
}
```

Good:

```cpp
// Good: Return the result type directly since absence is never possible.
SkPathConvexity SkPathPriv::ComputeConvexity(SkSpan<const SkPoint> pts,
                                            SkSpan<const SkPathVerb> vbs,
                                            SkSpan<const float> weights) {
    if (vbs.empty()) {
        return SkPathConvexity::kConvex_Degenerate;
    }
    return CalculateConvexity(pts, vbs, weights);
}
```
