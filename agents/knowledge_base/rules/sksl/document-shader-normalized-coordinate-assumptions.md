---
name: skia/sksl/document-shader-normalized-coordinate-assumptions
description: When defining shader functions that sample textures using pre-normalized coordinates without image dimension uniforms, document the coordinate space precondition with an explicit comment
rubric: readability
severity: should_fix
---

**Rule.** When defining SkSL shader functions or image helpers that sample
textures directly using pre-normalized coordinates rather than taking image
dimensions or inverse-dimension parameters (such as invImgSize), explicitly
document in a comment that the coordinate parameter is assumed to be normalized.
Clarify caller expectations whenever a shader entry point bypasses standard
pixel-space normalization.

**Why.** Most SkSL image shaders receive pixel coordinates and perform
normalization internally using dimensions or invImgSize uniforms. When a shader
function expects coordinates already normalized in [0, 1] texture space,
omitting dimension parameters without documenting the assumption confuses
callers and leads to coordinate space mismatches or redundant normalization.

## Examples

Bad:

```cpp
$pure half4 sk_hw_image_shader(float2 coords, sampler2D s) {
    return sample(s, coords);
}
```

Good:

```cpp
$pure half4 sk_hw_image_shader(float2 coords, sampler2D s) {
    // This assumes that `coords` have already been normalized.
    return sample(s, coords);
}
```
