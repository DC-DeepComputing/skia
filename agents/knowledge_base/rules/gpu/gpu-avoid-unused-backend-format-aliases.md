---
name: skia/gpu/gpu-avoid-unused-backend-format-aliases
description: When mapping backend formats to Graphite TextureFormat in bidirectional conversion functions, avoid asymmetric one-way switch cases outside the shared format mapping macro; eliminate unused formats.
rubric: readability
severity: nit
---

**Rule.** In Graphite backend format conversion utilities (such as
VulkanGraphiteUtils, DawnGraphiteUtils, or MtlGraphiteUtils), maintain a 1-to-1
correspondence between backend formats and TextureFormat within the shared
bidirectional mapping macro (*_FORMAT_MAPPING). Do not introduce asymmetric,
one-way format conversions or aliases in manual switch cases outside the mapping
macro; if an alternate backend format is unused in the codebase, eliminate it
entirely rather than preserving an ad-hoc alias.

**Why.** Graphite uses bidirectional X-macros to synchronize
<Backend>FormatToTextureFormat and TextureFormatToBackendFormat. Introducing
one-way format cases outside the shared macro breaks the 1-to-1 relationship,
creates ambiguity about canonical format encodings, and retains dead format code
that complicates backend capability tables.

## Examples

Bad:

```cpp
TextureFormat VkFormatToTextureFormat(VkFormat format) {
#define M(TF, VK) case VK: return TF;
    switch (format) {
        VK_FORMAT_MAPPING(M)
        // Kept outside bidirectional macro to avoid duplicate cases
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32: return TextureFormat::kRGBA8;
        default: return TextureFormat::kUnsupported;
    }
#undef M
}
```

Good:

```cpp
TextureFormat VkFormatToTextureFormat(VkFormat format) {
#define M(TF, VK) case VK: return TF;
    switch (format) {
        VK_FORMAT_MAPPING(M)
        default: return TextureFormat::kUnsupported;
    }
#undef M
}
```
