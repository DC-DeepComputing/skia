---
name: skia/prefer-ternary-to-avoid-shader-branches
description: When conditionally updating values or computing validity masks in SkSL shaders, prefer ternary expressions or branchless math intrinsics like sign() over if statements
rubric: safety
severity: should_fix
---

**Rule.** In SkSL shaders and GPU shader code, prefer ternary expressions
(`condition ? a : b`) over `if` statements for simple conditional assignments
and value updates (such as `alpha = cond ? (1 - alpha) : alpha`), particularly
when comparing against zero or sentinels. Similarly, prefer branchless math
intrinsics such as `sign()` to calculate boolean, directional, or {-1, 1}
validity values instead of `if`/`else` branching returns.

**Why.** GPU fragment and vertex execution units run threads in SIMD lockstep
(warps or wavefronts). Control-flow `if` branch statements cause thread
divergence and costly branch instructions, whereas ternary select operations and
math intrinsics like `sign()` vectorize cleanly across GPU execution units
without branching.

## Examples

Bad:

```cpp
// Branching on conditional assignment causes thread divergence:
if (radiusPlusHalf.x < 0) {
    alpha = 1.0 - alpha;
}

// Branching return to calculate validity:
if (t * dRadius + radius0Param < 0) {
    return float2(0, -1);
}
return float2(t, 1);
```

Good:

```cpp
// Branchless conditional select using ternary operator:
alpha = radiusPlusHalf.x < 0 ? (1.0 - alpha) : alpha;

// Branchless math intrinsic avoids thread divergence:
float isValid = sign(t * dRadius + radius0Param);
return float2(t, isValid);
```
