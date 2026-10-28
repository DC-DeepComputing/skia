---
name: skia/modules/keep-canvas-layer-management-in-render-context
description: In SVG rendering, keep unmatched canvas saveLayer calls within the context object that restores them rather than delegating layer pushes to child element methods
rubric: design
severity: nit
---

**Rule.** Keep unmatched canvas save() and saveLayer() calls inside the scope or
context object (such as SkSVGRenderContext) responsible for restoring them,
rather than delegating layer pushes to child element or helper methods. Keep
child element methods self-contained regarding canvas state or expose query
helpers (such as bounds()) so the coordinating context manages layer lifecycles.

**Why.** When an outer render context manages canvas layer lifecycles (such as
restoring canvas depth in its destructor via restoreToCount()), having child
element methods push unmatched layers obscures state ownership and makes
balancing difficult to trace. Co-locating layer pushes with the context that
restores them clarifies ownership and keeps child element render methods modular
and self-contained.

## Examples

Bad:

```cpp
// In SkSVGMask:
void SkSVGMask::apply(SkSVGRenderContext* ctx) const {
    const auto bounds = ctx->resolveOBBRect(fX, fY, fWidth, fHeight, fMaskUnits);
    ctx->canvas()->saveLayer(bounds, nullptr);
    ctx->canvas()->clipRect(bounds, true);
    // ... render mask content ...
    // Unbalanced layers left to be restored by SkSVGRenderContext's destructor
    ctx->canvas()->saveLayer(bounds, &masking_paint);
}
```

Good:

```cpp
// In SkSVGMask:
SkRect SkSVGMask::bounds(const SkSVGRenderContext& ctx) const {
    return ctx.resolveOBBRect(fX, fY, fWidth, fHeight, fMaskUnits);
}
void SkSVGMask::renderMask(const SkSVGRenderContext& ctx) const {
    SkSVGRenderContext localCtx(ctx);
    // ... render mask content in a self-contained local context ...
}

// In SkSVGRenderContext:
void SkSVGRenderContext::applyMask(const SkSVGFuncIRI& mask) {
    fCanvas->saveLayer(mask_bounds, nullptr);
    fCanvas->clipRect(mask_bounds, true);
    mask_node->renderMask(*this);
    fCanvas->saveLayer(mask_bounds, &masking_paint);
    // Balanced in SkSVGRenderContext destructor via restoreToCount()
}
```
