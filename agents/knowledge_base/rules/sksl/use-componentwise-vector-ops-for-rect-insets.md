---
name: skia/sksl/use-componentwise-vector-ops-for-rect-insets
description: When insetting or offsetting 2D bounding rects in SkSL or GPU shaders by a 2D vector, use component-wise vector arithmetic rather than adding scalar components
rubric: correctness
severity: should_fix
---

**Rule.** When insetting, expanding, or offsetting a 2D bounding rect
(represented as a float4 or half4 in LTRB order) by a 2D vector (such as a
float2 inset) in SkSL or GPU shaders, perform component-wise vector operations
(rect.xy + inset and rect.zw - inset) rather than adding or subtracting scalar
components. Do not write rect.xy + inset.x or rect.zw - inset.y.

**Why.** In SkSL and GLSL, adding or subtracting a scalar to or from a 2D vector
broadcasts the scalar across both components. Adding inset.x to rect.xy
mistakenly insets both left and top by the horizontal inset, and subtracting
inset.y from rect.zw insets both right and bottom by the vertical inset. Using
component-wise vector arithmetic pairs horizontal and vertical insets with their
corresponding axes correctly.

## Examples

Bad:

```cpp
// Bad: Adding scalar components broadcasts across both dimensions of the float2 vector
float4 insetClamp = float4(subset.xy + linearFilterInset.x,
                           subset.zw - linearFilterInset.y);
```

Good:

```cpp
// Good: Component-wise vector arithmetic correctly insets both X and Y coordinates
float4 insetClamp = float4(subset.xy + linearFilterInset,
                           subset.zw - linearFilterInset);
```
