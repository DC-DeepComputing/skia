---
name: skia/sksl/sksl-encapsulate-noop-bypass-in-shader-helpers
description: When defining SkSL multi-step shader transform helpers, place no-op or identity bypass checks inside the helper rather than duplicating them at caller sites
rubric: safety
severity: should_fix
---

**Rule.** In SkSL shader modules, place no-op or identity bypass checks (such as
returning early when flags == 0) inside shared multi-step transformation helper
functions rather than at individual caller sites. Ensure all shader entry points
automatically benefit from early-exit fast paths without duplicating branch
logic or omitting checks across callers.

**Why.** Placing bypass checks at caller sites requires every consuming shader
entry point (such as hardware sampling, YUV conversion, or cubic filtering) to
duplicate identical conditional branches, risking missed optimizations if a
caller omits the check. Encapsulating the early return inside the shared helper
guarantees that all callers bypass expensive ALU operations and matrix
transformations when flags are inactive.

## Examples

Bad:

```cpp
// Caller checks flags, while sibling callers omit the check and miss the fast path:
$pure half4 sk_hw_image_shader(...) {
    half4 sampleColor = $sample_image(coords, invImgSize, readSwizzle, s);
    if (csXformFlags == 0) {
        return sampleColor;
    }
    return sk_color_space_transform(sampleColor, csXformFlags, ...);
}

$pure half4 sk_yuv_image_shader(...) {
    // Omits check, executing redundant transformation ALU even when flags == 0:
    return sk_color_space_transform(sampleColor, csXformFlags, ...);
}
```

Good:

```cpp
// Shared helper encapsulates early-exit check, benefiting all callers automatically:
$pure half4 sk_color_space_transform(half4 halfColor,
                                     int flags,
                                     int srcKind,
                                     half3x3 gamutTransform,
                                     int dstKind,
                                     half4x4 coeffs) {
    if (flags == 0) {
        return halfColor;
    }
    float4 color = float4(halfColor);
    // ...
}
```
