---
name: skia/pass-points-directly-without-unpacking
description: When passing or calculating geometric points and vectors, use SkPoint and SkVector operator overloads and instances directly instead of unpacking coordinates
rubric: readability
severity: should_fix
---

**Rule.** Do not unpack an existing geometric object (such as SkPoint, SkIPoint,
or SkVector) into an initializer list or temporary constructor `{pt.fX, pt.fY}`
when passing it to functions that accept the object directly (such as
`SkMatrix::mapPoint`). Likewise, use overloaded vector arithmetic operators
(`+`, `-`, `*`) on SkPoint and SkVector (such as `dst - src` or `pt + vec * t`)
instead of manually computing and assigning per-component scalar coordinate math
on `.fX` and `.fY`.

**Why.** SkPoint, SkIPoint, and SkVector provide standard arithmetic operator
overloads (+, -, *) and are accepted natively across geometry APIs. Using
instances and vector expressions directly eliminates redundant aggregate
reconstruction, avoids verbose per-component boilerplate, and makes geometric
math more concise and readable.

## Examples

Bad:

```cpp
// Redundantly unpacking an existing SkPoint when calling mapPoint()
static XPS_POINT xps_point(const SkPoint& point, const SkMatrix& matrix) {
    return xps_point(matrix.mapPoint({point.fX, point.fY}));
}

// Manually subtracting coordinates instead of using SkPoint::operator-
SkMatrix m = SkMatrix::Translate(dst[0].fX - src[0].fX, dst[0].fY - src[0].fY);

// Manual per-component scalar math on .fX and .fY
SkPoint hit;
hit.fX = lineStart.fX + tangent.fX * t;
hit.fY = lineStart.fY + tangent.fY * t;
```

Good:

```cpp
// Pass the existing SkPoint instance directly
static XPS_POINT xps_point(const SkPoint& point, const SkMatrix& matrix) {
    return xps_point(matrix.mapPoint(point));
}

// Pass the vector difference directly to SkMatrix::Translate(SkVector)
SkMatrix m = SkMatrix::Translate(dst[0] - src[0]);

// Use overloaded vector arithmetic expressions
const SkPoint hit = lineStart + tangent * t;
```
