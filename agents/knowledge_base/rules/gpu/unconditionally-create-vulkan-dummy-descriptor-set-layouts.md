---
name: skia/gpu/unconditionally-create-vulkan-dummy-descriptor-set-layouts
description: When handling Vulkan dummy descriptor set layouts, create dummy layouts for pipeline layouts, but reject creating descriptor pools or allocating sets for them
rubric: design
severity: should_fix
---

**Rule.** When configuring Vulkan pipeline layouts where descriptor sets occupy
fixed slot indices, unconditionally create dummy VkDescriptorSetLayout handles
for empty resource lists rather than skipping layout creation. However, do not
support creating descriptor pools (VkDescriptorPool) or allocating descriptor
sets for empty or dummy placeholder layouts; reject empty requested descriptor
counts in pool creation (such as returning nullptr from
VulkanDescriptorPool::Make).

**Why.** In Vulkan, pipeline layouts require placeholder descriptor set layouts
to preserve fixed slot indices and pipeline compatibility across draws. However,
because empty dummy layouts never bind resources, allocating descriptor sets or
creating descriptor pools with empty pool sizes adds unnecessary complexity and
risks masking bugs where code mistakenly attempts to allocate sets for
placeholder layouts.

## Examples

Bad:

```cpp
// Pipeline layout: skipping dummy layout for empty uniform descriptors shifts slot indices
if (!uniformDescriptors.empty() || numInputAttachments == 0) {
    VkDescriptorSetLayout uniformSetLayout =
            descriptor_data_to_layout(sharedContext, {uniformDescriptors});
    setLayouts.push_back(uniformSetLayout);
}

// Descriptor pool: allowing empty pools for dummy "filler" layouts masks bugs
createInfo.pPoolSizes = requestedDescCounts.empty() ? nullptr : &poolSizes.front();
```

Good:

```cpp
// Pipeline layout: unconditionally create dummy layout so slot indices match
VkDescriptorSetLayout uniformSetLayout =
        descriptor_data_to_layout(sharedContext, {uniformDescriptors});
setLayouts.push_back(uniformSetLayout);

// Descriptor pool: reject empty descriptor counts; dummy layouts never need pools/sets
if (requestedDescCounts.empty()) {
    return nullptr;
}
createInfo.pPoolSizes = &poolSizes.front();
```
