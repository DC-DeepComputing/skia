---
name: skia/ports/fallback-color-for-invalid-palette-stops
description: When resolving font palette indices or populating color stops, assign a conspicuous fallback color for invalid indices instead of returning early or skipping entries
rubric: correctness
severity: should_fix
---

**Rule.** When resolving font palette indices to configure paint or populating
gradient stops and colors (such as for OpenType COLR glyphs), do not return
early or skip out-of-bounds palette indices. Assign a conspicuous fallback color
(such as SkColors::kMagenta) to ensure output SkPaint instances are fully
configured and to preserve 1:1 parity between parallel stop positions and color
vectors.

**Why.** Returning early or skipping entries on malformed font data leaves
output SkPaint objects with stale or uninitialized state, and breaks 1:1 length
parity in parallel gradient stop and color arrays (causing downstream
out-of-bounds memory accesses or distorted geometry). Using an eye-catching
fallback color keeps rendering structures fully initialized and makes font data
defects immediately visible.

## Examples

Bad:

```cpp
// Desynchronizes parallel gradient stops and colors
dest_stops.push_back(color_stop.stop);
if (color_stop.palette_index >= palette.size()) {
    continue;
}
dest_colors.push_back(color);

// Leaves paint unconfigured on invalid palette index
if (palette_index >= fPalette.size()) {
    return;
}
color = SkColor4f::FromColor(fPalette[palette_index]);
```

Good:

```cpp
// Assigns fallback color to maintain 1:1 stop-to-color mapping
dest_stops.push_back(color_stop.stop);
if (color_stop.palette_index >= palette.size()) {
    dest_colors.push_back(SkColors::kMagenta);
} else {
    dest_colors.push_back(SkColor4f::FromColor(palette[color_stop.palette_index]));
}

// Assigns fallback color to keep paint fully configured
if (palette_index >= fPalette.size()) {
    color = SkColors::kMagenta;
} else {
    color = SkColor4f::FromColor(fPalette[palette_index]);
}
```
