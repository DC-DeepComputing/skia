---
name: skia/avoid-redundant-vector-splats-in-sksl-arithmetic
description: When performing vector math or scalar-vector arithmetic in SkSL shaders, operate directly on vectors and scalars instead of unpacking components or splatting scalars
rubric: readability
severity: should_fix
---

**Rule.** In SkSL shader code, operate directly on vector variables using
component-wise math intrinsics (such as floor, ceil, or fract) and scalar
broadcasting instead of unpacking components (.x, .y) or reconstructing vectors
from per-component scalar math (e.g., half2(coords.x / s, coords.y / s)).
Likewise, do not wrap scalar expressions or single-channel texture samples in
vector constructors (such as half4(...) or float4(...)) or swizzle scalars into
repeated components (such as color.aaa) when performing scalar-vector
arithmetic.

**Why.** SkSL vector types natively support component-wise math intrinsics and
automatically broadcast scalar operands across vector components in binary
operations. Operating on vectors directly and relying on native broadcasting
eliminates redundant temporary variables, coordinate unpacking boilerplate, and
superfluous vector splats while enabling the SkSL compiler to optimize and fold
vector expressions.

## Examples

Bad:

```cpp
// Decomposing vector into components to perform scalar division and reconstructing
half2 lutCoords = half2(coords.x / kImgSize, coords.y / kImgSize);
half value = sample(lut, lutCoords).r - 0.5;

// Unpacking vector components instead of vectorizing math intrinsics and scalar addition
float2 snappedCoords = coords;
snappedCoords.x = floor(snappedCoords.x) + 0.5;
snappedCoords.y = floor(snappedCoords.y) + 0.5;
coords = snappedCoords;

// Redundant vector constructor wrapping a single-channel texture sample
half4 atlasClip = half4(sample(atlasSampler, texCoord).r);
return analyticClip * atlasClip;

// Redundant repeated swizzle splatting a scalar into a vector
return half4(color.rgb * color.aaa, color.a);
```

Good:

```cpp
// Direct vector-scalar arithmetic
half value = sample(lut, coords / kImgSize).r - 0.5;

// Vectorized component-wise intrinsic and scalar broadcasting
coords = floor(coords) + 0.5;

// Multiply vector directly by scalar sample
half atlasClip = sample(atlasSampler, texCoord).r;
return analyticClip * atlasClip;

// Multiply vector directly by scalar channel
return half4(color.rgb * color.a, color.a);
```
