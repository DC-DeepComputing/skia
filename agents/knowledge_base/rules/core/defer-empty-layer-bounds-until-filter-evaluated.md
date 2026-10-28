---
name: skia/core/defer-empty-layer-bounds-until-filter-evaluated
description: When computing layer bounds for saveLayer, do not early-return on empty content intersections before evaluating image filter input bounds
rubric: correctness
severity: should_fix
---

**Rule.** When calculating layer bounds for saveLayer or offscreen layer
allocation, do not early-return empty bounds when layer-space content bounds do
not intersect the target output if image filters are present. Defer empty-bounds
culling until after evaluating image filter input bounds via getInputBounds(),
or only early-return on empty content intersections when no image filters are
attached to the layer.

**Why.** Image filters such as blurs, offsets, or displacement maps can sample
source pixels from outside the target output rectangle and render them into the
visible output. Returning empty layer bounds prematurely before querying filter
input bounds incorrectly culls visible layer contents.

## Examples

Bad:

```cpp
skif::LayerSpace<SkIRect> knownBounds = mapping.paramToLayer(*contentBounds).roundOut();
if (!baseLayerBounds.intersect(knownBounds)) {
    // Premature early-return drops content that image filters could map into the target output.
    return {};
}
```

Good:

```cpp
skif::LayerSpace<SkIRect> knownBounds = mapping.paramToLayer(*contentBounds).roundOut();
if (!baseLayerBounds.intersect(knownBounds)) {
    baseLayerBounds = skif::LayerSpace<SkIRect>::Empty();
}
...
if (!filters.empty()) {
    layerBounds = evaluate_filter_bounds(filters, mapping, targetOutput, contentBounds);
} else {
    if (baseLayerBounds.isEmpty()) {
        return {};
    }
    layerBounds = baseLayerBounds;
}
```
