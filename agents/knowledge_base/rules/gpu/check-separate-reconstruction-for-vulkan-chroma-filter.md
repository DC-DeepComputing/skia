---
name: skia/gpu/check-separate-reconstruction-for-vulkan-chroma-filter
description: When configuring Vulkan YCbCr sampler conversion, do not force chromaFilter to VK_FILTER_NEAREST on missing sampler linear filtering without checking separate reconstruction support
rubric: correctness
severity: should_fix
---

**Rule.** When configuring Vulkan YCbCr sampler conversion
(VkSamplerYcbcrConversionCreateInfo), do not force the chroma reconstruction
filter (chromaFilter) to VK_FILTER_NEAREST solely because
VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT is missing. Only force
chromaFilter to match sampler nearest filtering when the format also lacks
VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT.

**Why.** VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT governs sampler
min/mag filtering, whereas chroma reconstruction filtering is governed by
VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT. Sampler and
chroma filters are only required to match if the format lacks separate
reconstruction support; when separate reconstruction is supported, chroma
filtering can remain linear even when sampler min/mag filtering is restricted to
nearest.

## Examples

Bad:

```cpp
VkFilter chromaFilter = conversionInfo.fChromaFilter;
if (!(conversionInfo.fFormatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
    chromaFilter = VK_FILTER_NEAREST;
}
```

Good:

```cpp
VkFilter chromaFilter = conversionInfo.fChromaFilter;
if (!(conversionInfo.fFormatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
    if (!(conversionInfo.fFormatFeatures &
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT)) {
        chromaFilter = VK_FILTER_NEAREST;
    }
}
```
