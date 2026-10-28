---
name: skia/tests/avoid-asserting-exact-anti-aliased-pixel-colors
description: When inspecting rasterized or anti-aliased pixels in unit tests, assert color ranges, tolerances, or non-background states instead of brittle exact intermediate color values
rubric: correctness
severity: should_fix
---

**Rule.** When authoring or modifying unit tests that inspect rasterized or
anti-aliased pixel outputs, do not assert exact intermediate color values (such
as hardcoded intermediate hex colors or specific shades of gray). Instead,
assert acceptable color bounding ranges (e.g., between minimum and maximum
bounds), comparison tolerances, or definite states (such as verifying a pixel is
non-white or not the background color).

**Why.** Antialiasing, linear blending, and subpixel rasterization calculations
vary subtly across CPU architectures, GPU backends, compilers, and
floating-point optimization settings. Asserting exact intermediate pixel values
makes unit tests brittle and causes spurious test failures when rendering math
or coverage equations undergo minor, valid adjustments.

## Examples

Bad:

```cpp
// Brittle: fails if antialiasing, interpolation, or rounding math changes slightly
InspectPixelColorAtPoint(reporter, pixmap, "above first point", {5.0, 4.0}, 0xffbfbfbf);
InspectPixelColorAtPoint(reporter, pixmap, "left of first point", {4.0, 5.0}, 0xff808080);
```

Good:

```cpp
// Robust: verifies bounded color range or definite non-background coverage
assert_color_in_range(reporter, pixmap, "above first point", {5, 4},
                      /*expected_min=*/0xff808080, /*expected_max=*/SK_ColorWHITE);
assert_not_color(reporter, pixmap, "left of first point", {4, 5}, SK_ColorWHITE);
```
