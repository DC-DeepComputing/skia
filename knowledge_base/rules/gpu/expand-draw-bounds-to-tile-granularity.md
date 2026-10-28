---
name: skia/gpu/expand-draw-bounds-to-tile-granularity
description: When computing draw or clip bounds for overlap tracking with tiled renderers, expand bounds to tile granularity to encompass all pixels touched by rasterization
rubric: correctness
severity: should_fix
---

**Rule.** Ensure draw and clip bounds used for bounds-overlap checks encompass
all pixels touched by rasterization, expanding bounds out to tile granularity
when renderers align geometry to tiles (unless rendering explicitly scissors or
clips out zero-coverage pixels). Do not rely solely on standard 1px
anti-aliasing outsets when a renderer touches full tiles beyond geometric shape
bounds.

**Why.** Graphite's bounds managers and DrawListLayer rely on clip draw bounds
to detect draw overlaps for depth assignment, batching, and layer ordering. If a
renderer aligns geometry to tile boundaries (such as SparseStrips end caps)
without scissoring transparent pixels, failing to expand clip bounds to tile
granularity causes bounds overlap tracking to miss touched pixels and produce
visual artifacts or depth conflicts.

## Examples

Bad:

```cpp
// Only accounts for standard 1px AA outsetting, missing tile-granularity alignment:
if (!renderer || renderer->outsetBoundsForAA()) {
    clip.outsetBoundsForAA();
}
```

Good:

```cpp
// Expand clip bounds to tile granularity when renderers align geometry to full tiles:
if (renderer && renderer->alignsToTiles()) {
    clip.expandToTileGranularity(renderer->tileSize());
} else if (!renderer || renderer->outsetBoundsForAA()) {
    clip.outsetBoundsForAA();
}
```
