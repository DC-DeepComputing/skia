---
name: skia/gpu/add-texture-binding-for-dawn-resolve-color-attachments
description: When configuring Dawn attachment texture usage flags under load/store resolve emulation, include TextureBinding only for color formats and exclude depth/stencil formats
rubric: correctness
severity: should_fix
---

**Rule.** When configuring Dawn attachment texture usage (wgpu::TextureUsage),
add wgpu::TextureUsage::TextureBinding if emulating load/store resolve
(fEmulateLoadStoreResolve), but only for color attachments
(!TextureFormatIsDepthOrStencil). Do not add TextureBinding to depth or stencil
attachment usages.

**Why.** Emulating MSAA load/store resolve performs texture copies via shader
sampling in a separate render pass, requiring wgpu::TextureUsage::TextureBinding
on the sampled MSAA color attachment. Depth and stencil attachments are not
sampled during resolve emulation, so adding TextureBinding to them imposes
unnecessary usage constraints and can violate driver format requirements.

## Examples

Bad:

```cpp
DawnTextureInfo info;
info.fUsage = wgpu::TextureUsage::RenderAttachment;
if (fSupportedTransientAttachmentUsage != wgpu::TextureUsage::None &&
    discardable == Discardable::kYes) {
    info.fUsage |= fSupportedTransientAttachmentUsage;
}
return TextureInfos::MakeDawn(info);
```

Good:

```cpp
DawnTextureInfo info;
info.fUsage = wgpu::TextureUsage::RenderAttachment;
if (fSupportedTransientAttachmentUsage != wgpu::TextureUsage::None &&
    discardable == Discardable::kYes) {
    info.fUsage |= fSupportedTransientAttachmentUsage;
}
if (fEmulateLoadStoreResolve && !TextureFormatIsDepthOrStencil(desc.fFormat)) {
    // When emulating resolve, the MSAA color attachment is sampled into the resolve target.
    info.fUsage |= wgpu::TextureUsage::TextureBinding;
}
return TextureInfos::MakeDawn(info);
```
