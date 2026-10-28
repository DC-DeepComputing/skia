---
name: skia/gpu/guard-vulkan-extension-flags-in-create-info
description: When setting Vulkan extension- or feature-dependent flags in create info structures, guard them with backend capability checks rather than setting them unconditionally
rubric: correctness
severity: should_fix
---

**Rule.** When setting Vulkan extension- or optional feature-dependent flags in
create info structures (such as VkGraphicsPipelineCreateInfo, VkImageCreateInfo,
or VkBufferCreateInfo), do not set them unconditionally. Guard feature-dependent
flags with backend capability queries (e.g., gpu->vkCaps().supports...() or
caps->supports...()) to ensure the feature is supported and enabled on the
VkDevice before specifying the flag.

**Why.** Under the Vulkan specification, passing extension- or optional
feature-dependent creation flags is only valid when the corresponding feature or
extension is supported and enabled on the VkDevice. Setting extension flags
unconditionally causes Vulkan validation layer errors and pipeline or resource
creation failures on devices and drivers where the feature is unsupported or
disabled.

## Examples

Bad:

```cpp
pipelineCreateInfo.flags = VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT;
```

Good:

```cpp
pipelineCreateInfo.flags = 0;
if (gpu->vkCaps().supportsPipelineCreationCacheControl()) {
    pipelineCreateInfo.flags |=
            VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT;
}
```
