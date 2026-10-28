---
name: skia/gpu/assert-matching-draw-traits-in-batching
description: When batching draws into matched or newly created draw lists in Graphite, assert with SkASSERT that compile-time draw traits match runtime list properties
rubric: correctness
severity: should_fix
---

**Rule.** When grouping or batching draws into an existing matched draw or
binding list, or initializing a newly allocated list, assert with SkASSERT that
the incoming draw's static traits (such as kIsDepthOnly) match the list's
runtime configuration properties (such as fIsDepthOnly).

**Why.** Upstream key comparisons, caching, or list lookup bugs can
inadvertently return an incompatible draw list. Asserting that compile-time draw
characteristics match runtime list properties catches accidental mixing of
incompatible draw types (such as depth-only draws with color or shading draws)
in debug builds.

## Examples

Bad:

```cpp
if (match) {
    stencil = static_cast<StencilDrawList*>(match);
    stencil->fBounds.join(draw->fDrawParams->drawBounds());
    searchStart = (*startList) ? (*startList)->fNext : stencil->fStencilDraws.head();
} else {
    stencil = alloc->make<StencilDrawList>(BindingListType::kStencil, fListOrder, kIsDepthOnly);
    ...
    match = stencil;
}
// Missing assertion that matched stencil matches incoming draw's depth traits
```

Good:

```cpp
if (match) {
    stencil = static_cast<StencilDrawList*>(match);
    stencil->fBounds.join(draw->fDrawParams->drawBounds());
    searchStart = (*startList) ? (*startList)->fNext : stencil->fStencilDraws.head();
} else {
    stencil = alloc->make<StencilDrawList>(BindingListType::kStencil, fListOrder, kIsDepthOnly);
    ...
    match = stencil;
}
SkASSERT(kIsDepthOnly == stencil->fIsDepthOnly);
```
