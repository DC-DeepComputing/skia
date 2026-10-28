---
name: skia/gpu/document-reusable-requires-purgeable
description: When setting reusableRequiresPurgeable = true on Graphite Resource subclasses, document the immediate CPU mutation or out-of-order execution hazard requiring it
rubric: readability
severity: nit
---

**Rule.** When initializing or overriding reusableRequiresPurgeable = true on a
Graphite Resource (or Buffer) subclass, document the rationale with an
explanatory comment. Specifically explain if the resource can be modified
immediately on the CPU or via synchronous driver calls during Skia API calls
outside sequential command buffer execution, making reuse unsafe while GPU
command buffer references remain active.

**Why.** Graphite resources default to reusableRequiresPurgeable = false,
allowing immediate cache reuse once CPU usage references reach zero even while
in-flight GPU command buffers still hold references. Documenting the non-obvious
immediate mutation or out-of-order execution hazard prevents future maintainers
from removing the purgeable requirement or causing host-GPU data races.

## Examples

Bad:

```cpp
VulkanDescriptorSet::VulkanDescriptorSet(const VulkanSharedContext* ctxt,
                                         VkDescriptorSet set,
                                         sk_sp<VulkanDescriptorPool> pool)
        : Resource(ctxt,
                   Ownership::kOwned,
                   /*gpuMemorySize=*/0,
                   /*reusableRequiresPurgeable=*/true)
```

Good:

```cpp
VulkanDescriptorSet::VulkanDescriptorSet(const VulkanSharedContext* ctxt,
                                         VkDescriptorSet set,
                                         sk_sp<VulkanDescriptorPool> pool)
        : Resource(ctxt,
                   Ownership::kOwned,
                   /*gpuMemorySize=*/0,
                   // Explicitly set reusableRequiresPurgeable because this object may be modified
                   // by VulkanCommandBuffer immediately during Skia API calls, outside of the flow
                   // of command buffer execution.
                   /*reusableRequiresPurgeable=*/true)
```
