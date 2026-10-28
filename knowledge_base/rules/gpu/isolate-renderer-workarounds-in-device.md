---
name: skia/gpu/isolate-renderer-workarounds-in-device
description: When adapting geometry for renderer limitations (such as perspective transforms with tessellation), perform transformations in Device draw methods rather than leaking renderer workarounds into ClipStack
rubric: design
severity: should_fix
---

**Rule.** When adapting geometry to satisfy backend renderer limitations (such
as pre-transforming perspective shapes to device coordinates for tessellating
path renderers), perform the geometric transformations within Device draw
recording methods where renderers are selected. Do not leak renderer-specific
capability checks or geometric workarounds into ClipStack or clip tracking
elements.

**Why.** ClipStack is responsible for managing geometric clipping hierarchy and
element bounds, whereas Device selects appropriate Renderers and interfaces with
DrawContext. Embedding renderer-specific constraints (such as tessellation
render steps asserting non-perspective transforms) inside ClipStack creates
layering violations and couples clip state management to low-level renderer
requirements.

## Examples

Bad:

```cpp
// Inside ClipStack.cpp (leaking renderer limitation workaround into clip state)
if (fLocalToDevice.type() == Transform::Type::kPerspective) {
    fCaptureParams->fGeometry = Geometry(
            Shape(fShape.asPath().makeTransform(fLocalToDevice.matrix().asM33())));
}
```

Good:

```cpp
// Inside Device.cpp (drawClipShapeImmediate / drawClipShape)
if (localToDevice.type() == Transform::Type::kPerspective) {
    SkPath devicePath = shape.asPath().makeTransform(localToDevice.matrix().asM33());
    return fDC->recordDraw(renderer, Transform::Identity(), Geometry(Shape(devicePath)),
                           clip, order, ...);
}
```
