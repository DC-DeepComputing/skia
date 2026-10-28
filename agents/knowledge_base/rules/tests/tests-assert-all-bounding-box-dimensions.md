---
name: skia/tests/tests-assert-all-bounding-box-dimensions
description: When testing geometric bounds or bounding rectangles in unit tests, assert expected values for all dimensions and corners rather than loose single-coordinate inequalities
rubric: testing
severity: should_fix
---

**Rule.** When testing geometric bounds, cull rects, or bounding boxes in unit
tests, do not rely on loose or partial assertions (such as checking only that a
single coordinate is positive or non-zero). Assert specific expected values
across all dimensions or corners (such as left, top, right, and bottom
coordinates), using numerical tolerance where appropriate for floating-point
bounds.

**Why.** Asserting only that a single dimension or coordinate is positive does
not verify that the bounding geometry was calculated correctly; remaining
coordinates may be uninitialized, zero, or incorrect while the test still
passes. Verifying all dimensions or corner coordinates against expected values
ensures bounding calculations and transformations are fully tested and prevents
undetected layout or clipping regressions.

## Examples

Bad:

```cpp
// Loose single-coordinate check leaves other bounds components unverified:
const cullRect = pic.cullRect();
expect(cullRect[2]).toBeGreaterThan(0);

// In C++, checking only width > 0 leaves left, top, and height untested:
SkRect bounds = picture->cullRect();
REPORTER_ASSERT(reporter, bounds.width() > 0);
```

Good:

```cpp
// Verify expected values across all four bounding coordinates:
const cullRect = pic.cullRect();
expect(cullRect[0]).toBeCloseTo(0);
expect(cullRect[1]).toBeCloseTo(31);
expect(cullRect[2]).toBeCloseTo(357.84);
expect(cullRect[3]).toBeCloseTo(109.42);

// In C++, assert all coordinates against expected bounding geometry:
SkRect bounds = picture->cullRect();
REPORTER_ASSERT(reporter, SkScalarNearlyEqual(bounds.fLeft, 0.0f));
REPORTER_ASSERT(reporter, SkScalarNearlyEqual(bounds.fTop, 31.0f));
REPORTER_ASSERT(reporter, SkScalarNearlyEqual(bounds.fRight, 357.84f));
REPORTER_ASSERT(reporter, SkScalarNearlyEqual(bounds.fBottom, 109.42f));
```
