---
name: skia/api/document-layer-color-space-filter-semantics
description: When documenting layer or intermediate surface configurations that specify a custom color space, explicitly document that backdrop and restore filters execute in the layer color space
rubric: readability
severity: nit
---

**Rule.** When an API or configuration struct allows specifying an independent
color space for a layer or intermediate surface (such as
SkCanvas::SaveLayerRec::fColorSpace), explicitly document which color space
backdrop filters and layer restore filters operate in. Clarify in doc comments
that filtering evaluates in the layer's configured color space rather than the
parent canvas or destination device color space.

**Why.** In Skia, rendering and filtering operate in the destination device's
color space. Because the backdrop filter's destination is the newly created
layer, the filter runs in the layer's color space before drawing or color space
conversion occurs. Callers might reasonably assume filters execute either in the
canvas's source color space or after layer restoration; documenting the exact
evaluation stage and color space prevents incorrect rendering expectations and
misconfigured filter graphs.

## Examples

Bad:

```cpp
/**
 * If not null, this triggers a color space conversion when the layer is restored. It
 * will be as if the layer's contents are drawn in this color space.
 */
const SkColorSpace* fColorSpace = nullptr;
```

Good:

```cpp
/**
 * If not null, this triggers a color space conversion when the layer is restored. It
 * will be as if the layer's contents are drawn in this color space. Filters from
 * fBackdrop and fPaint will be applied in this color space.
 */
const SkColorSpace* fColorSpace = nullptr;
```
