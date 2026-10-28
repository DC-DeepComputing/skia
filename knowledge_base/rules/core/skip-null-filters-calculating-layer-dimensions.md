---
name: skia/core/skip-null-filters-calculating-layer-dimensions
description: When computing maximum layer dimensions or bounds for image filter collections, iterate over filters and skip null entries rather than transforming identity bounds
rubric: readability
severity: should_fix
---

**Rule.** When adjusting maximum layer dimensions or allocating bounds for a
collection of image filters, iterate through the filter list and skip null
entries rather than computing and unioning identity bounds. Only non-null image
filters can expand layer geometry beyond default bounds thresholds, so null
filters representing identity passes should be bypassed.

**Why.** Null image filter entries represent unfiltered identity content that
never requires layer dimensions larger than the default base layer. Skipping
null entries avoids unnecessary coordinate transformations, bounds calculations,
and rect union overhead for identity passes.

## Examples

Bad:

```cpp
skif::Mapping idealMapping{mapping.layerMatrix()};
auto idealLayerBounds = skif::LayerSpace<SkIRect>::Union(filters.size(), [&](int i) {
    if (filters[i]) {
        return as_IFB(filters[i])->getInputBounds(idealMapping, targetOutput, contentBounds);
    } else {
        return idealMapping.deviceToLayer(targetOutput);
    }
});
maxLayerDim = std::max(std::max(idealLayerBounds.width(), idealLayerBounds.height()), maxLayerDim);
```

Good:

```cpp
skif::Mapping idealMapping{mapping.layerMatrix()};
for (const sk_sp<SkImageFilter>& filter : filters) {
    if (filter) {
        auto idealLayerBounds = as_IFB(filter)->getInputBounds(
                idealMapping, targetOutput, contentBounds);
        maxLayerDim = std::max(std::max(idealLayerBounds.width(),
                                        idealLayerBounds.height()),
                               maxLayerDim);
    }
}
```
