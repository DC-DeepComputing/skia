---
name: skia/gpu/qualify-plane-specific-yuv-identifiers
description: When declaring parameters, struct members, uniforms, or variables specific to subsampled planes in planar YUV pipelines, qualify identifiers with plane names like UV
rubric: readability
severity: nit
---

**Rule.** When declaring parameters, struct members, uniform definitions, or
shader variables that apply specifically to subsampled planes (such as UV in
planar YUV or YUVA formats), explicitly qualify the identifier with the target
plane (e.g., linearFilterUVInset, subsetUV, invImgSizeUV). Maintain this
qualified naming consistently across C++ data structures, uniform tables, and
SkSL shader entry points rather than using generic, unqualified names like
linearFilterInset.

**Why.** Generic names falsely imply that an inset, subset, or dimension applies
uniformly to all image planes. In YUV pipelines where luma (Y) and chroma (UV)
have different resolutions and filtering behaviors, plane-qualified identifiers
clarify intent and avoid coordination bugs between C++ pipeline setup and SkSL
shaders.

## Examples

Bad:

```cpp
// C++ uniform and struct setup:
struct YUVImageShaderBlock {
    SkPoint fLinearFilterInset;
};
Uniform{"linearFilterInset", SkSLType::kFloat2};

// SkSL shader parameter:
$pure half4 sk_hw_yuv_image_shader(float2 coords,
                                   float2 invImgSizeY,
                                   float2 invImgSizeUV,
                                   float4 subset,
                                   float2 linearFilterInset,
                                   sampler2D sY,
                                   sampler2D sU,
                                   sampler2D sV) {
```

Good:

```cpp
// C++ uniform and struct setup:
struct YUVImageShaderBlock {
    SkPoint fLinearFilterUVInset;
};
Uniform{"linearFilterUVInset", SkSLType::kFloat2};

// SkSL shader parameter:
$pure half4 sk_hw_yuv_image_shader(float2 coords,
                                   float2 invImgSizeY,
                                   float2 invImgSizeUV,
                                   float4 subset,
                                   float2 linearFilterUVInset,
                                   sampler2D sY,
                                   sampler2D sU,
                                   sampler2D sV) {
```
