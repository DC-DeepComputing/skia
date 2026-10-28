---
name: skia/gpu/evaluate-clip-elements-independently
description: When aggregating clip dependencies across clip elements, evaluate elements independently and compare monotonic layer indices (layer->fIndex) instead of threading state sequentially
rubric: correctness
severity: should_fix
---

**Rule.** When aggregating clip dependencies for a draw across multiple clip
elements, do not pass state or target layer pointers sequentially from one
element to the next, and do not assume iteration order matches recording
recency. Evaluate each clip element independently from a neutral baseline and
determine the latest layer or order by explicitly comparing monotonic sequence
indices (such as layer->fIndex).

**Why.** Clip elements in a stack are independent of one another and are
activated or updated out of stack order depending on draw bounds. Threading
state sequentially or assuming iteration order matches recency causes ordering
inversions; comparing monotonic layer indices ensures the dependent draw orders
after the latest affecting layer.

## Examples

Bad:

```cpp
Layer* prevLayer = nullptr;
for (int i = 0; i < effectiveElements.size(); ++i) {
    const RawElement* e = static_cast<const RawElement*>(effectiveElements[i]);
    // Passing prevLayer sequentially assumes element iteration order matches layer creation order.
    CompressedPaintersOrder order = const_cast<RawElement*>(e)->updateForDraw(
            device, boundsManager, deviceBounds, clipBounds, z, prevLayer);
    maxClipOrder = std::max(order, maxClipOrder);
}
return {maxClipOrder, prevLayer};
```

Good:

```cpp
Layer* latestInsertedLayer = nullptr;
for (int i = 0; i < effectiveElements.size(); ++i) {
    const RawElement* e = static_cast<const RawElement*>(effectiveElements[i]);
    auto [order, insertedLayer] = const_cast<RawElement*>(e)->updateForDraw(
            device, boundsManager, deviceBounds, clipBounds, z);
    maxClipOrder = std::max(order, maxClipOrder);
    if (!latestInsertedLayer || (insertedLayer && insertedLayer->fIndex > latestInsertedLayer->fIndex)) {
        latestInsertedLayer = insertedLayer;
    }
}
return {maxClipOrder, latestInsertedLayer};
```
