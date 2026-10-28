---
name: skia/extract-complex-lambdas-to-helpers
description: When implementing multi-line helper logic inside a function, extract the routine into an anonymous-namespace helper rather than declaring a complex local lambda
rubric: design
severity: nit
---

**Rule.** Extract non-trivial or multi-line local lambdas into file-static or
anonymous-namespace helper functions instead of inlining them inside member
functions. Pass explicit arguments rather than capturing outer local state by
reference ([&]).

**Why.** Inlining multi-line lambdas inside member functions inflates function
length, increases nesting depth, and obscures caller control flow. Extracting
helpers to an anonymous namespace keeps functions concise and makes data
dependencies explicit by eliminating implicit reference captures.

## Examples

Bad:

```cpp
void VulkanCommandBuffer::recordTextureAndSamplerDescSet(...) {
    ...
    auto populateWriteInfo = [&](const VulkanTexture* texture,
                                 const VulkanSampler* sampler,
                                 int binding) {
        VkDescriptorImageInfo& textureInfo = descriptorImageInfos.push_back();
        textureInfo = {};
        textureInfo.sampler = sampler ? sampler->vkSampler() : VK_NULL_HANDLE;
        textureInfo.imageView =
                texture->getImageView(VulkanImageView::Usage::kShaderInput)->imageView();
        textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet& writeInfo = writeDescriptorSets.push_back();
        writeInfo = {};
        writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeInfo.dstSet = *set->descriptorSet();
        writeInfo.dstBinding = binding;
        writeInfo.descriptorCount = 1;
        writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeInfo.pImageInfo = &textureInfo;
    };
    ...
}
```

Good:

```cpp
namespace {
void populate_write_info(VulkanDescriptorSet* set,
                         TArray<VkDescriptorImageInfo>& descriptorImageInfos,
                         TArray<VkWriteDescriptorSet>& writeDescriptorSets,
                         const VulkanTexture* texture,
                         const VulkanSampler* sampler,
                         int32_t binding) {
    SkASSERT(set);
    VkDescriptorImageInfo& textureInfo = descriptorImageInfos.push_back();
    textureInfo = {};
    textureInfo.sampler = sampler ? sampler->vkSampler() : VK_NULL_HANDLE;
    textureInfo.imageView =
            texture->getImageView(VulkanImageView::Usage::kShaderInput)->imageView();
    textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet& writeInfo = writeDescriptorSets.push_back();
    writeInfo = {};
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.dstSet = *set->descriptorSet();
    writeInfo.dstBinding = binding;
    writeInfo.descriptorCount = 1;
    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeInfo.pImageInfo = &textureInfo;
}
}  // namespace
```
