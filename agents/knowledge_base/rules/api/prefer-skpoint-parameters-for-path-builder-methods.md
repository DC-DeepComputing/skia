---
name: skia/api/prefer-skpoint-parameters-for-path-builder-methods
description: When defining geometry or relative navigation methods in path builders, accept SkPoint or SkVector parameters rather than separate scalar coordinates, delegating scalar overloads
rubric: readability
severity: nit
---

**Rule.** When defining geometry and relative navigation methods (such as
rMoveTo, rLineTo, or rQuadTo) in SkPathBuilder or related path builder classes,
declare primary methods to take SkPoint or SkVector parameters rather than
separate scalar coordinate pairs (such as SkScalar dx, SkScalar dy). If scalar
overloads are provided for caller convenience, define them as inline wrappers
that delegate to the primary SkPoint/SkVector overload.

**Why.** Standardizing on SkPoint and SkVector across path builder methods
ensures consistent API design and allows callers to pass structured points or
braced initializers (such as {x, y}) uniformly. Delegating convenience scalar
overloads directly to the primary point-taking methods prevents duplicate
implementation logic and maintains single-sourced geometry handling.

## Examples

Bad:

```cpp
// Bad: Primary builder method takes separate scalar coordinates directly
SkPathBuilder& SkPathBuilder::rMoveTo(SkScalar dx, SkScalar dy) {
    return this->moveTo(fLastPt.fX + dx, fLastPt.fY + dy);
}
```

Good:

```cpp
// Good: Primary method takes SkVector/SkPoint; scalar overload delegates inline
SkPathBuilder& SkPathBuilder::rMoveTo(SkVector pt) {
    return this->moveTo(fLastPt + pt);
}
inline SkPathBuilder& SkPathBuilder::rMoveTo(SkScalar dx, SkScalar dy) {
    return this->rMoveTo({dx, dy});
}
```
