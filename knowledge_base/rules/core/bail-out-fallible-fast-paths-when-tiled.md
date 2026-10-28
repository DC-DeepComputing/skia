---
name: skia/core/bail-out-fallible-fast-paths-when-tiled
description: When implementing fallible raster draw fast paths using SkDrawTiler, return false if needsTiling() is true rather than attempting multi-tile execution
rubric: correctness
severity: should_fix
---

**Rule.** When executing fallible raster draw fast paths (such as
drawBlurredRRect) that return a boolean status, check that tiling is not
required (e.g., !tiler.needsTiling()) before executing via SkDrawTiler, and bail
out by returning false if tiling is needed. Do not loop over multiple tiles for
operations that can fail mid-way, as partial execution across a subset of tiles
causes incomplete rendering or double-drawing upon fallback.

**Why.** Fallible fast paths rely on returning false so callers can fall back to
general rendering passes like mask filtering. If a fallible operation executes
tile-by-tile, some tiles may succeed while others fail, leaving partially
rendered content or causing pixels to be drawn twice when the caller falls back.

## Examples

Bad:

```cpp
bool success = false;
SkDrawTiler tiler(this, Bounder(rrect.getBounds(), paint));
while (const SkDraw* draw = tiler.next()) {
    success |= draw->drawRRectNinePatch(rrect, paint);
}
return success;
```

Good:

```cpp
SkDrawTiler tiler(this, Bounder(rrect.getBounds(), paint));
if (!tiler.needsTiling()) {
    if (const SkDraw* draw = tiler.next()) {
        return draw->drawRRectNinePatch(rrect, paint);
    }
}
return false;
```
