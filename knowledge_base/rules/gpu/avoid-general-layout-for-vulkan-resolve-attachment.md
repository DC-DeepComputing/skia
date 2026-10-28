---
name: skia/gpu/avoid-general-layout-for-vulkan-resolve-attachment
description: When configuring Vulkan image layouts for passes reading dst as input, keep resolve attachments in optimal attachment layouts instead of transitioning to VK_IMAGE_LAYOUT_GENERAL
rubric: correctness
severity: should_fix
---

**Rule.** Do not transition or configure Vulkan resolve attachments to
VK_IMAGE_LAYOUT_GENERAL when enabling destination texture reads as shader input.
Intra-pass destination reads sample exclusively from the active color
attachment, so keep resolve attachments in
VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL or their optimal layout.

**Why.** In multisampled passes, shaders reading the destination buffer as an
input attachment sample directly from the multisampled color attachment rather
than the resolve attachment. Transitioning the resolve attachment to
VK_IMAGE_LAYOUT_GENERAL is unnecessary and disables optimal hardware layouts and
compression on textures that are only written during pass resolution.

## Examples

Bad:

```cpp
resolveTexture.updateImageLayout(rp_must_support_reading_dst_as_input(rpDesc)
        ? VK_IMAGE_LAYOUT_GENERAL
        : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
```

Good:

```cpp
// Resolve attachments are not sampled as destination input attachments during drawing.
resolveTexture.updateImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
```
