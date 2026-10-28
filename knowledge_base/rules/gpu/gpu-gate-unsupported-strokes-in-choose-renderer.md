---
name: skia/gpu/gpu-gate-unsupported-strokes-in-choose-renderer
description: When a path renderer strategy lacks native stroke support in Graphite, gate selection in chooseRenderer() to fill styles rather than expanding strokes in drawGeometry
rubric: design
severity: should_fix
---

**Rule.** When integrating or updating a path renderer strategy that does not
yet support stroked geometry natively in Graphite, restrict its selection in
Device::chooseRenderer() to fill styles (e.g., via style.isFillStyle()) so
strokes fall back cleanly to standard path renderers. Do not perform ad-hoc CPU
stroke expansion (such as calling SkStrokeRec::applyToPath) inside
Device::drawGeometry to force-feed stroked paths to the renderer.

**Why.** Device::drawGeometry is a high-level draw dispatcher that should remain
simple and generic. Performing manual CPU stroke expansion in drawGeometry
clutters central dispatch, bypasses the renderer's dedicated generation
pipeline, and duplicates stroke-to-fill conversions that standard fallback
renderers already handle.

## Examples

Bad:

```cpp
// In Device::drawGeometry:
if (strategy == PathRendererStrategy::kCPUSparseStripsMSAA8) {
    if (style.needToApply()) {
        SkPathBuilder strokePath;
        style.applyToPath(&strokePath, geometry.shape().asPath());
        finalPath = strokePath.detach();
    }
}
```

Good:

```cpp
// In Device::chooseRenderer:
case PathRendererStrategy::kCPUSparseStripsMSAA8:
    if (style.isFillStyle()) {
        return {renderers->sparseStrips(), nullptr};
    }
    break; // Falls back to chooseMSAARenderer or standard stroke renderers

// In Device::drawGeometry:
SkASSERT(style.isFillStyle());
```
