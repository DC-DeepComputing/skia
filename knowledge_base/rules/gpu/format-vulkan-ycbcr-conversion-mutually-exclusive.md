---
name: skia/gpu/format-vulkan-ycbcr-conversion-mutually-exclusive
description: When stringifying or formatting Vulkan YCbCr conversion info, check hasExternalFormat to format externalFormat prefixed with 'x' or format the standard VkFormat
rubric: readability
severity: should_fix
---

**Rule.** When formatting or stringifying Vulkan YCbCr conversion descriptions
(such as VulkanYcbcrConversionInfo), treat standard VkFormat and externalFormat
as mutually exclusive. Check whether an external format is present (such as via
hasExternalFormat()) to format either externalFormat prefixed with 'x' or the
standard format, rather than printing both fields unconditionally.

**Why.** In Vulkan YCbCr conversions, standard VkFormat and externalFormat (e.g.
from Android Hardware Buffers) are mutually exclusive—one is always zero or
undefined when the other is configured. Emitting both values unconditionally
produces confusing debug output with placeholder zeroes (e.g., '0 12345' or '44
0'), whereas formatting only the active format and prefixing external formats
with 'x' keeps sampler and pipeline labels concise and easily interpretable.

## Examples

Bad:

```cpp
std::string VulkanYcbcrConversion::InfoToString(const VulkanYcbcrConversionInfo& info) {
    // Bad: unconditionally formatting both VkFormat and externalFormat
    return SkStringPrintf("%d %" PRIu64 " %s ...",
                          info.format(), info.externalFormat(), ...);
}
```

Good:

```cpp
std::string VulkanYcbcrConversion::InfoToString(const VulkanYcbcrConversionInfo& info) {
    std::string result;
    if (info.hasExternalFormat()) {
        result += 'x';
        result += std::to_string(info.externalFormat());
    } else {
        result += std::to_string(info.format());
    }
    // ... format remaining YCbCr conversion properties
    return result;
}
```
