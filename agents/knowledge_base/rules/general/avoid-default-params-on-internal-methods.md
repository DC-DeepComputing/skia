---
name: skia/avoid-default-params-on-internal-methods
description: When declaring internal subsystem functions or methods, avoid default parameter values; require callers to pass explicit arguments, deliberate fallbacks, or std::nullopt
rubric: readability
severity: should_fix
---

**Rule.** Do not add default parameter values (such as `= nullptr`, `=
std::nullopt`, or fallback literals like `= {0, 0, 0, 0}`) to internal subsystem
functions or methods. Require callers to explicitly pass arguments (such as a
concrete value, pointer, deliberate fallback like `SkRect::MakeEmpty()`, or
`std::nullopt`) so that call sites are deliberately audited and data flow
remains unambiguous.

**Why.** Default parameter values allow internal callers to silently inherit
fallback or sentinel values without conscious auditing. Unlike public APIs in
include/ where default arguments preserve backward compatibility for external
consumers, internal APIs benefit from forcing all call sites to deliberately
supply intended values or explicitly justify fallbacks.

## Examples

Bad:

```cpp
// Bad: Default arguments allow callers to silently omit deliberate choices
void purgeResourcesNotUsedSince(
        StdSteadyClock::time_point purgeTime,
        const StdSteadyClock::time_point* stopPurgingPastTime = nullptr);

SkBlitter* SkCreateRasterPipelineBlitter(const SkPixmap&,
                                         const SkPaint&,
                                         const SkMatrix& ctm,
                                         SkArenaAlloc*,
                                         sk_sp<SkShader> clipShader,
                                         const SkSurfaceProps& props,
                                         const SkRect& devBounds = {0, 0, 0, 0});
```

Good:

```cpp
// Good: Callers must explicitly pass arguments or justify fallbacks
void purgeResourcesNotUsedSince(
        StdSteadyClock::time_point purgeTime,
        std::optional<StdSteadyClock::time_point> stopPurgingPastTime);

SkBlitter* SkCreateRasterPipelineBlitter(const SkPixmap&,
                                         const SkPaint&,
                                         const SkMatrix& ctm,
                                         SkArenaAlloc*,
                                         sk_sp<SkShader> clipShader,
                                         const SkSurfaceProps& props,
                                         const SkRect& devBounds);
```
