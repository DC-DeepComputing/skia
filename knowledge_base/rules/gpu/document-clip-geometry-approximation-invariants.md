---
name: skia/gpu/document-clip-geometry-approximation-invariants
description: When defining internal helper classes that adapt or approximate draw geometries for clipping, document the abstraction's purpose and approximation invariants
rubric: readability
severity: should_fix
---

**Rule.** When defining or modifying internal helper classes that approximate or
adapt draw geometries for clipping and simplification (such as
ClipStack::DrawShape), document the abstraction's purpose and its geometric
approximation invariants. Clarify how the helper maps into the clip stack, which
geometry types preserve exact shape geometry (e.g., filled shapes), and which
types are reduced to bounding approximations (e.g., stroked or non-shape
geometries).

**Why.** GPU clip stacks rely on geometric simplifications to reconcile draw
calls with clip elements. Without clear documentation of approximation
invariants, maintainers may make incorrect assumptions about clipping precision,
edge-antialiasing preservation, or renderer selection.

## Examples

Bad:

```cpp
// ClipStack::DrawShape

class ClipStack::DrawShape {
public:
    DrawShape(const Transform& localToDevice, const Geometry& geometry);
    ...
};
```

Good:

```cpp
/**
 * DrawShape represents the approximate shape that is being drawn in order to compare it against
 * the clip stack's RawElements. It is able to map to a `TransformedShape` to be simplified with
 * either the SaveRecord or each element. For non-Shape geometries and stroked shapes, it
 * represents the oriented bounding box. For filled Shapes, it preserves the original shape for
 * more accurate contains/intersect checks and geometrically combining RawElements into the
 * shape.
 */
class ClipStack::DrawShape {
public:
    DrawShape(const Transform& localToDevice, const Geometry& geometry);
    ...
};
```
