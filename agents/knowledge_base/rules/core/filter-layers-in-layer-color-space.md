---
name: skia/core/filter-layers-in-layer-color-space
description: When evaluating image filters for layers with custom color spaces—including empty-layer or bypass paths—pass explicit SkColorInfo rather than falling back to destination devices
rubric: correctness
severity: should_fix
---

**Rule.** When evaluating backdrop or restore image filters on layers that
specify custom color spaces, evaluate the filter in the layer's own color space.
Pass an explicit SkColorInfo for filter evaluation instead of deriving color
info from destination devices, and in layer-bypass or empty-layer optimization
paths (such as when layerBounds.isEmpty()), explicitly apply rec.fColorSpace to
the filter's SkColorInfo before invoking device filtering helpers.

**Why.** Destination devices often have a different color space than
intermediate layers when custom layer color spaces are configured. In both
standard layer rendering and empty-layer bypass optimizations, defaulting to
destination or prior device color info evaluates filters in the wrong color
space, causing incorrect color transformations, visual shifts, or improper
format selection.

## Examples

Bad:

```cpp
// Deriving filter color space from dst device assumes src and dst share color spaces:
sk_sp<SkColorSpace> filterColorSpace = dst->imageInfo().refColorSpace();
const SkColorType filterColorType = image_filter_color_type(dst->imageInfo().colorInfo());

// Empty layer bounds optimization falls back to priorDevice color space:
if (layerBounds.isEmpty()) {
    this->internalDrawDeviceWithFilter(/*src=*/nullptr, priorDevice, filters, restorePaint,
                                       DeviceCompatibleWithFilter::kUnknown);
    abortLayer();
    return;
}
```

Good:

```cpp
// Passing explicit filterColorInfo preserves layer color space during filtering:
sk_sp<SkColorSpace> filterColorSpace = filterColorInfo.refColorSpace();
const SkColorType filterColorType = image_filter_color_type(filterColorInfo);

// Explicitly apply rec.fColorSpace to filterColorInfo before filtering:
if (layerBounds.isEmpty()) {
    SkColorInfo filterColorInfo = priorDevice->imageInfo().colorInfo();
    if (rec.fColorSpace) {
        filterColorInfo = filterColorInfo.makeColorSpace(sk_ref_sp(rec.fColorSpace));
    }
    this->internalDrawDeviceWithFilter(/*src=*/nullptr, priorDevice, filters, restorePaint,
                                       DeviceCompatibleWithFilter::kUnknown, filterColorInfo);
    abortLayer();
    return;
}
```
