---
name: skia/gpu/subtract-integer-translation-for-subpixel-cached-masks
description: When rasterizing mask textures or atlas entries cached by fractional subpixel offsets, subtract the integer translation from draw transforms to align with mask bounds
rubric: correctness
severity: should_fix
---

**Rule.** When rasterizing shape or clip masks into texture atlas entries or
mask buffers cached by subpixel/fractional offsets, ensure the rasterization
pass subtracts the base integer translation (such as `-bounds.topLeft()`) from
the draw transforms. Do not render shapes using unadjusted device-space
transforms, which misplaces or clips geometry inside the atlas plot.

**Why.** Mask cache keys that discard integer translation allow masks to be
reused across integer device coordinates. If the base integer offset is not
subtracted before rasterization, geometry is rendered at absolute device
coordinates rather than relative to the mask's local origin, producing misplaced
or clipped masks.

## Examples

Bad:

```cpp
// Transforms retain absolute device coordinates despite subpixel-only key
SkIRect iBounds = bounds.roundOut();
RasterMaskHelper helper(pixmap);
for (const auto& element : elements) {
    helper.drawClip(element.fShape, element.fLocalToDevice, alpha);
}
```

Good:

```cpp
// Offset rasterization by -iBounds.topLeft() so shapes draw relative to mask origin
SkIRect iBounds = bounds.roundOut();
SkIVector translate = -iBounds.topLeft() + SkIVector{kEntryPadding, kEntryPadding};
RasterMaskHelper helper(pixmap, translate);
for (const auto& element : elements) {
    helper.drawClip(element.fShape, element.fLocalToDevice, alpha);
}
```
