---
name: skia/gpu/verify-vulkan-timestamp-period-before-enabling-stats
description: When enabling elapsed-time GPU statistics from Vulkan capabilities, verify that VkPhysicalDeviceLimits::timestampPeriod is greater than zero in addition to timestampComputeAndGraphics
rubric: correctness
severity: should_fix
---

**Rule.** When querying Vulkan device capabilities to enable timestamp or
elapsed-time statistics (such as GpuStatsFlags::kElapsedTime), verify that
VkPhysicalDeviceLimits::timestampPeriod is greater than zero in addition to
checking timestampComputeAndGraphics. Do not enable elapsed-time statistics
based solely on timestampComputeAndGraphics.

**Why.** In Vulkan, timestampPeriod specifies the number of nanoseconds per
timestamp query increment. If the driver does not support converting timestamps
to real time, timestampPeriod may be 0 even when timestampComputeAndGraphics is
true, causing elapsed-time calculations to report zero or invalid durations.

## Examples

Bad:

```cpp
if (deviceProperties.fBase.properties.limits.timestampComputeAndGraphics) {
    fSupportedGpuStats |= GpuStatsFlags::kElapsedTime;
}
```

Good:

```cpp
if (deviceProperties.fBase.properties.limits.timestampComputeAndGraphics &&
    fTimestampPeriod > 0) {
    fSupportedGpuStats |= GpuStatsFlags::kElapsedTime;
}
```
