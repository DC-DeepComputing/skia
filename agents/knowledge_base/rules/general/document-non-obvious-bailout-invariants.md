---
name: skia/document-non-obvious-bailout-invariants
description: When bailing out early or rejecting edge cases using non-obvious algorithmic invariants or conservative heuristics, document the reasoning in an explanatory comment
rubric: readability
severity: should_fix
---

**Rule.** When an early return, rejection, or bailout on an edge case relies on
non-obvious algorithmic invariants, pre-conditions, or conservative heuristics,
document the underlying reasoning with an explanatory comment. Detail the domain
assumptions (such as shape convexity, degenerate topologies, or lack of
measurable edges) and explain why the conservative or simplified handling is
safe.

**Why.** Early bailouts on edge cases frequently depend on subtle domain
assumptions or deliberate compromises that cannot be deduced from the
conditional check alone. Documenting these invariants prevents future
maintainers from misunderstanding the check as an oversight, breaking subtle
edge cases, or introducing unnecessary complexity.

## Examples

Bad:

```cpp
if (prevPt == pts[nextPt]) {
    return false;
}
```

Good:

```cpp
if (prevPt == pts[nextPt]) {
    // A pre-condition to getting here is that the path is convex, so if a
    // verb's start and end points are the same, it means it's the only
    // verb in the contour (and the only contour). While it's possible for
    // such a single verb to be a convex curve, we do not have any non-zero
    // length edges to conservatively test against without splitting or
    // evaluating the curve. For simplicity, just reject the rectangle.
    return false;
}
```
