---
name: skia/core/keep-empty-bounds-bailout-in-filter-union-fallback
description: When unioning layer bounds across multiple image filters with optional unfiltered content, pass clipped base bounds and keep empty-bounds early returns inside the no-filters fallback
rubric: correctness
severity: should_fix
---

**Rule.** When aggregating layer or input bounds across multiple image filters
where null entries represent unfiltered content, supply the clipped base layer
bounds for null filters and keep empty-bounds early returns inside the
no-filters fallback branch. Do not hoist empty-bounds early bailouts before
computing the union, as non-null filters may still produce valid bounds even
when base content bounds are empty.

**Why.** An empty base layer rect indicates only that unfiltered content was
clipped out, but other active image filters in the union can still generate
visible output or require non-empty input bounds. Aborting early on an empty
base rect skips valid filter processing, whereas unioning allows active filters
to contribute and correctly results in an empty layer only if all inputs are
empty.

## Examples

Bad:

```cpp
skif::LayerSpace<SkIRect> layerBounds;
if (!filters.empty()) {
    layerBounds = skif::LayerSpace<SkIRect>::Union(filters.size(), [&](int i) {
        if (filters[i]) {
            return as_IFB(filters[i])->getInputBounds(mapping, targetOutput, contentBounds);
        } else {
            return mapping.deviceToLayer(targetOutput);
        }
    });
} else {
    layerBounds = mapping.deviceToLayer(targetOutput);
    if (contentBounds && !layerBounds.intersect(knownBounds)) {
        return {};
    }
}
```

Good:

```cpp
auto baseLayerBounds = mapping.deviceToLayer(targetOutput);
if (contentBounds) {
    skif::LayerSpace<SkIRect> knownBounds = mapping.paramToLayer(*contentBounds).roundOut();
    if (!baseLayerBounds.intersect(knownBounds)) {
        baseLayerBounds = skif::LayerSpace<SkIRect>::Empty();
    }
}
skif::LayerSpace<SkIRect> layerBounds;
if (!filters.empty()) {
    layerBounds = skif::LayerSpace<SkIRect>::Union(filters.size(), [&](int i) {
        return filters[i] ? as_IFB(filters[i])->getInputBounds(mapping, targetOutput, contentBounds)
                          : baseLayerBounds;
    });
} else {
    if (baseLayerBounds.isEmpty()) {
        return {};
    }
    layerBounds = baseLayerBounds;
}
```
