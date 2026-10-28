---
name: skia/gpu/factor-symmetric-math-into-helper-methods
description: When performing symmetric operations on paired geometric operands during intersection or distance routines, factor duplicated coordinate calculations into helper methods rather than repeating math inline
rubric: design
severity: should_fix
---

**Rule.** When performing symmetric operations on paired geometric objects (such
as computing point projections, parameter ratios, or deltas along edges during
intersection or clipping), factor the common coordinate math into a member
helper method on the operand type. Do not duplicate multi-line coordinate
calculations inline for both operands.

**Why.** Duplicating coordinate calculations across symmetric operands (such as
`this` and `other`) increases visual noise, obscures mathematical intent, and
introduces subtle bugs if numerical handling or precision updates are applied to
one operand but missed in the other.

## Examples

Bad:

```cpp
// In Edge::intersect(const Edge& other, ...):
float sDx = fBottom->fPoint.fX - fTop->fPoint.fX;
float sDy = fBottom->fPoint.fY - fTop->fPoint.fY;
double s = (std::abs(sDx) > std::abs(sDy)) ? ((double)p->fX - fTop->fPoint.fX) / sDx
                                           : ((double)p->fY - fTop->fPoint.fY) / sDy;
float tDx = other.fBottom->fPoint.fX - other.fTop->fPoint.fX;
float tDy = other.fBottom->fPoint.fY - other.fTop->fPoint.fY;
double t = (std::abs(tDx) > std::abs(tDy)) ? ((double)p->fX - other.fTop->fPoint.fX) / tDx
                                           : ((double)p->fY - other.fTop->fPoint.fY) / tDy;
```

Good:

```cpp
// Member method on Edge:
double pointTo(const SkPoint& p) const {
    float dx = fBottom->fPoint.fX - fTop->fPoint.fX;
    float dy = fBottom->fPoint.fY - fTop->fPoint.fY;
    return (std::abs(dx) > std::abs(dy)) ? ((double)p.fX - fTop->fPoint.fX) / dx
                                         : ((double)p.fY - fTop->fPoint.fY) / dy;
}

// In Edge::intersect(const Edge& other, ...):
double s = this->pointTo(*p);
double t = other.pointTo(*p);
```
