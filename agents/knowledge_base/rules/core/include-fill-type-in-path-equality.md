---
name: skia/core/include-fill-type-in-path-equality
description: When implementing equality (operator==) for path containers, representations, or builders, compare fundamental semantic properties like SkPathFillType alongside verbs and points
rubric: correctness
severity: should_fix
---

**Rule.** When implementing equality (operator==) for path representations,
builders, or geometry containers (such as SkPath or SkPathBuilder), include
fundamental semantic properties like SkPathFillType in the comparison alongside
raw geometry (verbs, points, and conic weights). Do not rely solely on
coordinate and verb equivalence when shapes can differ in fill rules.

**Why.** A path's fill type determines how contours and interior areas are
evaluated (e.g. winding vs. even-odd) during rasterization and hit-testing.
Paths with identical verbs and coordinates but different fill types produce
distinct visual output, so omitting fill type from equality comparisons produces
false-positive equality.

## Examples

Bad:

```cpp
bool SkPathBuilder::operator==(const SkPathBuilder& o) const {
    if (this == &o) {
        return true;
    }
    if (fSegmentMask != o.fSegmentMask) {
        return false;
    }
    return fVerbs == o.fVerbs && fPts == o.fPts && fConicWeights == o.fConicWeights;
}
```

Good:

```cpp
bool SkPathBuilder::operator==(const SkPathBuilder& o) const {
    if (this == &o) {
        return true;
    }
    if (fSegmentMask != o.fSegmentMask || fFillType != o.fFillType) {
        return false;
    }
    return fVerbs == o.fVerbs && fPts == o.fPts && fConicWeights == o.fConicWeights;
}
```
