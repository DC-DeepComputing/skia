---
name: skia/gpu/avoid-dummy-vulkan-descriptor-sets
description: When recording Vulkan commands or allocating descriptor sets for draws requiring no descriptors, exit early rather than creating or binding dummy empty descriptor sets
rubric: correctness
severity: blocking
---

**Rule.** When recording Vulkan draw commands or managing resource bindings
where a pipeline set slot has no active descriptors (such as empty uniform
buffers or samplers), return early and do not create or bind a dummy descriptor
set. Do not allocate descriptor sets for zero descriptors, as Vulkan requires
descriptor pool sizes to be non-zero.

**Why.** In Vulkan, shaders only require descriptor sets to be bound for set
indices that are statically referenced by active pipeline stages. Allocating a
descriptor set for zero descriptors requires creating a VkDescriptorPool with
poolSizeCount == 0, which violates the Vulkan specification and triggers
validation layer errors.

## Examples

Bad:

```cpp
// BAD: Allocates and binds a dummy descriptor set even when no descriptors are needed
sk_sp<VulkanDescriptorSet> descSet = fResourceProvider->findOrCreateDescriptorSet(descriptors);
fCommandBuffer->bindDescriptorSet(descSet, kUniformSetIdx);
```

Good:

```cpp
// GOOD: Return early when no descriptors are needed, avoiding invalid zero-sized pool allocation
if (descriptors.empty()) {
    return;
}
sk_sp<VulkanDescriptorSet> descSet = fResourceProvider->findOrCreateDescriptorSet(descriptors);
fCommandBuffer->bindDescriptorSet(descSet, kUniformSetIdx);
```
