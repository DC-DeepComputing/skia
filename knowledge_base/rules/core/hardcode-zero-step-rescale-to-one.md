---
name: skia/core/hardcode-zero-step-rescale-to-one
description: When downscale_step_count reports zero steps for an axis in image filter rescaling, explicitly set scale to 1.0f before centering scale
rubric: correctness
severity: should_fix
---

**Rule.** In skif image filter rescaling, when downscale_step_count() reports 0
steps for an axis, explicitly hardcode that axis's scale factor to 1.0f rather
than passing raw floating-point scale factors to scale_about_center() or
downstream operations.

**Why.** scale_about_center() performs exact floating-point equality checks (sx
== 1.f and sy == 1.f) to bypass centering offset calculations that would
otherwise introduce half-pixel shifts on odd-sized bounds. Because fuzzy
tolerance for near-identity scaling is encapsulated in downscale_step_count(),
calling code must explicitly pass 1.0f for 0-step axes.

## Examples

Bad:

```cpp
float finalScaleX = scale.width();
float finalScaleY = scale.height();
if (deferPeriodicTiling) {
    PixelSpace<SkRect> dstBoundsF = scale_about_center(stepBoundsF, finalScaleX, finalScaleY);
}
```

Good:

```cpp
float finalScaleX = xSteps > 0 ? scale.width() : 1.f;
float finalScaleY = ySteps > 0 ? scale.height() : 1.f;
if (deferPeriodicTiling) {
    PixelSpace<SkRect> dstBoundsF = scale_about_center(stepBoundsF, finalScaleX, finalScaleY);
}
```
