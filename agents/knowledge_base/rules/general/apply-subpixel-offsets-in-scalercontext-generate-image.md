---
name: skia/apply-subpixel-offsets-in-scalercontext-generate-image
description: When rasterizing glyph images in SkScalerContext::generateImage, translate the canvas by glyph subpixel offsets matching generateMetrics bounds adjustments
rubric: correctness
severity: should_fix
---

**Rule.** When rasterizing glyphs into an image buffer or canvas in
SkScalerContext::generateImage, translate by the subpixel offset
(SkFixedToScalar(glyph.getSubXFixed()), SkFixedToScalar(glyph.getSubYFixed()))
in addition to (-glyph.left(), -glyph.top()) whenever generateMetrics offsets
glyph bounds by subpixel positioning. Do not omit the subpixel translation when
aligning the canvas to glyph origin bounds.

**Why.** If generateMetrics() offsets glyph bounds by fractional subpixel
positions before rounding out to integer coordinates, rasterization must apply
the identical subpixel offset. Omitting it causes the glyph image to render
snapped to integer pixel boundaries instead of the requested subpixel position,
producing visual jitter and alignment discrepancies.

## Examples

Bad:

```cpp
canvas->translate(-glyph.left(), -glyph.top());
canvas->drawDrawable(rec.fDrawable.get(), &matrix);
```

Good:

```cpp
canvas->translate(-glyph.left(), -glyph.top());
canvas->translate(SkFixedToScalar(glyph.getSubXFixed()),
                  SkFixedToScalar(glyph.getSubYFixed()));
canvas->drawDrawable(rec.fDrawable.get(), &matrix);
```
