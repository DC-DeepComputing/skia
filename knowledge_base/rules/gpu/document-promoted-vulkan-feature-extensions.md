---
name: skia/gpu/document-promoted-vulkan-feature-extensions
description: When negotiating Vulkan features, document which core struct supersedes promoted extensions and clarify when standalone extension checks have not been promoted to core
rubric: readability
severity: should_fix
---

**Rule.** When omitting, disabling, or adjusting Vulkan extension feature
structs based on core API versions (e.g., apiVersion >= VK_API_VERSION_1_X),
document the original extension name and the corresponding core struct (e.g.,
VkPhysicalDeviceVulkan1*Features) that supersedes it, stating explicitly that
the extension's feature is included in or subsumed by the core struct.
Additionally, when querying or enabling standalone extension feature structs
following version-promoted sections, add comments clarifying that those
extension features have not been promoted to core in any supported Vulkan
version.

**Why.** Core Vulkan versions promote standalone extension features into unified
core feature structs (such as VkPhysicalDeviceVulkan13Features or
VkPhysicalDeviceVulkan14Features). Clarifying which core struct subsumes an
extension explains why extension structs are disabled under newer API versions,
while documenting unpromoted extensions explains why standalone checks remain
unguarded by version checks, preventing confusion during feature negotiation
maintenance.

## Examples

Bad:

```cpp
if (apiVersion >= VK_API_VERSION_1_4) {
    toQuery.fDynamicRenderingLocalRead = false;
    toQuery.fHostImageCopy = false;
}

toAdd.fRasterizationOrderAttachmentAccess = exts.fRasterizationOrderAttachmentAccessARM ||
                                            exts.fRasterizationOrderAttachmentAccessEXT;
toAdd.fBlendOperationAdvanced = exts.fBlendOperationAdvancedEXT;
```

Good:

```cpp
if (apiVersion >= VK_API_VERSION_1_4) {
    // VK_KHR_dynamic_rendering_local_read's feature is included in
    // VkPhysicalDeviceVulkan14Features
    toQuery.fDynamicRenderingLocalRead = false;
    // VK_EXT_host_image_copy's feature is included in VkPhysicalDeviceVulkan14Features
    toQuery.fHostImageCopy = false;
}

// Check for support via individual extensions' feature structs (none of which have been
// promoted to core).
toAdd.fRasterizationOrderAttachmentAccess = exts.fRasterizationOrderAttachmentAccessARM ||
                                            exts.fRasterizationOrderAttachmentAccessEXT;
toAdd.fBlendOperationAdvanced = exts.fBlendOperationAdvancedEXT;
```
