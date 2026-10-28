---
name: skia/gpu/avoid-mutating-invariant-draw-parameters
description: When updating captured draw parameters in deferred passes, do not reassign invariant transform or geometry fields; update only dynamic properties like scissor, bounds, and order
rubric: design
severity: should_fix
---

**Rule.** When updating captured draw parameters in deferred clip or draw
recording passes (such as ClipStack or DrawListLayer), do not reassign or mutate
invariant draw parameters such as transforms, geometry, or unclipped shape
bounds. Initialize invariant parameters once during initial draw recording, and
restrict deferred update passes strictly to properties that depend on
accumulated draw usage (such as scissor rects, aggregated draw bounds, and
painter's depth/order).

**Why.** Static draw properties like geometry and coordinate transforms are
fixed once a draw command is recorded. Reassigning or recomputing them during
deferred passes violates lifecycle separation, introduces redundant state
mutations, and risks clobbering transformed state or creating inconsistencies
with the pre-recorded draw command.

## Examples

Bad:

```cpp
// In deferred clip update pass:
fCaptureParams->fTransform = transform;
fCaptureParams->fGeometry = geom;
fCaptureParams->fTransformedShapeBounds = fOuterBounds;
fCaptureParams->fOrder = order;
fCaptureParams->fDrawBounds = drawBounds;
fCaptureParams->fScissor = scissor.asSkIRect();
```

Good:

```cpp
// In deferred clip update pass, only update properties depending on accumulated usage:
fCaptureParams->fOrder = order;
fCaptureParams->fDrawBounds = drawBounds;
fCaptureParams->fScissor = scissor.asSkIRect();
device->updateNextDepthForClipping(fCaptureParams->fOrder.depth());
```
