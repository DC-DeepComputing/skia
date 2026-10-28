---
name: skia/gpu/assert-compile-time-sizes-against-spec-minimums
description: When fixed-size GPU resource allocations have compile-time sizes and API specifications guarantee minimum limits, use static_assert instead of tracking dynamic Caps capabilities
rubric: readability
severity: should_fix
---

**Rule.** When GPU resource allocations (such as push constants or uniform
blocks) have fixed sizes known at compile time and the underlying graphics API
specification guarantees a baseline minimum limit across all conforming
implementations (e.g., Vulkan's 128-byte push constant guarantee), validate the
size constraint at build time using static_assert rather than tracking,
querying, and checking dynamic runtime capability limits in Caps. Place the
static_assert alongside the backend resource provider or caps initialization.

**Why.** Querying driver limits dynamically and storing capability members in
Caps adds unnecessary runtime overhead, code bloat, and defensive branching when
data structures are fixed at compile time and guaranteed to fit within
specification baselines. A static_assert validates the constraint across all
build targets at compile time and clearly documents the specification invariant
without runtime cost.

## Examples

Bad:

```cpp
// In Caps.h / VulkanCaps.cpp
class VulkanCaps : public Caps {
public:
    uint32_t maxPushConstantSize() const { return fMaxPushConstantSize; }
private:
    uint32_t fMaxPushConstantSize;
};

// Populating Caps dynamically from driver properties:
fMaxPushConstantSize = std::min(physDevProperties.limits.maxPushConstantsSize, (uint32_t)INT_MAX);
```

Good:

```cpp
// In VulkanCaps.cpp / VulkanResourceProvider.cpp
// Push constant sizes are known at compile time and Vulkan guarantees at least 128 bytes per spec;
// enforce with static_assert rather than adding dynamic Caps queries.
static_assert(VulkanResourceProvider::kIntrinsicConstantSize < 128,
              "Intrinsic push constants exceed Vulkan spec-mandated 128-byte minimum");
static_assert(VulkanResourceProvider::kLoadMSAAPushConstantSize < 128,
              "MSAA load push constants exceed Vulkan spec-mandated 128-byte minimum");
```
