---
name: skia/validate-geometry-bounds-at-api-boundary
description: When processing point coordinates, validate bounds with SkRect::Bounds() at caller boundaries and early-exit instead of calling .value() or BoundsOrEmpty() in internal helpers
rubric: safety
severity: should_fix
---

**Rule.** Validate point coordinates using SkRect::Bounds() at caller and API
boundaries before passing geometry to internal constructors or helpers,
early-exiting or bailing out when nullopt is returned. Do not call .value() on
unvalidated bounds or use BoundsOrEmpty() in internal helpers to synthesize
fallback bounds for non-finite inputs; require callers to pass pre-validated
SkRect bounds into internal helpers and assert their validity with SkASSERT.

**Why.** SkRect::Bounds() returns std::optional<SkRect>, which evaluates to
nullopt when coordinates are non-finite (NaN or infinity). Calling .value()
directly on unvalidated input risks throwing std::bad_optional_access or
crashing, while BoundsOrEmpty() silently substitutes a (0, 0, 0, 0) rectangle
that contradicts the geometry. Validating upfront prevents redundant bounds
recomputations and ensures internal helpers operate safely under asserted
invariants.

## Examples

Bad:

```cpp
// Internal helper constructor computing and unwrapping bounds directly on unvalidated points:
SkPathRawShapes::Triangle::Triangle(SkSpan<const SkPoint> threePoints)
    : SkPathRaw{threePoints, gTriangle_Verbs, {},
                SkRect::Bounds(threePoints).value(),
                SkPathFillType::kDefault, true, kLine_SkPathSegmentMask}
{
    SkASSERT(threePoints.size() == 3);
}
```

Good:

```cpp
// Caller boundary validates bounds and early-exits if non-finite:
void SkScan::FillTriangle(const SkPoint pts[], const SkRasterClip& clip, SkBlitter* blitter) {
    const auto r = SkRect::Bounds({pts, 3});
    if (!r) {
        return;
    }
    SkPathRawShapes::Triangle tri({pts, 3}, *r);
    ...
}

// Internal helper constructor accepts pre-validated bounds and asserts validity:
SkPathRawShapes::Triangle::Triangle(SkSpan<const SkPoint> threePoints, const SkRect& bounds)
    : SkPathRaw{threePoints, gTriangle_Verbs, {}, bounds,
                SkPathFillType::kDefault, true, kLine_SkPathSegmentMask}
{
    SkASSERT(threePoints.size() == 3);
    SkASSERT(SkRect::Bounds(threePoints).value() == bounds);
}
```
