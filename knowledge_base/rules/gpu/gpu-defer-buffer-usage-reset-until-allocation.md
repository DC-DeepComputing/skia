---
name: skia/gpu/gpu-defer-buffer-usage-reset-until-allocation
description: When allocating new GPU buffers or resources upon capacity exhaustion in resource managers, defer resetting usage tracking until after verifying the fallible allocation succeeded
rubric: correctness
severity: should_fix
---

**Rule.** When acquiring or allocating replacement GPU buffers or resources upon
capacity exhaustion, do not reset usage tracking state (such as slot counters or
byte offsets) before verifying that the allocation succeeded. Defer resetting or
updating usage counters until after checking for null or allocation failure.

**Why.** Resetting usage counters before confirming that fallible resource
creation succeeded leaves the manager in an inconsistent state on allocation
failure. The manager retains the existing exhausted buffer while reporting zero
usage, causing subsequent operations to incorrectly reuse the full buffer and
overwrite in-use data.

## Examples

Bad:

```cpp
auto dawnBuffer = resourceProvider->findOrCreateDawnBuffer(
        stride * IntrinsicBuffer::kNumSlots,
        BufferType::kUniform,
        AccessPattern::kGpuOnly,
        "IntrinsicConstantBuffer");
fSlotsUsed = 0; // Premature reset before verifying allocation

if (!dawnBuffer) {
    return {};
}
fCurrentBuffer = new IntrinsicBuffer(dawnBuffer);
```

Good:

```cpp
auto dawnBuffer = resourceProvider->findOrCreateDawnBuffer(
        stride * IntrinsicBuffer::kNumSlots,
        BufferType::kUniform,
        AccessPattern::kGpuOnly,
        "IntrinsicConstantBuffer");
if (!dawnBuffer) {
    return {}; // Prior usage tracking left intact on failure
}

fSlotsUsed = 0;
fCurrentBuffer = new IntrinsicBuffer(dawnBuffer);
```
