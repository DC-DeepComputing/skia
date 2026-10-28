---
name: skia/core/keep-core-geometry-data-minimal
description: When modifying core geometry data objects like SkPathRef or SkPathData, do not add auxiliary or algorithm-specific cache members; manage derived measurements externally
rubric: safety
severity: should_fix
---

**Rule.** Do not add auxiliary or algorithm-specific member variables (such as
cached segment distances or temporary calculation state) to ubiquitous,
size-sensitive core geometry storage classes like SkPathRef or SkPathData. Keep
core geometry storage layouts strictly minimal, storing only intrinsic geometry
definition arrays (verbs, points, weights), and compute or cache auxiliary
measurements in external utility classes.

**Why.** Core geometry objects are allocated for every path in the system and
rely on inline STArrays where even small increases in object layout footprint
waste significant heap memory across drawing operations. Storing
algorithm-specific caches in core data bloats all path allocations for a feature
that only a fraction of paths use.

## Examples

Bad:

```cpp
// include/private/SkPathRef.h / src/core/SkPathData.h
using PathDistancesArray = skia_private::STArray<2, SkScalar>;
...
PointsArray fPoints;
VerbsArray fVerbs;
ConicWeightsArray fConicWeights;
PathDistancesArray fDistances; // Bloats every path with interpolation cache
```

Good:

```cpp
// include/private/SkPathRef.h / src/core/SkPathData.h
PointsArray fPoints;
VerbsArray fVerbs;
ConicWeightsArray fConicWeights;
// Keep measurements and algorithm caches external (e.g. via SkContourMeasure)
```
