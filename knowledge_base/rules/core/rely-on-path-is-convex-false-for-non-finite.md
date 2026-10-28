---
name: skia/core/rely-on-path-is-convex-false-for-non-finite
description: When querying path convexity or populating path metadata, rely on SkPath::isConvex() returning false for non-finite paths instead of branching or duplicating code
rubric: readability
severity: should_fix
---

**Rule.** When checking convexity or populating path descriptors, rely on
SkPath::isConvex() evaluating to false for non-finite paths (!path.isFinite()).
Do not write special-case branches or duplicate initialization code solely to
avoid calling isConvex() or to hardcode false for non-finite paths.

**Why.** SkPath::computeConvexity() explicitly classifies any non-finite path as
concave (SkPathConvexity::kConcave), guaranteeing that isConvex() safely returns
false whenever !path.isFinite(). Branching or hardcoding false to avoid calling
isConvex() introduces redundant control flow and unnecessary code duplication.

## Examples

Bad:

```cpp
if (ref->isFinite()) {
    return {
        ref->pointSpan(), ref->verbs(), ref->conicSpan(),
        ref->getBounds(), path.getFillType(), path.isConvex(),
        SkTo<uint8_t>(ref->getSegmentMasks()),
    };
} else {
    return {
        ref->pointSpan(), ref->verbs(), ref->conicSpan(),
        {SK_ScalarInfinity, SK_ScalarInfinity, SK_ScalarInfinity, SK_ScalarInfinity},
        path.getFillType(), false,
        SkTo<uint8_t>(ref->getSegmentMasks()),
    };
}
```

Good:

```cpp
return {
    ref->pointSpan(),
    ref->verbs(),
    ref->conicSpan(),
    ref->isFinite() ? ref->getBounds()
                    : SkRect{SK_ScalarInfinity, SK_ScalarInfinity,
                             SK_ScalarInfinity, SK_ScalarInfinity},
    path.getFillType(),
    path.isConvex(),
    SkTo<uint8_t>(ref->getSegmentMasks()),
};
```
