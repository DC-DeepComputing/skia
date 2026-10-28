---
name: skia/gpu/spell-out-vulkan-version-in-comments
description: When documenting extension-promoted Vulkan functions or capabilities in section comments, explicitly write 'Vulkan <version>' rather than bare version numbers
rubric: readability
severity: nit
---

**Rule.** When referencing core Vulkan API versions in comments documenting
functions or capabilities (such as procedures available via an extension or
promoted to core), explicitly write 'Vulkan <version>' (e.g., 'Vulkan 1.2')
rather than omitting the 'Vulkan' prefix (e.g., '1.2').

**Why.** Omitting the 'Vulkan' prefix when referencing core API versions creates
ambiguous phrasing in comments (such as 'or 1.2') and inconsistent documentation
across Vulkan interface loaders and capability headers. Explicitly writing
'Vulkan <version>' maintains clear, searchable, and uniform comments throughout
the subsystem.

## Examples

Bad:

```cpp
// Functions for VK_KHR_create_renderpass2 or 1.2
if (physicalDeviceVersion >= VK_API_VERSION_1_2) {
    ACQUIRE_PROC(CreateRenderPass2, KHR);
}
```

Good:

```cpp
// Functions for VK_KHR_create_renderpass2 or Vulkan 1.2
if (physicalDeviceVersion >= VK_API_VERSION_1_2) {
    ACQUIRE_PROC(CreateRenderPass2, KHR);
}
```
