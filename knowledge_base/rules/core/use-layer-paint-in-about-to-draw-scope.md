---
name: skia/core/use-layer-paint-in-about-to-draw-scope
description: When executing draws or specialized fast paths in SkCanvas, operate within an aboutToDraw() scope using layer->paint(), and preserve AutoLayerForImageFilter via addMaskFilterLayer on fallback
rubric: correctness
severity: should_fix
---

**Rule.** In SkCanvas draw methods, execute all device rendering—including
specialized or analytic fast-paths—within an aboutToDraw() layer scope (passing
PredrawFlags::kSkipMaskFilterAutoLayer when deferring mask filter evaluation).
Inside the layer scope, dispatch draws directly to this->topDevice()->draw*()
using layer->paint() instead of re-entering SkCanvas onDraw* virtuals or using
the original SkPaint, updating the device transform via
topDevice()->setLocalToDevice() if needed. If a specialized fast-path fails, do
not discard and re-instantiate AutoLayerForImageFilter; preserve the existing
layer and augment it via layer->addMaskFilterLayer() for fallback drawing.

**Why.** aboutToDraw() sets up an AutoLayerForImageFilter with a temporary
SkDevice on the canvas stack to evaluate paint image filters and dispatch
pre-draw overwrite notifications; bypassing it for specialized draws skips image
filters. Calling SkCanvas onDraw* virtuals inside the layer scope re-enters
dispatch and triggers redundant notifications, while passing the original paint
bypasses filter adjustments. Letting AutoLayerForImageFilter destruct on
fast-path failure prematurely restores canvas state and evaluates image filters
on an empty layer, while constructing a replacement layer results in an invalid
layer stack.

## Examples

Bad:

```cpp
// Bad: calling specialized device draw before aboutToDraw bypasses image filters
if (this->topDevice()->drawRectBlurMaskFilter(r, paint)) {
    return;
}
auto layer = this->aboutToDraw(paint, &r);
if (layer) {
    // Bad: re-enters canvas virtual dispatch and uses original paint
    this->onDrawEdgeAAQuad(r, clip, aaFlags, paint.getColor4f(), SkBlendMode::kSrcOver);
}
// Bad: discarding layer on fast-path failure restores canvas and filters an empty layer
if (this->topDevice()->drawBlurredRRect(rrect, layer->paint(), sigma)) {
    return std::nullopt;
}
return std::optional<AutoLayerForImageFilter>(
        std::in_place, this, paint, &bounds, /*skipMaskFilterLayer=*/false);
```

Good:

```cpp
// Good: specialized draw runs inside aboutToDraw with kSkipMaskFilterAutoLayer
auto layer = this->aboutToDraw(paint, &bounds, flags | PredrawFlags::kSkipMaskFilterAutoLayer);
if (layer && this->topDevice()->drawBlurredRRect(rrect, layer->paint(), sigma)) {
    return std::nullopt; // Analytic draw succeeded
}
// Good: on fallback, augment existing layer instead of re-instantiating
layer->addMaskFilterLayer(&bounds);
// Good: general draw dispatches directly to topDevice using layer->paint()
this->topDevice()->drawEdgeAAQuad(r, clip, aaFlags, layer->paint().getColor4f(), SkBlendMode::kSrcOver);
return layer;
```
