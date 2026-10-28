---
name: skia/core-pass-unexpanded-bounds-to-savelayer
description: When isolating filtered content with SkCanvas::saveLayer, pass unexpanded content bounds rather than inflating the bounds hint with SkImageFilter::filterBounds
rubric: correctness
severity: should_fix
---

**Rule.** When calling SkCanvas::saveLayer or constructing
SkCanvas::SaveLayerRec with an SkImageFilter, pass the unexpanded bounds of the
content drawn into the layer. Do not expand or map the layer bounds hint using
SkImageFilter::filterBounds().

**Why.** The bounds hint passed to saveLayer defines the geometric limit of
content drawn into the layer before filtering, which the backend uses to
allocate the offscreen buffer. Skia's image filter pipeline automatically
handles expanding the output destination geometry; manually inflating saveLayer
bounds allocates an unnecessarily large offscreen layer and causes edge-clamping
artifacts when filters evaluate tiling.

## Examples

Bad:

```cpp
const auto filter_bounds =
        filter->filterBounds(bounds.roundOut(), ctm, SkImageFilter::kForward_MapDirection);
canvas->saveLayer(SkRect::Make(filter_bounds), &layer_paint);
```

Good:

```cpp
canvas->saveLayer(bounds, &layer_paint);
```
