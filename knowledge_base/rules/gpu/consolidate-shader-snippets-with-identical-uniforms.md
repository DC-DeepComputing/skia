---
name: skia/gpu/consolidate-shader-snippets-with-identical-uniforms
description: When defining Graphite shader snippets sharing identical uniform payloads, consolidate minor variations into a single snippet using flag branching rather than adding separate BuiltInCodeSnippetIDs
rubric: design
severity: should_fix
---

**Rule.** In Graphite, do not introduce separate BuiltInCodeSnippetID entries
and SkSL functions for minor behavioral variations when they upload the exact
same uniform data layout. Consolidate related variations into a single shader
snippet and SkSL function, gating the variant behavior with existing or packed
flag uniforms so general optimizations apply across all invocations without
duplicate snippet registration.

**Why.** Separate code snippets with identical uniform layouts duplicate
registration and serialization boilerplate across BuiltInCodeSnippetID,
ShaderCodeDictionary, and KeyHelpers without saving uniform buffer bandwidth.
Consolidating them into a single snippet simplifies the snippet dictionary,
prevents shader code duplication, and allows optimizations (such as early-outs
when flags are zero) to benefit all callers.

## Examples

Bad:

```cpp
// Separate snippet and BuiltInCodeSnippetID despite identical uniform layout:
$pure half4 sk_color_space_transform_swizzle(half4 halfColor, int flags, int srcKind,
                                             half3x3 gamutTransform, int dstKind,
                                             half4x4 coeffs) {
    if (flags == 0) return halfColor;
    if (bool(flags & $kColorSpaceXformFlagAlphaSwizzle)) {
        halfColor.a = dot(halfColor.r1, half2(coeffs[1][3], coeffs[3][3]));
    }
    return sk_color_space_transform(halfColor, flags, srcKind, gamutTransform, dstKind, coeffs);
}
```

Good:

```cpp
// Consolidated into a single snippet handling the flag-gated swizzle:
$pure half4 sk_color_space_transform(half4 halfColor, int flags, int srcKind,
                                     half3x3 gamutTransform, int dstKind,
                                     half4x4 coeffs) {
    if (flags == 0) {
        return halfColor;
    }
    float4 color = float4(halfColor);
    if (bool(flags & $kColorSpaceXformFlagAlphaSwizzle)) {
        color.a = dot(color.r1, float2(coeffs[1][3], coeffs[3][3]));
    }
    // ...
}
```
