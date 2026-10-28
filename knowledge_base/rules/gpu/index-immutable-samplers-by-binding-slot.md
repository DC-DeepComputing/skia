---
name: skia/gpu/index-immutable-samplers-by-binding-slot
description: When retrieving samplers during Graphite command buffer recording, use a getSampler() helper fetching dynamic samplers from GlobalCache and indexing pipeline immutable samplers by slot
rubric: design
severity: should_fix
---

**Rule.** In Graphite CommandBuffer implementations, retrieve samplers through a
dedicated getSampler() helper instead of querying ResourceProvider directly. In
getSampler(), return pre-cached dynamic samplers from GlobalCache without
per-command buffer lifetime tracking, and query immutable samplers directly from
the active GraphicsPipeline indexed by binding slot rather than searching by
SamplerDesc. Register only pipeline immutable samplers with trackResource().

**Why.** Dynamic samplers are cached in GlobalCache for the context lifetime,
making per-command-buffer tracking and ResourceProvider lookups redundant.
Aligning pipeline immutable samplers with draw command binding slots allows
direct O(1) indexing on the recording hot path while ensuring only pipeline
resources are tracked with trackResource().

## Examples

Bad:

```cpp
// Bad: Querying ResourceProvider directly or searching descriptors linearly
const Sampler* VulkanCommandBuffer::getSampler(SamplerDesc desc) {
    if (desc.isImmutable()) {
        const VulkanSampler* s = fActiveGraphicsPipeline->findImmutableSampler(desc);
        if (s) { this->trackResource(sk_ref_sp<Sampler>(s)); }
        return s;
    }
    auto s = fResourceProvider->findOrCreateCompatibleSampler(desc);
    this->trackResource(s);
    return s.get();
}
```

Good:

```cpp
// Good: Fetch dynamic samplers from GlobalCache and index immutable samplers by slot
const Sampler* VulkanCommandBuffer::getSampler(
        const DrawPassCommands::BindTexturesAndSamplers* command, int32_t index) {
    auto desc = command->fSamplers[index];
    if (desc.isImmutable()) {
        const VulkanSampler* s = fActiveGraphicsPipeline->immutableSampler(index);
        if (s) { this->trackResource(sk_ref_sp<Sampler>(s)); }
        return s;
    }
    return fSharedContext->globalCache()->getDynamicSampler(desc);
}
```
