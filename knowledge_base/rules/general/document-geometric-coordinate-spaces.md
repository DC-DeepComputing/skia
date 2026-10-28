---
name: skia/document-geometric-coordinate-spaces
description: When declaring geometric variables, bounds fields, or clipping and conversion helpers, document coordinate spaces, units, preconditions, and invariant guarantees
rubric: readability
severity: should_fix
---

**Rule.** When declaring geometric member variables, parameters, struct fields,
or return values (such as SkRect, SkIRect, or SkPoint), explicitly specify their
coordinate space (e.g., local, tile, device, or target texture space) and
whether transformations or clipping apply. For geometric clipping and
transformation helpers, document parameter semantics, input preconditions,
empty-return semantics, and output guarantees (such as point ordering, boundary
snapping, and degenerate handling). When converting coordinates between spaces
(such as scaling by inverse tile dimensions), document target units if variable
names do not make it explicit. Document empty-value sentinel semantics on bounds
fields, and place invariant comments directly on member declarations rather than
stale method comments.

**Why.** Geometric routines and variables span multiple coordinate frames (such
as tile units, local, and device space), clipping states, and sentinel
conventions across rendering pipelines. Omitting coordinate spaces, unit
conversions, or output guarantees (like point ordering and boundary snapping)
forces downstream callers to guess frames and invariants, risking
double-transforms or pipeline ordering bugs.

## Examples

Bad:

```cpp
// Undocumented clipping helper lacking parameter roles, coordinate frames, and guarantees:
static SK_ALWAYS_INLINE std::array<SkPoint, 2> clipToTile(
        const Line& line, const std::array<float, 4>& bounds, uint32_t mask);

// Coordinate space and return semantics omitted on geometric helper:
std::optional<std::array<SkPoint, 2>> reference_clip(
        const Line& line, float minX, float minY, float maxX, float maxY);

// Coordinate conversion without documenting target coordinate space or units:
constexpr float invW = 1.0f / static_cast<float>(kTileWidth);
float p0X = line.p0.fX * invW;

// Ambiguous coordinate space and undocumented empty semantics:
struct SkStageRec {
    SkRect fDstBounds;
};
```

Good:

```cpp
// Consumes tile coordinates and intersection mask; returns intersection points in tile space.
// Guarantees: 1) Y-Sorting (top-to-bottom), 2) Boundary Snapping to tile bounds.
static SK_ALWAYS_INLINE std::array<SkPoint, 2> ClipToTile(
        const Line& line, const std::array<float, 4>& bounds, uint32_t mask);

// If a clipped segment exists, returns endpoints in tile space (or nullopt if clipped out).
std::optional<std::array<SkPoint, 2>> reference_clip(
        const Line& line, float minX, float minY, float maxX, float maxY);

// Explicit comment documenting target unit conversion:
constexpr float invW = 1.0f / static_cast<float>(kTileWidth);
// Map line into tile units
float p0X = line.p0.fX * invW;

// Explicit coordinate space and empty-value fallback semantics:
struct SkStageRec {
    // Device-space bounds. An empty value indicates uncomputed bounds falling back to heuristics.
    SkRect fDstBounds;
};
```
