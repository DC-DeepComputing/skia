---
name: skia/modules/clear-canvas-for-luma-filter-tests
description: When writing visual GM tests for SkLumaColorFilter or MakeLuma, clear the canvas with an opaque contrasting color before drawing
rubric: testing
severity: should_fix
---

**Rule.** When authoring visual Golden Master (GM) tests that exercise
SkLumaColorFilter or CanvasKit's ColorFilter.MakeLuma(), explicitly clear the
canvas with a contrasting opaque background (such as
canvas.clear(CanvasKit.WHITE)) before drawing. Do not draw luma-filtered shapes
directly onto an uninitialized or transparent surface.

**Why.** SkLumaColorFilter zeros out RGB channels and maps calculated luminance
into the alpha channel: {0, 0, 0, a * luma(r,g,b)}. Rendering luma-filtered
geometry directly onto an uninitialized or transparent canvas produces
translucent black over transparent pixels, yielding invisible or ineffective
visual baselines in Skia Gold.

## Examples

Bad:

```cpp
gm('luma_filter', (canvas) => {
    const paint = new CanvasKit.Paint();
    paint.setAntiAlias(true);
    const lumaCF = CanvasKit.ColorFilter.MakeLuma();
    paint.setColor(CanvasKit.BLUE);
    paint.setColorFilter(lumaCF);
    canvas.drawCircle(256, 256, 256, paint);
    paint.delete();
    lumaCF.delete();
});
```

Good:

```cpp
gm('luma_filter', (canvas) => {
    canvas.clear(CanvasKit.WHITE);
    const paint = new CanvasKit.Paint();
    paint.setAntiAlias(true);
    const lumaCF = CanvasKit.ColorFilter.MakeLuma();
    paint.setColor(CanvasKit.BLUE);
    paint.setColorFilter(lumaCF);
    canvas.drawCircle(256, 256, 256, paint);
    paint.delete();
    lumaCF.delete();
});
```
