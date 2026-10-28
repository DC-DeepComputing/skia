---
name: skia/gpu/avoid-redundant-entity-parameters
description: When managing shared GPU resources or render pass targets, decouple shared samplers from texture pairs and pass requirement flags instead of redundant entity pointers
rubric: design
severity: should_fix
---

**Rule.** Do not store shared GPU resources (such as samplers) in coupled pairs
with specific texture sources (e.g., std::pair<Texture*, Sampler*>) or pass
redundant entity pointers in method signatures when the underlying texture is
already provided by another parameter. Decouple shared resources into standalone
managed members (e.g., fDstSampler) and pass only distinct configuration state
or boolean requirement flags (such as requiresDstSampler).

**Why.** Coupling shared samplers with texture pointers duplicates member state,
complicates method signatures, and forces callers to construct and pass
redundant resource objects. Decoupling the shared sampler allows the command
buffer to centralize resource management across render passes, ensures a single
source of truth for target attachments, and simplifies task APIs to a simple
requirement flag.

## Examples

Bad:

```cpp
// In CommandBuffer:
std::pair<const Texture*, const Sampler*> fDstCopy;
std::pair<const Texture*, const Sampler*> fDstTextureSampler;

bool addRenderPass(const RenderPassDesc&,
                   sk_sp<Texture> colorTexture,
                   const Texture* dstCopy,
                   std::pair<const Texture*, sk_sp<Sampler>> dstTextureSampler,
                   const DrawPassList& drawPasses);
```

Good:

```cpp
// In CommandBuffer:
const Texture* fDstCopy;
const Sampler* fDstSampler;

bool addRenderPass(const RenderPassDesc&,
                   sk_sp<Texture> colorTexture,
                   const Texture* dstCopy,
                   bool requiresDstSampler,
                   const DrawPassList& drawPasses);
```
