---
name: skia/modules/resolve-non-directional-svg-lengths-with-kother
description: When resolving non-directional SVG lengths or multi-axis scale factors, use SkSVGLengthContext::resolve with LengthType::kOther instead of ad-hoc geometric calculations
rubric: design
severity: should_fix
---

**Rule.** In the SVG module, resolve non-directional lengths, radii, or
multi-axis scale factors using SkSVGLengthContext::resolve(...,
SkSVGLengthContext::LengthType::kOther) (or resolveRect for 2D bounds). Do not
hand-roll custom geometric formulas or ad-hoc approximations like sqrt(width *
height) or arithmetic averages.

**Why.** The SVG specification specifies that non-directional lengths resolved
against a rectangular viewport or bounding box must evaluate using sqrt((w^2 +
h^2) / 2). SkSVGLengthContext::resolve(..., LengthType::kOther) encapsulates
this standard formula, avoiding duplicated math and ensuring SVG specification
conformance across elements and filter primitives.

## Examples

Bad:

```cpp
// Bad: hand-rolling ad-hoc scale calculation against bounding box
const SkScalar lengthSize = SkScalarSqrt(objBounds.width() * objBounds.height());
scale *= lengthSize;
```

Good:

```cpp
// Good: use SkSVGLengthContext::resolve with LengthType::kOther
const auto obbt = ctx.transformForCurrentOBB(fctx.primitiveUnits());
scale = SkSVGLengthContext({obbt.scale.x, obbt.scale.y})
            .resolve(SkSVGLength(scale, SkSVGLength::Unit::kPercentage),
                     SkSVGLengthContext::LengthType::kOther);
```
