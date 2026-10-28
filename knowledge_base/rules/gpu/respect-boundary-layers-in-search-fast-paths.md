---
name: skia/gpu/respect-boundary-layers-in-search-fast-paths
description: When implementing fast-path optimizations in bounded layer or draw searches, target the bounding stop element's successor rather than unconditionally selecting the list head
rubric: correctness
severity: should_fix
---

**Rule.** When optimizing bounded layer or draw searches with a fast path,
ensure candidate target layers respect any bounding stop layer or limit. Do not
unconditionally select the head of the layer list (fLayers.head()), which may
precede the stop boundary and violate draw order constraints; select the
boundary's successor (stopLayer ? stopLayer->fNext : fLayers.head()) instead.

**Why.** Unconditionally selecting the container head element bypasses ordering
boundaries established by earlier clip or depth dependencies. When a stop
boundary is present, draws or bindings inserted at the head would execute before
the boundary draw, violating dependencies and producing incorrect rendering.

## Examples

Bad:

```cpp
if (!isStencil && !dependsOnDst) {
    // Violates ordering constraints if head precedes stopLayer.
    targetLayer = fLayers.head();
    if (targetLayer) {
        targetMatch = targetLayer->searchBinding(key);
    }
    current = const_cast<Layer*>(stopLayer);
}
```

Good:

```cpp
if (!isStencil && !dependsOnDst) {
    // Respects stopLayer boundary before falling back to head.
    targetLayer = stopLayer ? stopLayer->fNext : fLayers.head();
    if (targetLayer) {
        targetMatch = targetLayer->searchBinding(key);
    }
    current = const_cast<Layer*>(stopLayer);
}
```
