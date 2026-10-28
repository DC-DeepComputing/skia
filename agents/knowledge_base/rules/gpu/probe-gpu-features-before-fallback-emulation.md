---
name: skia/gpu/probe-gpu-features-before-fallback-emulation
description: When configuring GPU backend capabilities in Caps, probe native device feature extensions first and gate emulation fallbacks on their absence rather than hardcoding backend checks
rubric: correctness
severity: should_fix
---

**Rule.** When initializing GPU backend capabilities (such as in Caps
subclasses), probe driver and device feature extensions first to set native
capability flags, and only enable emulation or fallback logic when those native
features are unsupported. Do not predetermine emulation by hardcoding backend
API or device type checks.

**Why.** Hardcoding backend or device checks (such as backendType != D3D11)
creates brittle fallback logic that can skip supported native features or fail
if an individual device lacks driver support. Probing native features first
ensures emulation cleanly acts as a fallback only when native support is absent.

## Examples

Bad:

```cpp
if (fSupportedTransientAttachmentUsage == wgpu::TextureUsage::None) {
#if !defined(__EMSCRIPTEN__)
    fEmulateLoadStoreResolve = info.backendType != wgpu::BackendType::D3D11;
#else
    fEmulateLoadStoreResolve = true;
#endif
    fDifferentResolveAttachmentSizeSupport = true;
}

#if !defined(__EMSCRIPTEN__)
if (!fEmulateLoadStoreResolve &&
    backendContext.fDevice.HasFeature(wgpu::FeatureName::DawnLoadResolveTexture)) {
    fSupportedResolveTextureLoadOp = wgpu::LoadOp::ExpandResolveTexture;
    fSupportsPartialLoadResolve =
            backendContext.fDevice.HasFeature(wgpu::FeatureName::DawnPartialLoadResolveTexture);
}
#endif
```

Good:

```cpp
#if !defined(__EMSCRIPTEN__)
if (backendContext.fDevice.HasFeature(wgpu::FeatureName::DawnLoadResolveTexture)) {
    fSupportedResolveTextureLoadOp = wgpu::LoadOp::ExpandResolveTexture;
    fSupportsPartialLoadResolve =
            backendContext.fDevice.HasFeature(wgpu::FeatureName::DawnPartialLoadResolveTexture);
    fDifferentResolveAttachmentSizeSupport = fSupportsPartialLoadResolve;
}
#endif

if (!fSupportsPartialLoadResolve && fSupportedTransientAttachmentUsage == wgpu::TextureUsage::None) {
    fEmulateLoadStoreResolve = true;
    fDifferentResolveAttachmentSizeSupport = true;
    fSupportedResolveTextureLoadOp = std::nullopt;
}
```
