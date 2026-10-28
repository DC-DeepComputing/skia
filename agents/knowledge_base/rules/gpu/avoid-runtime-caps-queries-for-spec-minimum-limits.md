---
name: skia/gpu/avoid-runtime-caps-queries-for-spec-minimum-limits
description: When validating GPU payload sizes within specification minimums, use compile-time static_assert instead of adding runtime Caps query methods or limit tracking
rubric: design
severity: should_fix
---

**Rule.** Do not add runtime Caps query methods or state tracking for fixed-size
GPU structures or payloads whose sizes are guaranteed by the graphics API
specification (such as Vulkan's 128-byte push constant guarantee). Instead,
validate compile-time structure sizes against specification minimum limits using
static_assert until variable-sized payloads or dynamic requirements actually
necessitate runtime querying.

**Why.** Adding runtime Caps queries and device limit tracking for compile-time
constants introduces speculative dead code and unnecessary runtime complexity.
Graphics API specifications mandate minimum guaranteed limits across all
conforming hardware, making static_assert sufficient to catch oversized payloads
at build time.

## Examples

Bad:

```cpp
// VulkanCaps.h
bool pushConstantSizeWithinDeviceLimits(uint32_t size) const {
    return size <= fMaxPushConstantSize;
}
```

Good:

```cpp
// VulkanCaps.cpp
// Assert that push constant sizes are below the maximum allowed (which is guaranteed to be
// at least 128 bytes per spec).
static_assert(VulkanResourceProvider::kIntrinsicConstantSize < 128 &&
              VulkanResourceProvider::kLoadMSAAPushConstantSize < 128);
```
