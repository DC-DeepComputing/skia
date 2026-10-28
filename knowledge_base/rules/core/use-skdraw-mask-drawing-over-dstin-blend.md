---
name: skia/core/use-skdraw-mask-drawing-over-dstin-blend
description: When rendering coverage or alpha masks on CPU raster devices, use SkDraw mask-drawing routines rather than allocating intermediate bitmaps with SkBlendMode::kDstIn
rubric: safety
severity: should_fix
---

**Rule.** When rendering an alpha or coverage mask on CPU raster devices (such
as SkBitmapDevice), apply the mask directly to the paint via SkDraw mask-drawing
routines like drawBitmapAsMask or drawDevMask. Do not allocate a temporary
intermediate N32 bitmap to evaluate the paint and composite the mask using
SkBlendMode::kDstIn.

**Why.** Allocating temporary N32 bitmaps and performing dst-in compositing
introduces heap churn and requires multiple pixel passes. Skia's CPU blitters
natively support applying 8-bit alpha masks directly as coverage to the paint's
shader in a single rasterization pass.

## Examples

Bad:

```cpp
SkBitmap paintBM;
if (!paintBM.tryAllocN32Pixels(maskBM.width(), maskBM.height())) {
    return;
}
SkCanvas paintCanvas(paintBM);
paintCanvas.concat(localToMask);
paintCanvas.drawPaint(paint);

SkPaint blendPaint;
blendPaint.setBlendMode(SkBlendMode::kDstIn);
paintCanvas.drawImage(maskBM.asImage(), 0, 0, SkSamplingOptions{}, &blendPaint);

SkDraw draw;
if (this->accessPixels(&draw.fDst)) {
    draw.drawBitmap(paintBM, SkMatrix::I(), nullptr, sampling, paint, nullptr);
}
```

Good:

```cpp
SkDraw draw;
if (!this->accessPixels(&draw.fDst)) {
    return;
}
draw.fRC = &fRCStack.rc();
draw.fCTM = &maskToDevice;
draw.drawBitmapAsMask(maskBM, sampling, paint, &this->localToDevice());
```
