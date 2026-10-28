---
name: skia/core/allow-empty-points-for-point-free-path-verbs
description: When constructing SkPath from point and verb spans, do not reject empty point spans early; validate sufficiency against verbs because verbs like kClose consume zero points
rubric: correctness
severity: should_fix
---

**Rule.** When constructing SkPath instances from raw point and verb spans (such
as in SkPath::Make or SkPathData::Make), do not reject or early-exit on empty
point spans (pts.empty()). Verify point count sufficiency against the verb
sequence (e.g. info.points > pts.size()), as valid paths may contain point-free
verbs like kClose.

**Why.** Degenerate paths or sequences containing only point-free verbs (such as
kClose) consume zero points and should not be discarded. Unconditionally
checking pts.empty() causes subtle behavioral regressions by dropping valid
paths that verb analysis would otherwise accept.

## Examples

Bad:

```cpp
SkPath SkPath::Make(SkSpan<const SkPoint> pts,
                    SkSpan<const uint8_t> vbs,
                    SkSpan<const SkScalar> ws,
                    SkPathFillType ft, bool isVolatile) {
    if (pts.empty() || vbs.empty()) {
        return SkPath();
    }
    // ...
}
```

Good:

```cpp
SkPath SkPath::Make(SkSpan<const SkPoint> pts,
                    SkSpan<const uint8_t> vbs,
                    SkSpan<const SkScalar> ws,
                    SkPathFillType ft, bool isVolatile) {
    if (vbs.empty()) {
        return SkPath();
    }
    // Point count sufficiency is validated against verb requirements.
    // ...
}
```
