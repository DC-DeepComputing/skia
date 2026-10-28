---
name: skia/sksl/pack-mutually-exclusive-shader-uniforms
description: When encoding shader modes with signed sentinels, place mutually exclusive branchless logic in else blocks to drop defensive clamp calls and match arithmetic identities directly
rubric: safety
severity: should_fix
---

**Rule.** When passing uniforms to control shader modes or branchless operations
(such as unpremul, premul, or opacity), pack flags into compact vectors using
signed sentinels to isolate expensive branches. Place remaining mutually
exclusive branchless computations inside an else block rather than executing
them unconditionally, ensuring parameters are non-negative to eliminate
defensive clamp() calls and skipped ALU work. On the CPU, encode uniform values
to match target arithmetic identities directly (e.g., 0.0 and 1.0 for max or
multiply) instead of computing per-pixel decoding math or offsets in the shader.

**Why.** Evaluating per-pixel arithmetic decoding, offsets, or defensive clamp()
calls on GPU fragment shaders wastes ALU cycles and register bandwidth. Packing
flags into compact vectors conserves uniform buffer storage, isolating negative
sentinels in an if branch guarantees valid parameter domains in the else branch
to drop clamping, and matching arithmetic identities on the CPU allows
branchless paths to consume uniform components directly.

## Examples

Bad:

```cpp
// Bad: per-pixel decoding, runtime offsets, and clamping outside conditional branch
if (srcArgs.w < 0) {
    color = unpremul(color);
}
// args.x encodes opaque; args.y encodes premul with runtime offset
color.a = max(color.a, srcArgs.x * srcArgs.y);
color.rgb = color.rgb * max(color.a, 1.0 + srcArgs.y);
// Defensive clamping needed because negative sentinel leaks outside the branch
half3 mask = clamp(half3(1.0 - dstArgs.w - srcArgs.w, dstArgs.w, srcArgs.w),
                   half3(0), half3(1));
color.a = dot(color.ar1, mask);
```

Good:

```cpp
// Good: signed sentinel branch, CPU-aligned identities, and branchless logic in else
if (srcArgs.w < 0) {
    color = unpremul(color);
} else {
    // Non-negative domain in else allows dropping clamp() and skipping math on unpremul
    color.a = max(color.a, srcArgs.x);
    color.rgb = color.rgb * max(color.a, srcArgs.y); // args.y directly matches identity
    half alphaSwizzleA = 1.0 - srcArgs.w;
    half alphaSwizzle1 = srcArgs.w * dstArgs.w;
    half alphaSwizzleR = srcArgs.w - alphaSwizzle1;
    color.a = dot(color.ar1, half3(alphaSwizzleA, alphaSwizzleR, alphaSwizzle1));
}
```
