---
name: skia/gpu/validate-render-target-usage-flags-in-caps
description: When validating backend texture renderability in Graphite Caps, verify all required backend usage flags like color and input attachments to fail incompatible wrapped textures early
rubric: correctness
severity: should_fix
---

**Rule.** In backend Caps renderability implementations (such as
Caps::isRenderable or getTextureUsage), validate that client-provided backend
texture info contains all required render target usage flags—such as
VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT and VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT in
Vulkan, or wgpu::TextureUsage::RenderAttachment in Dawn. Do not determine
renderability based solely on format and sample count support.

**Why.** Graphite validates wrapped external backend textures via
Caps::isRenderable during surface creation (e.g., in
SkSurfaces::WrapBackendTexture). If backend Caps fails to check necessary usage
flags (such as input attachment usage needed for subpass reads), incompatible
wrapped textures pass initial validation and cause GPU validation layer errors
or driver crashes when rendered to.

## Examples

Bad:

```cpp
bool VulkanCaps::isRenderable(const VulkanTextureInfo& vkInfo) const {
    const FormatInfo& info = this->getFormatInfo(vkInfo.fFormat);
    // Bad: Only checks format capability, ignoring required Vulkan usage flags.
    return info.isRenderable(vkInfo.fImageTiling, vkInfo.fSampleCount);
}
```

Good:

```cpp
bool VulkanCaps::isRenderable(const VulkanTextureInfo& vkInfo) const {
    const FormatInfo& info = this->getFormatInfo(vkInfo.fFormat);
    // Good: Verifies format support and all required render target usage flags.
    return info.isRenderable(vkInfo.fImageTiling, vkInfo.fSampleCount) &&
           SkToBool(vkInfo.fImageUsageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
}
```
