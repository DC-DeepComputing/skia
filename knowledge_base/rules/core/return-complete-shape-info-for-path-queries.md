---
name: skia/core/return-complete-shape-info-for-path-queries
description: When exposing path shape query or inspection APIs, return complete info structs bundling winding direction and start index with geometry rather than bare SkRect or SkRRect
rubric: correctness
severity: should_fix
---

**Rule.** When exposing query or inspection APIs for path shapes (such as ovals,
rectangles, or round rects), return complete shape info structs (such as
SkPathOvalInfo or SkPathRRectInfo) that include contour winding direction
(SkPathDirection) and start index (fStartIndex) alongside the geometry. Do not
return bare geometry types like SkRect or SkRRect without orientation metadata.

**Why.** In Skia, path representations of primitives preserve winding direction
and the starting vertex or corner of the contour. Omitting direction and start
index in inspection or query APIs prevents callers from faithfully inspecting,
transforming, serializing, or reconstructing the original path contours.

## Examples

Bad:

```cpp
// Returns bare geometry and loses contour orientation
std::optional<SkRect> asOval() const;
std::optional<SkRRect> asRRect() const;
```

Good:

```cpp
// Bundles geometry with winding direction and starting vertex
struct SkPathOvalInfo {
    SkRect          fOval;
    SkPathDirection fDirection;
    uint8_t         fStartIndex;
};
std::optional<SkPathOvalInfo> asOval() const;

struct SkPathRRectInfo {
    SkRRect         fRRect;
    SkPathDirection fDirection;
    uint8_t         fStartIndex;
};
std::optional<SkPathRRectInfo> asRRect() const;
```
