---
name: skia/gpu/restrict-intel-sample-counts-to-single-sample
description: When configuring supported sample counts in GPU backend Caps, restrict Intel GPUs to single-sample counts to avoid severe MSAA regressions
rubric: safety
severity: should_fix
---

**Rule.** Do not report multisample counts (greater than 1) as supported on
Intel GPUs when populating supported sample counts or format tables in GPU
backend Caps (such as VulkanCaps, GrVkCaps, or GrGLCaps). Mask Intel GPUs to
single-sample counts (SampleCount::k1 or VK_SAMPLE_COUNT_1_BIT) or enable
fAvoidMSAA to prevent severe performance regressions.

**Why.** MSAA causes severe performance degradation and driver instability on
Intel GPUs across multiple hardware generations (crbug.com/527565,
crbug.com/983926). Even if the underlying graphics driver advertises multisample
capabilities, Skia Caps must restrict Intel hardware to single-sample
operations.

## Examples

Bad:

```cpp
// Fails to mask sample counts on Intel GPUs, advertising slow MSAA
VkSampleCountFlags sampleCounts = properties.sampleCounts &
        (VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_2_BIT | VK_SAMPLE_COUNT_4_BIT |
         VK_SAMPLE_COUNT_8_BIT | VK_SAMPLE_COUNT_16_BIT);
```

Good:

```cpp
VkSampleCountFlags sampleCounts = properties.sampleCounts &
        (VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_2_BIT | VK_SAMPLE_COUNT_4_BIT |
         VK_SAMPLE_COUNT_8_BIT | VK_SAMPLE_COUNT_16_BIT);
if (vendorID == skgpu::kIntel_VkVendor) {
    // MSAA doesn't work well on Intel GPUs (crbug.com/527565, crbug.com/983926)
    sampleCounts &= VK_SAMPLE_COUNT_1_BIT;
}
```
