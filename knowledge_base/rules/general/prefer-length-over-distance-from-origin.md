---
name: skia/prefer-length-over-distance-from-origin
description: When computing the distance of a vector from the origin in SkSL shaders, use length(v) rather than distance(v, float2(0))
rubric: readability
severity: nit
---

**Rule.** In SkSL shader code, prefer calling length(v) rather than distance(v,
float2(0)) (or distance(v, floatN(0)) / distance(v, halfN(0))) when measuring
distance from the origin. Do not pass an explicit zero vector or origin
coordinate to distance().

**Why.** In shading languages, distance(a, b) is defined as length(a - b). When
computing distance from the origin, calling length(v) is mathematically
equivalent, idiomatic, and more concise, avoiding redundant zero-vector
constructor allocation and subtraction.

## Examples

Bad:

```cpp
// Redundant zero-vector construction and distance subtraction
float t = distance(pos, float2(0));
```

Good:

```cpp
// Direct vector length from origin
float t = length(pos);
```
