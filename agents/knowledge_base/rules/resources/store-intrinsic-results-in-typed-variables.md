---
name: skia/resources/store-intrinsic-results-in-typed-variables
description: In SkSL intrinsic tests, assign return values to typed variables matching the expected return type instead of wrapping them in float or half constructors
rubric: testing
severity: should_fix
---

**Rule.** In SkSL test shaders (especially for intrinsics), assign return values
to local variables explicitly typed with the expected SkSL return type (such as
int, uint2, or bool) before aggregating them into output colors (sk_FragColor).
Do not wrap intrinsic calls directly inside float or half constructor
expressions or vector constructors.

**Why.** Directly wrapping intrinsic return values in float or half constructors
masks return type errors because floating-point types can implicitly construct
from signed or unsigned integers. Assigning return values to typed variables
forces downstream code generators and backend compilers (such as Tint for WGSL,
MSL, or GLSL) to validate type correctness in the generated shader.

## Examples

Bad:

```cpp
// resources/sksl/intrinsics/FindMSB.sksl
sk_FragColor.x  = half(findMSB(a))        + half(findMSB(b));
sk_FragColor.xy = half2(findMSB(int2(a))) + half2(findMSB(uint2(b)));
```

Good:

```cpp
// resources/sksl/intrinsics/FindMSB.sksl
int b1  = findMSB(a) + findMSB(b);
int2 b2 = findMSB(int2(a)) + findMSB(uint2(b));
sk_FragColor = half4(b1.xxxx + b2.xyxy);
```
