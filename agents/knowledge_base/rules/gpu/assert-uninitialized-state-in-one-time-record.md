---
name: skia/gpu/assert-uninitialized-state-in-one-time-record
description: When defining one-time recording or draw methods that lazily populate cached member state, assert with SkASSERT that members are null on entry
rubric: readability
severity: should_fix
---

**Rule.** In one-time recording or draw methods that lazily populate member
pointers or cached state (such as clip layers, captured parameters, or draw
bounds), assert with SkASSERT on method entry that the target member is null or
uninitialized (e.g., SkASSERT(!fMember)), and document the one-time recording
invariant in a comment. Verify the member is properly populated after recording.

**Why.** Asserting that cached member state is null prior to lazy recording or
drawing enforces that operations execute only once per element, preventing
accidental state overwrites or duplicate draw emission. It also clearly
documents for callers and reviewers that the element has not been previously
processed.

## Examples

Bad:

```cpp
Layer* ClipStack::RawElement::drawClipImmediate(Device* device) {
    SkASSERT(!fShape.isEmpty());
    // Missing assertion that this element has not already been drawn
    std::tie(fCaptureParams, fDepthLayer) = device->drawClipShapeImmediate(...);
    return fDepthLayer;
}
```

Good:

```cpp
Layer* ClipStack::RawElement::drawClipImmediate(Device* device) {
    SkASSERT(!fShape.isEmpty());
    // We shouldn't be drawing if we already drew this clip element
    SkASSERT(!fDepthLayer);
    SkASSERT(!fCaptureParams);

    std::tie(fCaptureParams, fDepthLayer) = device->drawClipShapeImmediate(...);
    SkASSERT(fDepthLayer);
    return fDepthLayer;
}
```
