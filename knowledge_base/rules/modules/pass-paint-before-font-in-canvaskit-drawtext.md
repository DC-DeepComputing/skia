---
name: skia/modules/pass-paint-before-font-in-canvaskit-drawtext
description: When calling Canvas.drawText in CanvasKit JavaScript or TypeScript code, pass Paint before Font rather than following C++ SkCanvas argument order
rubric: correctness
severity: should_fix
---

**Rule.** When calling Canvas.drawText(str, x, y, paint, font) in CanvasKit
JavaScript or TypeScript, pass the Paint object before the Font object. Do not
pass the Font before the Paint, as CanvasKit reverses the C++ SkCanvas text
drawing parameter ordering.

**Why.** In Skia's C++ API (such as SkCanvas::drawString or drawSimpleText),
SkFont precedes SkPaint. In CanvasKit's JavaScript binding, Canvas.drawText
expects (str, x, y, paint, font); swapping them passes a Font where a Paint is
expected, causing Embind type conversion errors or runtime rendering failures.

## Examples

Bad:

```cpp
// C++ SkCanvas order (font before paint) causes runtime errors in CanvasKit
canvas.drawText('Hello World', 10, 50, font, paint);
```

Good:

```cpp
// CanvasKit Canvas.drawText expects paint before font
canvas.drawText('Hello World', 10, 50, paint, font);
```
