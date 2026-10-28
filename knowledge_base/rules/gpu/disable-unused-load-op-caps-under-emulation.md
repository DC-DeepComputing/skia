---
name: skia/gpu/disable-unused-load-op-caps-under-emulation
description: When backend caps enable load/store/resolve emulation in Graphite, disable native resolve load-op capabilities to prevent precompiling redundant pipeline permutations
rubric: safety
severity: nit
---

**Rule.** When Graphite backend capabilities enable load/store/resolve emulation
(such as fEmulateLoadStoreResolve), explicitly disable native resolve load-op
capabilities (such as clearing fSupportedResolveTextureLoadOp). Do not leave
resolve load ops enabled when passes are emulated, which causes
loadOpAffectsMSAAPipelines() to return true and forces precompilation to
generate dead pipeline permutations.

**Why.** Under load/store/resolve emulation, native resolve load operations
(such as ExpandResolveTexture) are never executed. Leaving load-op capabilities
enabled causes Caps::loadOpAffectsMSAAPipelines() to report true, prompting
precompilation to generate duplicate pipeline variants for unused load ops and
wasting compilation time and driver memory.

## Examples

Bad:

```cpp
if (!fSupportsPartialLoadResolve && fSupportedTransientAttachmentUsage == wgpu::TextureUsage::None) {
    fEmulateLoadStoreResolve = true;
    fDifferentResolveAttachmentSizeSupport = true;
}
```

Good:

```cpp
if (!fSupportsPartialLoadResolve && fSupportedTransientAttachmentUsage == wgpu::TextureUsage::None) {
    fEmulateLoadStoreResolve = true;
    fDifferentResolveAttachmentSizeSupport = true;
    // Under emulation, ExpandResolveTexture isn't used; disabling it prevents
    // the precompilation API from generating duplicate pipeline permutations.
    fSupportedResolveTextureLoadOp = std::nullopt;
}
```
