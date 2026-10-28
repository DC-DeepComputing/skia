---
name: skia/gpu/condition-vulkan-input-attachment-self-dependency
description: When configuring Vulkan render pass subpasses, only declare an intra-subpass self-dependency for input attachment reads if the subpass binds an input attachment
rubric: correctness
severity: should_fix
---

**Rule.** When constructing Vulkan render passes, only declare intra-subpass
self-dependencies (where srcSubpass equals dstSubpass) specifying
VK_ACCESS_INPUT_ATTACHMENT_READ_BIT on subpasses that actually bind and read an
input attachment. Do not declare input attachment read self-dependencies on
subpasses that do not bind an input attachment or only perform direct drawing
and resolve operations.

**Why.** Intra-subpass self-dependencies synchronize earlier attachment writes
with subsequent input attachment reads within the same subpass. Declaring this
dependency on a subpass that does not bind an input attachment mismatches the
VkSubpassDescription configuration and introduces unnecessary or invalid
pipeline synchronization overhead.

## Examples

Bad:

```cpp
// Unconditionally declaring input attachment self-dependency on a subpass
SubpassDependency& selfDependency = deps.push_back();
selfDependency.srcSubpass = mainSubpassIdx;
selfDependency.dstSubpass = mainSubpassIdx;
selfDependency.dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
selfDependency.dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
```

Good:

```cpp
// Only declare input attachment self-dependency when an input attachment is bound
if (subpassHasInputAttachment) {
    SubpassDependency& selfDependency = deps.push_back();
    selfDependency.srcSubpass = mainSubpassIdx;
    selfDependency.dstSubpass = mainSubpassIdx;
    selfDependency.dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    selfDependency.dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
}
```
