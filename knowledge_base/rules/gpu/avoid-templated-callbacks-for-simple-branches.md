---
name: skia/gpu/avoid-templated-callbacks-for-simple-branches
description: When code paths differ only by a simple condition or capability check, branch directly inside the function rather than introducing templated callback abstractions
rubric: design
severity: should_fix
---

**Rule.** Do not introduce templated callback abstractions, higher-order
functions, or wrapper methods solely to parameterize a simple conditional branch
or capability check between callers. Place the conditional check directly inside
the shared function and invoke the appropriate operations inline.

**Why.** Using templated function parameters (such as `Fn&&`) and caller
wrappers to parameterize a two-way branch introduces unnecessary template
instantiation overhead, indirection, and code complexity. Inlining the
conditional check keeps control flow straightforward and easier to read and
debug.

## Examples

Bad:

```cpp
template <typename Fn>
void RawElement::handleClipDraw(Device* device, Fn&& onDraw) {
    // common setup...
    onDraw(drawBounds, scissor, order);
}

void RawElement::drawClip(Device* device) {
    if (device->caps()->useDrawListLayer()) {
        this->updateDeferredClipDraw(device); // calls handleClipDraw with capture lambda
    } else {
        this->handleClipDraw(device, [&](...) { device->drawClipShape(...); });
    }
}
```

Good:

```cpp
void RawElement::drawClip(Device* device) {
    // common setup...
    if (device->caps()->useDrawListLayer()) {
        fCaptureParams->fOrder = order;
        fCaptureParams->fDrawBounds = drawBounds;
        fCaptureParams->fScissor = scissor.asSkIRect();
        device->updateNextDepthForClipping(fCaptureParams->fOrder.depth());
        fCaptureParams = nullptr;
        fDeferredLayer = nullptr;
    } else {
        device->drawClipShape(fLocalToDevice, fShape, ...);
    }
}
```
