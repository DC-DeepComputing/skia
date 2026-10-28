---
name: skia/gm/document-hairline-cap-extensions-accurately
description: When documenting hairline stroke cap geometry, describe capped hairlines as extending by up to half a pixel rather than using approximate phrasing like 'about half a pixel'
rubric: readability
severity: nit
---

**Rule.** When documenting hairline stroke cap geometry and rasterization
behavior, accurately describe capped hairlines as extending line endpoints by up
to half a device pixel (0.5 pixels for round and square caps, 0 for butt caps)
rather than using approximate phrasing like 'about half a pixel'. Hairline
strokes have an exact 1-pixel effective width, and cap bounds are geometrically
bounded.

**Why.** In Skia, hairline strokes (width 0) rasterize with an exact effective
stroke width of 1 device pixel. Describing capped hairlines as extending by
'about half a pixel' implies non-deterministic or variable geometry, when in
reality square and round caps extend endpoints along the stroke direction by
exactly half the stroke width (0.5 device pixels) while butt caps do not extend
beyond endpoints.

## Examples

Bad:

```cpp
// Capped hairlines have interesting behavior because they extend out
// by about half a pixel, which has led to unintended effects when closing
// contours with antialiasing on.
```

Good:

```cpp
// Capped hairlines have interesting behavior because they extend out
// by up to half a pixel (0.5 device pixels for round/square caps, 0 for
// butt caps), which has led to unintended effects when closing contours.
```
