---
name: skia/gpu/set-reusable-requires-purgeable-for-host-mutated-resources
description: When constructing Graphite Resource subclasses, explicitly pass caching policy arguments like reusableRequiresPurgeable and label to the base constructor instead of relying on default values
rubric: readability
severity: should_fix
---

**Rule.** When constructing Graphite Resource subclasses, explicitly pass
resource caching and lifecycle arguments (specifically label and
reusableRequiresPurgeable) to the base Resource constructor rather than relying
on default parameter values. Set reusableRequiresPurgeable = true (or compute it
dynamically from usage flags) for resources mutated out-of-band or modified on
the host (such as VulkanDescriptorSet or CPU-mappable buffers) to prevent cache
reuse during in-flight GPU execution; explicitly pass false for resources
accessed exclusively through sequential GPU commands.

**Why.** Graphite's ResourceCache inspects reusableRequiresPurgeable to
determine whether resources can be reused immediately when CPU usage refs drop
to zero while GPU command buffer refs remain active. Relying on default
arguments obscures the intended lifecycle policy at call sites and risks
host-mutated resources inadvertently defaulting to false, causing concurrent
host-GPU data races and visual corruption.

## Examples

Bad:

```cpp
// Bad: Relying on default constructor parameters obscures lifecycle policy, or defaults
// to false when the resource can be mutated out of band on the host.
VulkanYcbcrConversion::VulkanYcbcrConversion(const VulkanSharedContext* context,
                                             VkSamplerYcbcrConversion ycbcrConversion,
                                             std::optional<VkFilter> requiredFilter)
        : Resource(context,
                   Ownership::kOwned,
                   /*gpuMemorySize=*/0) {}

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanSharedContext* ctxt,
                                         VkDescriptorSet set,
                                         sk_sp<VulkanDescriptorPool> pool)
        : Resource(ctxt,
                   Ownership::kOwned,
                   /*gpuMemorySize=*/0) {}
```

Good:

```cpp
// Good: Explicitly pass label and reusableRequiresPurgeable to base Resource constructor.
VulkanYcbcrConversion::VulkanYcbcrConversion(const VulkanSharedContext* context,
                                             VkSamplerYcbcrConversion ycbcrConversion,
                                             std::optional<VkFilter> requiredFilter)
        : Resource(context,
                   Ownership::kOwned,
                   /*gpuMemorySize=*/0,
                   /*label=*/{},
                   /*reusableRequiresPurgeable=*/false) {}

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanSharedContext* ctxt,
                                         VkDescriptorSet set,
                                         sk_sp<VulkanDescriptorPool> pool)
        : Resource(ctxt,
                   Ownership::kOwned,
                   /*gpuMemorySize=*/0,
                   /*label=*/{},
                   /*reusableRequiresPurgeable=*/true) {}
```
