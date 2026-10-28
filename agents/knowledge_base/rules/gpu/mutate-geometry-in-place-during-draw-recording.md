---
name: skia/gpu/mutate-geometry-in-place-during-draw-recording
description: When adapting geometry in Graphite draw recording methods, update the existing Geometry container in place via its setter methods rather than declaring intermediate local shapes
rubric: readability
severity: nit
---

**Rule.** In Graphite draw recording methods (such as `Device::drawGeometry`),
update an existing `Geometry` container in place using its setter methods (such
as `setCoverageMaskShape()` or `setShape()`) when adapting or replacing
primitives. Do not maintain separate intermediate local shape variables across
function scope to wrap into a temporary `Geometry` constructor at the draw
recording site.

**Why.** Mutating the in-scope `Geometry` instance directly eliminates
unnecessary intermediate local variables floating across function scope and
avoids constructing redundant temporary `Geometry` wrappers when dispatching to
`recordDraw()`.

## Examples

Bad:

```cpp
CoverageMaskShape maskShape;
...
std::tie(maskShape, pos, renderer) = *shapeResult;
...
fDC->recordDraw(renderer, Transform::Translate(pos.fX, pos.fY), Geometry(maskShape), ...);
```

Good:

```cpp
geometry.setCoverageMaskShape(std::get<0>(*shapeResult));
pos      = std::get<1>(*shapeResult);
renderer = std::get<2>(*shapeResult);
...
fDC->recordDraw(renderer, Transform::Translate(pos.fX, pos.fY), geometry, ...);
```
