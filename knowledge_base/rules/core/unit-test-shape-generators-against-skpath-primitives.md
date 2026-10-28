---
name: skia/core/unit-test-shape-generators-against-skpath-primitives
description: When implementing new geometric shape representations or path generators, add unit tests (DEF_TEST) validating verbs, points, and conics against standard SkPath primitives
rubric: testing
severity: should_fix
---

**Rule.** When implementing geometric shape representations or path generators
(such as SkPathRawShapes), add unit tests using DEF_TEST in tests/ that validate
the generated verbs, points, and conic weights against standard SkPath factory
primitives (such as SkPath::Rect, SkPath::Oval, and SkPath::RRect). Verify that
bounds and segment masks match and test across all supported winding directions
(SkPathDirection::kCW and kCCW) and starting point indices.

**Why.** Geometric coordinate calculation and midpoint generation for curves are
prone to subtle arithmetic, orientation, and indexing bugs. Comparing generated
raw shape components directly against canonical SkPath equivalents across all
winding directions ensures geometric parity and catches discrepancies before
landing.

## Examples

Bad:

```cpp
// Implementing a shape generator without companion unit tests verifying parity:
SkPathRawShapes::Oval::Oval(const SkRect& r, SkPathDirection dir, unsigned index)
    : SkPathRaw{fStorage, gVerbs, gConics, r, SkPathFillType::kWinding, true, kConic_SkPathSegmentMask}
{
    // Generates points and conics for oval contour, but lacks DEF_TEST coverage
}
```

Good:

```cpp
// In tests/PathRawShapesTest.cpp:
DEF_TEST(pathrawshapes_oval, reporter) {
    const SkRect r = {1, 2, 3, 4};
    for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
        SkPathRawShapes::Oval shape(r, dir);
        REPORTER_ASSERT(reporter, shape.bounds() == r);
        REPORTER_ASSERT(reporter, shape.isKnownToBeConvex());

        const SkPath canonical = SkPath::Oval(r, dir);
        check_path_is_raw(reporter, canonical, shape);
    }
}
```
