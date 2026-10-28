---
name: skia/avoid-backend-specific-bounds-in-skcanvas
description: When guarding draw operations against backend-specific finiteness or bounds issues, implement checks inside the target SkDevice rather than computing bounds in SkCanvas
rubric: safety
severity: should_fix
---

**Rule.** Do not add geometry bounds calculations or quick-reject checks to
SkCanvas draw methods to prevent backend-specific crashes or non-finite
assertions. Implement geometry bounds, matrix mapping, or finiteness checks
inside the specific SkDevice backend implementation that requires them, unless
SkCanvas requires bounds for filter evaluation.

**Why.** Computing geometry bounds in shared SkCanvas methods forces backends
that already perform post-transform clipping and bounds checking (such as CPU
rasterization in SkDraw) to calculate bounds twice. Confining backend-specific
guards to that backend's SkDevice avoids redundant overhead across other
rendering pipelines.

## Examples

Bad:

```cpp
// In SkCanvas::onDrawPoints, computing bounds for all backends:
SkRect bounds;
bounds.setBounds(pts, SkToInt(count));
if (this->internalQuickReject(bounds, strokePaint)) {
    return;
}
```

Good:

```cpp
// In SkCanvas::onDrawPoints, only compute bounds if required by filters:
if (paint.getImageFilter() || paint.getMaskFilter()) {
    auto bounds = SkRect::Bounds(pts);
    if (bounds && this->internalQuickReject(bounds.value(), strokePaint)) {
        return;
    }
}
// In src/gpu/ganesh/Device.cpp (drawPoints), guard GPU-specific finiteness:
auto bounds = SkRect::Bounds(pts);
if (!bounds || paint.nothingToDraw()) {
    return;
}
SkRect devBounds = SkMatrixPriv::MapRect(this->localToDevice44(), bounds.value());
if (!devBounds.isFinite()) {
    return;
}
```
