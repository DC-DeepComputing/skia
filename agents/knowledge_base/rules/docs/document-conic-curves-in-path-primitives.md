---
name: skia/docs/document-conic-curves-in-path-primitives
description: When documenting SkPath curve primitives or geometry models, describe conic curves (rational quadratics) alongside quadratic and cubic Bézier curves
rubric: readability
severity: should_fix
---

**Rule.** When documenting or describing SkPath curve primitives or geometry
models, include conic curves (rational quadratics) alongside quadratic and cubic
Bézier curves. Explicitly describe conics as using control points and a weight
to represent circles, ovals, and arcs exactly without polynomial approximation.

**Why.** SkPath natively supports conics (rational quadratics specified by
control points and a weight) to represent conic sections such as circles and
arcs exactly. Describing Skia's curve model as only quadratic and cubic Bézier
curves is technically inaccurate and misrepresents the core path geometry
engine.

## Examples

Bad:

```cpp
* **Path**: A sequence of lines and curves that describe a shape. Skia uses
  quadratic and cubic Bézier curves to create smooth, scalable curves. A
  quadratic curve has one control point, while a cubic curve has two for
  more complex shapes.
```

Good:

```cpp
* **Path**: A sequence of lines and curves that describe a shape. Skia uses
  quadratic, cubic, and conic Bézier curves to create smooth, scalable curves. A
  quadratic curve has one control point, while a cubic curve has two for more
  complex shapes. A conic curve represents perfect circles etc using one control
  point and one weight.
```
