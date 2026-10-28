---
name: skia/opts/factor-channel-math-in-raster-pipeline-stages
description: When defining families of raster pipeline stages operating across color channels or coordinate axes, factor identical math into lane-parameterized inline helpers or macros
rubric: design
severity: should_fix
---

**Rule.** When implementing families of raster pipeline stages or SIMD
operations that execute identical calculations across different color channels
(r, g, b, a) or coordinate axes (x, y), factor the common arithmetic and memory
access into a static inline helper function (`SI void`) or macro parameterized
by lane or channel. Do not duplicate identical multi-step math and store
operations across each stage variant.

**Why.** Duplicating multi-step arithmetic, bit manipulation, and memory storage
logic across sibling per-channel or per-lane stages creates boilerplate and
risks maintenance divergence when formulas or memory access patterns change.
Consolidating the core calculation into a parameterized helper keeps stage
definitions concise and ensures consistent behavior.

## Examples

Bad:

```cpp
LOWP_STAGE_GG(debug_x, const SkRasterPipelineContexts::MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx, dy);
    auto r2 = cast<U32>(abs_(x) / 256) & 0xFF;
    auto g2 = cast<U32>(abs_(x)) & 0xFF;
    auto b2 = cast<U32>(abs_(x) * 256) & 0xFF;
    auto a2 = cast<U32>(min(max(x * -256 * 256, 0), 0xFF));
    store(ptr, r2 | (g2 << 8) | (b2 << 16) | (a2 << 24));
}
LOWP_STAGE_GG(debug_y, const SkRasterPipelineContexts::MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx, dy);
    auto r2 = cast<U32>(abs_(y) / 256) & 0xFF;
    auto g2 = cast<U32>(abs_(y)) & 0xFF;
    auto b2 = cast<U32>(abs_(y) * 256) & 0xFF;
    auto a2 = cast<U32>(min(max(y * -256 * 256, 0), 0xFF));
    store(ptr, r2 | (g2 << 8) | (b2 << 16) | (a2 << 24));
}
```

Good:

```cpp
SI void lowp_fixed_point(const SkRasterPipelineContexts::MemoryCtx* ctx,
                         int dx, int dy, F lane) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx, dy);
    auto r2 = cast<U32>(abs_(lane) / 256) & 0xFF;
    auto g2 = cast<U32>(abs_(lane)) & 0xFF;
    auto b2 = cast<U32>(abs_(lane) * 256) & 0xFF;
    auto a2 = cast<U32>(min(max(lane * -256 * 256, 0), 0xFF));
    store(ptr, r2 | (g2 << 8) | (b2 << 16) | (a2 << 24));
}

LOWP_STAGE_GG(debug_x, const SkRasterPipelineContexts::MemoryCtx* ctx) {
    lowp_fixed_point(ctx, dx, dy, x);
}
LOWP_STAGE_GG(debug_y, const SkRasterPipelineContexts::MemoryCtx* ctx) {
    lowp_fixed_point(ctx, dx, dy, y);
}
```
