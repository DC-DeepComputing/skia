---
name: skia/omit-antialias-for-draw-vertices
description: When configuring SkPaint for SkCanvas::drawVertices, drawMesh, or drawPatch, omit setAntiAlias, mask filters, and path effects as mesh draws ignore them
rubric: readability
severity: should_fix
---

**Rule.** Do not configure antialiasing, mask filters, or path effects (such as
calling paint.setAntiAlias(true)) on SkPaint objects passed to
SkCanvas::drawVertices, drawMesh, or drawPatch. These mesh and patch draw
methods intentionally ignore antialiasing, mask filters, and path effects to
avoid visible seams along shared internal triangle edges.

**Why.** Mesh and patch primitives rasterize adjacent triangles that must align
seamlessly; per-primitive antialiasing would create visible seams along shared
edges. Because SkCanvas mesh draw methods explicitly ignore antialiasing, mask
filters, and path effects, configuring them is dead code and misleads readers
about rendering behavior.

## Examples

Bad:

```cpp
SkPaint paint;
paint.setAntiAlias(true);
paint.setColor(SK_ColorWHITE);
canvas->drawVertices(vertices, SkBlendMode::kModulate, paint);
```

Good:

```cpp
SkPaint paint;
paint.setColor(SK_ColorWHITE);
canvas->drawVertices(vertices, SkBlendMode::kModulate, paint);
```
