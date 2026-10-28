---
name: skia/core/pass-dither-to-shader-filter-not-restore-paint
description: 'When decomposing an SkPaint into coverage and restore layers, treat dithering as shading: disable it on coverage paints and route to restorePaint or SkImageFilters::Shader'
rubric: correctness
severity: should_fix
---

**Rule.** Treat dithering as a shading effect rather than a geometry coverage or
layer restoration property. When decomposing an SkPaint into an intermediate
coverage/alpha layer and a restore pass, disable dithering on the coverage paint
(setDither(false)). On the restoration pass, set
restorePaint.setDither(paint.isDither()) only when shading is executed during
layer restoration; if shading is delegated to an image filter DAG, pass
dithering directly into SkImageFilters::Shader (as
SkImageFilters::Dither::kYes/kNo) and omit setDither() from the restore paint.

**Why.** Dithering distributes color quantization error across smooth
transitions when generating pixels from shaders during rasterization; it is not
a geometry anti-aliasing property or a general layer compositing operation.
Leaving dithering enabled on coverage passes produces redundant or unintended
noise, while failing to route dithering to the stage that actually evaluates
shaders causes banding.

## Examples

Bad:

```cpp
// Bad: Leaving dithering on coverage paint, or unconditionally setting it on restorePaint
SkPaint restorePaint;
if (!appliesShading) {
    restorePaint.setColor4f(fPaint.getColor4f());
    restorePaint.setShader(fPaint.refShader());
}
restorePaint.setBlender(fPaint.refBlender());
restorePaint.setDither(fPaint.isDither()); // Bad if filter handles shading
restorePaint.setImageFilter(maskFilterAsImageFilter);

fPaint.setColor4f(SkColors::kWhite);
fPaint.setShader(nullptr);
// Bad: Missing fPaint.setDither(false) on coverage-only rasterization pass
```

Good:

```cpp
// Good: Disable dithering on coverage paint; route to restorePaint or shader filter based on shading stage
SkPaint restorePaint;
if (!appliesShading) {
    restorePaint.setColor4f(fPaint.getColor4f());
    restorePaint.setShader(fPaint.refShader());
    restorePaint.setDither(fPaint.isDither());
}
restorePaint.setBlender(fPaint.refBlender());
restorePaint.setImageFilter(maskFilterAsImageFilter);

fPaint.setColor4f(SkColors::kWhite);
fPaint.setShader(nullptr);
fPaint.setDither(false);
```
