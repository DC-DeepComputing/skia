---
name: skia/modules/use-drawpaint-for-procedural-shaders
description: In CanvasKit rendering tests for source-independent shaders or image filters, use canvas.drawPaint or drawRect instead of loading image assets
rubric: readability
severity: should_fix
---

**Rule.** In CanvasKit tests (`modules/canvaskit/tests/`), when testing shaders,
runtime effects, or image filters (such as `ImageFilter.MakeShader`) that
generate procedural or constant color output without sampling source image
pixels, draw using `canvas.drawPaint(paint)` or `canvas.drawRect(...)` rather
than fetching and decoding image assets with `MakeImageFromEncoded` and
`canvas.drawImage(...)`.

**Why.** Fetching and decoding external image assets adds unnecessary
asynchronous boilerplate, test runtime overhead, and potential I/O failure
points when the shader or filter produces output independently of any source
bitmap. Using `canvas.drawPaint(paint)` or `canvas.drawRect(...)` keeps visual
tests minimal, self-contained, and focused strictly on the shader logic.

## Examples

Bad:

```cpp
gm('ImageFilter_MakeShader', (canvas, fetchedByteBuffers) => {
    const img = CanvasKit.MakeImageFromEncoded(fetchedByteBuffers[0]);
    const rt = CanvasKit.RuntimeEffect.Make('...');
    const shader = rt.makeShader([0.0, 0.0, 1.0, 0.5]);
    const filter = CanvasKit.ImageFilter.MakeShader(shader);
    const paint = new CanvasKit.Paint();
    paint.setImageFilter(filter);
    canvas.drawImage(img, 50, 50, paint);
    img.delete();
    paint.delete();
});
```

Good:

```cpp
gm('ImageFilter_MakeShader', (canvas) => {
    const rt = CanvasKit.RuntimeEffect.Make('...');
    const shader = rt.makeShader([0.0, 0.0, 1.0, 0.5]);
    const filter = CanvasKit.ImageFilter.MakeShader(shader);
    const paint = new CanvasKit.Paint();
    paint.setImageFilter(filter);
    canvas.drawPaint(paint);
    paint.delete();
});
```
