---
name: skia/core/use-canvas-localtodevice-when-drawing-via-canvas
description: When pre-transforming geometry for operations that draw back through SkCanvas, query canvas->getLocalToDevice() rather than topDevice()->localToDevice() to preserve layer offsets
rubric: correctness
severity: should_fix
---

**Rule.** When computing transforms or pre-transforming geometry (such as with
makeTransform or transform) for draw operations dispatched through SkCanvas
(such as canvas->drawPath), obtain the transformation matrix using
canvas->getLocalToDevice() or canvas->getLocalToDeviceAs3x3(). Do not query
topDevice()->localToDevice() on SkCanvas or this->localToDevice() on SkDevice
when drawing back through canvas APIs.

**Why.** In the presence of nested layers (such as from saveLayer), the active
SkDevice's localToDevice() includes layer-relative origin and device offsets,
whereas SkCanvas::getLocalToDevice() reflects the canvas CTM. Drawing through
SkCanvas automatically applies the base-canvas-to-layer adjustment, so
transforming with the device's matrix causes geometry to be misaligned or
double-offsets the layer translation.

## Examples

Bad:

```cpp
// Bad: Querying device transform double-applies layer offsets when drawing via canvas
const SkMatrix& viewMatrix = this->topDevice()->localToDevice();
SkPath devSpacePath;
path.transform(viewMatrix, &devSpacePath);

SkAutoCanvasRestore autoRestore(this, /*doSave=*/true);
this->setMatrix(SkM44());
this->drawPath(devSpacePath, paint);
```

Good:

```cpp
// Good: Canvas getLocalToDevice correctly reflects CTM across nested layers
SkPath devSpacePath = path.makeTransform(canvas->getLocalToDeviceAs3x3());

SkAutoCanvasRestore autoRestore(canvas, /*doSave=*/true);
canvas->setMatrix(SkM44());
canvas->drawPath(devSpacePath, paint);
```
