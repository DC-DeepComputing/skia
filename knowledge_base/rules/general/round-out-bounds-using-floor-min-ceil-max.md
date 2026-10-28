---
name: skia/round-out-bounds-using-floor-min-ceil-max
description: When rounding out bounding rectangles or texture subsets to enclosing integer boundaries, round minimum coordinates down using floor and maximum coordinates up using ceil
rubric: correctness
severity: should_fix
---

**Rule.** When snapping or rounding out bounding rectangles or image/texture
subsets (such as float4(left, top, right, bottom) or .xy and .zw) to enclosing
integer texel or pixel boundaries, round minimum coordinates (left and top /
.xy) down using floor() and maximum coordinates (right and bottom / .zw) up
using ceil(). Do not use floor() across all four coordinates when computing
enclosing bounds.

**Why.** Using floor() on upper or maximum coordinates truncates or contracts
the bounding box inward rather than enclosing the full region, clipping edge
texels or sub-pixel geometry. Rounding minimums down with floor() and maximums
up with ceil() guarantees that all fractional content remains fully enclosed
within the resulting integer boundaries.

## Examples

Bad:

```cpp
// Bad: Using floor() on maximum coordinates (.zw) truncates the bounds inward
subset = float4(floor(subset.xy), floor(subset.zw));
```

Good:

```cpp
// Good: Round min down with floor() and max up with ceil() to enclose the subset
subset = float4(floor(subset.xy), ceil(subset.zw));
```
