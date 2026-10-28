---
name: skia/core/dispatch-draw-before-paint-adaptation
description: When delegating canvas batch draw operations to drawImageRect, dispatch before invoking paint and geometry adaptation helpers to avoid double transformation.
rubric: correctness
severity: should_fix
---

**Rule.** When delegating canvas draw operations (such as in drawEdgeAAImageSet)
to high-level methods like drawImageRect, dispatch to them before applying paint
and destination geometry adaptations (such as
SkModifyPaintAndDstForDrawImageRect or SkImageShader::MakeForDrawRect). Pass the
original unmodified destination rect and paint to the delegated draw call, as
high-level entry points apply these transformations internally.

**Why.** High-level canvas drawing methods like drawImageRect already handle
paint shading and destination rect adaptation for mask filters or subset
constraints. Delegating to drawImageRect after transforming the destination rect
and paint results in corrupt paint setups and double-transformed destination
geometry.

## Examples

Bad:

```cpp
// Adapting destination rect and paint before delegating to drawImageRect
SkRect drawDst = SkModifyPaintAndDstForDrawImageRect(
        image.get(), sampling, srcRect, dstRect, strict, &paint);
if (canDelegateToDrawImageRect) {
    this->drawImageRect(image.get(), srcRect, drawDst, sampling, &paint, strict);
    return;
}
```

Good:

```cpp
// Delegate with original rect and paint before applying low-level adaptations
if (canDelegateToDrawImageRect) {
    this->drawImageRect(image.get(), srcRect, dstRect, sampling, &paint, strict);
    return;
}
SkRect drawDst = SkModifyPaintAndDstForDrawImageRect(
        image.get(), sampling, srcRect, dstRect, strict, &paint);
```
