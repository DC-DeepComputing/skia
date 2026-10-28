---
name: skia/name-color-conversions-by-space-family
description: When naming color conversion or transformation helpers whose output space varies across a family, use general family names like rgb rather than specific spaces like srgb
rubric: readability
severity: nit
---

**Rule.** Do not name color conversion or transformation helpers after a
specific color space (such as 'srgb') when the output color space varies across
an entire family (such as linear sRGB, sRGB, or linear XYZ depending on
arguments); name them after the general family (such as 'rgb'). Reserve specific
color space names for helpers that convert strictly to that single, fixed
destination space.

**Why.** Naming a color conversion helper after a specific color space when the
output is parameterized or depends on runtime inputs misleads callers into
assuming the result is always converted to that specific format. Using the
general family name accurately reflects the multi-space capability and prevents
callers from introducing redundant downstream conversions.

## Examples

Bad:

```cpp
// Bad: helper output varies based on colorSpace argument, but is named after a specific space 'srgb'.
$pure half4 $interpolated_to_srgb_unpremul(half4 color, int colorSpace, int doUnpremul) {
    ...
}
```

Good:

```cpp
// Good: named after the general 'rgb' family because the output space varies based on colorSpace.
$pure half4 $interpolated_to_rgb_unpremul(half4 color, int colorSpace, int doUnpremul) {
    ...
}
```
