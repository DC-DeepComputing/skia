---
name: skia/reserve-normalize-for-unit-scaling
description: When naming methods that scale vectors or line equations by non-unit factors like winding, avoid normalize(); use explicit names like rescaleToWinding
rubric: readability
severity: should_fix
---

**Rule.** Do not name a method or helper 'normalize()' if the resulting vector,
normal, or line equation is scaled by a non-unit factor (such as winding
count/direction or weight). Use an explicit name such as 'rescaleToWinding()' or
'scaleBy()' to accurately convey the scaling behavior.

**Why.** In mathematics, geometry, and graphics conventions across Skia,
'normalize()' denotes scaling a vector, normal, or implicit line equation to
unit length (magnitude 1.0) or to a standard [0, 1] range. Applying non-unit
scaling inside a method named 'normalize()' misleads callers who expect unit
magnitude, leading to subtle geometric calculation errors or redundant
normalization passes.

## Examples

Bad:

```cpp
void normalize() {
    fLine.normalize();
    fLine = fLine * fWinding;

    if (fOriginalSegment) {
        fOriginalSegment->fLine.normalize();
        fOriginalSegment->fLine = fOriginalSegment->fLine * fWinding;
    }
}
```

Good:

```cpp
void rescaleToWinding() {
    fLine.normalize();
    fLine = fLine * fWinding;

    if (fOriginalSegment) {
        fOriginalSegment->fLine.normalize();
        fOriginalSegment->fLine = fOriginalSegment->fLine * fWinding;
    }
}
```
