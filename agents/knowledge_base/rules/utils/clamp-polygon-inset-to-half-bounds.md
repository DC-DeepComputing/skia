---
name: skia/utils/clamp-polygon-inset-to-half-bounds
description: When computing or clamping polygon inset distances, clamp to at most half bounds using SkRectPriv::HalfWidth and HalfHeight to prevent polygon collapse and offset failure
rubric: correctness
severity: should_fix
---

**Rule.** When computing, clamping, or validating polygon inset distances (such
as for SkOffsetSimplePolygon or shadow tessellation), clamp the inset to at most
half of the polygon's bounds using SkRectPriv::HalfWidth and
SkRectPriv::HalfHeight rather than the full bounds width or height. Do not allow
insets to exceed half the bounding dimensions, which causes polygon collapse,
self-intersection, and offset failure.

**Why.** Insetting a simple polygon by more than half its width or height causes
opposite edges to cross, collapsing the geometry or introducing invalid
self-intersections. In Skia, SkOffsetSimplePolygon enforces this invariant and
returns false if the inset exceeds half the bounds; clamping to full bounds
permits invalid inset values that trigger unexpected offsetting failures.

## Examples

Bad:

```cpp
// Bad: Clamping inset to full bounds allows insets that collapse the polygon.
inset = SkTPin(inset, 0.0f, std::min(path.getBounds().width(), path.getBounds().height()));
if (!SkOffsetSimplePolygon(verts, count, bounds, inset, &outVerts)) {
    return false;
}
```

Good:

```cpp
// Good: Clamp inset to at most half-width and half-height to prevent offset failure.
inset = std::min(inset, std::min(SkTAbs(SkRectPriv::HalfWidth(fPathBounds)),
                                 SkTAbs(SkRectPriv::HalfHeight(fPathBounds))));
if (!SkOffsetSimplePolygon(verts, count, bounds, inset, &outVerts)) {
    return false;
}
```
