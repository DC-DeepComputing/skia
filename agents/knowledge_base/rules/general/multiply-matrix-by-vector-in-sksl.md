---
name: skia/multiply-matrix-by-vector-in-sksl
description: When applying matrix transformations to vectors in SkSL shaders, multiply matrix * vector rather than vector * matrix to respect column-major layout
rubric: correctness
severity: should_fix
---

**Rule.** When applying a matrix transformation to a vector in SkSL shaders,
multiply matrix * vector rather than vector * matrix. Because SkSL matrix types
and constructors are column-major, multiplying a vector on the left (v * M)
calculates the transpose transformation (M^T * v).

**Why.** In SkSL and GLSL, matrix constructors populate matrices
column-by-column, and matrix * vector evaluates the dot product of each matrix
row with the column vector (M * v). Multiplying vector * matrix evaluates the
dot product of each matrix column with the vector (v^T * M = M^T * v),
unintentionally transposing asymmetric transformations such as Jacobians or
shears.

## Examples

Bad:

```cpp
// Anti-pattern: vector * matrix computes transpose (M^T * v) instead of M * v
float2x2 jacobian = float2x2(dFdx(unormTexCoords), dFdy(unormTexCoords));
half2 offset = half2(pixelGeometryDelta * jacobian);
```

Good:

```cpp
// Correct: matrix * vector applies column-major transformation M * v
float2x2 jacobian = float2x2(dFdx(unormTexCoords), dFdy(unormTexCoords));
half2 offset = half2(jacobian * pixelGeometryDelta);
```
