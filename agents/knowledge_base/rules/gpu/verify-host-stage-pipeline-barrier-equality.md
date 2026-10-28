---
name: skia/gpu/verify-host-stage-pipeline-barrier-equality
description: When bypassing Vulkan pipeline barriers for host-synchronized memory access, check equality with VK_PIPELINE_STAGE_HOST_BIT rather than inequality to preserve GPU barriers
rubric: correctness
severity: should_fix
---

**Rule.** When omitting or bypassing Vulkan pipeline memory barriers for
host-synchronized memory stages, ensure the condition checks equality with
VK_PIPELINE_STAGE_HOST_BIT (e.g., srcStageMask == VK_PIPELINE_STAGE_HOST_BIT)
before clearing barrier requirements. Never test inequality (!=), which
erroneously disables synchronization for GPU-to-GPU stages and preserves
barriers only for host transitions.

**Why.** Host memory writes on the CPU are implicitly synchronized upon Vulkan
command buffer submission, making pipeline barriers transitioning from
VK_PIPELINE_STAGE_HOST_BIT unnecessary. Inverting the check with inequality
skips barriers for all GPU access stages while enforcing them only for host
transitions, resulting in data races and GPU-to-GPU desynchronization.

## Examples

Bad:

```cpp
// Host writes are implicitly synchronized on submission, so omit host barriers.
if (srcStageMask != VK_PIPELINE_STAGE_HOST_BIT) {
    needsBarrier = false;
}
```

Good:

```cpp
// Host writes are implicitly synchronized on submission, so omit host barriers.
if (srcStageMask == VK_PIPELINE_STAGE_HOST_BIT) {
    needsBarrier = false;
}
```
