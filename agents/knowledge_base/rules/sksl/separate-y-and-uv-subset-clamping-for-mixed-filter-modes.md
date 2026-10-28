---
name: skia/sksl/separate-y-and-uv-subset-clamping-for-mixed-filter-modes
description: When clamping planar YUV texture coordinates with mixed filter modes in shaders, clamp UV planes to un-snapped subset bounds rather than Y's integer-snapped bounds
rubric: correctness
severity: should_fix
---

**Rule.** When implementing planar YUV image sampling shaders with mixed filter
modes (such as nearest-neighbor for the Y plane and linear for UV planes), clamp
UV coordinates against the original floating-point subset bounds inset by linear
filter margins, rather than sharing the Y plane's integer-snapped subset rect.
Preserve the unsnapped subset bounds for UV plane clamping before snapping the Y
subset to texel boundaries.

**Why.** Nearest-neighbor Y filtering requires snapping coordinates to texel
centers and snapping subset bounds to integer texel boundaries (floor of
top-left, ceil of bottom-right) to avoid sampling outside the subset.
Linear-filtered UV planes interpolate across fractional coordinates, so clamping
UV coordinates to integer-snapped bounds distorts edge filtering and causes
visible color discrepancies along subset boundaries.

## Examples

Bad:

```cpp
float2 uvCoords = linearFilterUVInset.x < 0 ? floor(coords) + 0.5 : coords;
if (linearFilterUVInset.y < 0) {
    linearFilterUVInset = abs(linearFilterUVInset);
    // Anti-pattern: UV coords clamped to integer-snapped subset bounds
    uvCoords = clamp(coords, subset.xy + linearFilterUVInset, subset.zw - linearFilterUVInset);
    coords = clamp(coords, subset.xy + $kLinearInset, subset.zw - $kLinearInset);
}
```

Good:

```cpp
float4 subsetUV = subset;
if (linearFilterUVInset.x < 0) {
    coords = floor(coords) + 0.5;
    subset = float4(floor(subset.xy), ceil(subset.zw));
}
float2 coordsUV = coords;
if (linearFilterUVInset.y < 0) {
    linearFilterUVInset = abs(linearFilterUVInset);
    // Preserve unsnapped subset bounds with linear filter insets for UV sampling
    coordsUV = clamp(coords, subsetUV.xy + linearFilterUVInset, subsetUV.zw - linearFilterUVInset);
    coords = clamp(coords, subset.xy + $kLinearInset, subset.zw - $kLinearInset);
}
```
