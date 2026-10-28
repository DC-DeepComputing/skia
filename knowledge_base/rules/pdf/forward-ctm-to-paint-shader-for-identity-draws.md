---
name: skia/pdf/forward-ctm-to-paint-shader-for-identity-draws
description: When rendering device-space geometry with an identity transform in device draw lowerings, wrap any existing paint shader with makeWithLocalMatrix(localToDevice())
rubric: correctness
severity: should_fix
---

**Rule.** When drawing geometry in device space using an identity matrix (such
as in device drawPaint implementations or inverse-clip rendering), check if the
SkPaint has an active SkShader and forward the device transform using
paint.setShader(paint.getShader()->makeWithLocalMatrix(this->localToDevice())).
Do not pass an unadjusted paint with an identity CTM to internal draw routines
when coordinate-dependent shaders may be present.

**Why.** Shaders such as gradients, image shaders, and runtime effects sample
coordinates in local canvas space mapped by the local-to-device CTM. Rendering
device-space geometry with an identity transform without pre-multiplying the
shader by localToDevice() causes the shader to sample in device coordinates,
distorting coordinate-dependent shading.

## Examples

Bad:

```cpp
SkPaint newPaint = srcPaint;
newPaint.setStyle(SkPaint::kFill_Style);
this->internalDrawPath(this->cs(), SkMatrix::I(), SkPath::Rect(bbox), newPaint);
```

Good:

```cpp
SkPaint newPaint = srcPaint;
newPaint.setStyle(SkPaint::kFill_Style);
if (newPaint.getShader()) {
    newPaint.setShader(newPaint.getShader()->makeWithLocalMatrix(this->localToDevice()));
}
this->internalDrawPath(this->cs(), SkMatrix::I(), SkPath::Rect(bbox), newPaint);
```
