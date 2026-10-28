---
name: skia/gpu/document-chained-struct-member-lifetimes
description: When bundling structs into a container to manage pointer-chain lifetime dependencies, explicitly document which member fields are chained into the target struct
rubric: readability
severity: nit
---

**Rule.** When bundling structs or resources into a container to manage pointer
lifetime dependencies (such as Vulkan pNext chains), explicitly document which
member fields are chained into the target struct and must share its lifetime.
Avoid vague or generic scoping comments that describe arbitrary chained structs
rather than naming the specific container fields.

**Why.** Generic comments stating that chained structs must remain in scope
obscure why specific fields are grouped together. Explicitly naming which member
fields are chained into which target struct clarifies the purpose of the
container and documents exact lifetime dependencies for callers and maintainers.

## Examples

Bad:

```cpp
struct TestVkFeatures {
    VkPhysicalDeviceFeatures2 deviceFeatures;

    // Note: Any struct that CreateVkBackendContext may chain to VkPhysicalDeviceFeatures2
    // should remain in scope together with the physical device features.
    skgpu::VulkanPreferredFeatures skiaFeatures;
    VkPhysicalDeviceProtectedMemoryFeatures protectedMemoryFeatures;
};
```

Good:

```cpp
struct TestVkFeatures {
    VkPhysicalDeviceFeatures2 deviceFeatures;

    // protectedMemoryFeatures and structs from skiaFeatures may be chained into deviceFeatures,
    // so must share the same lifetime.
    skgpu::VulkanPreferredFeatures skiaFeatures;
    VkPhysicalDeviceProtectedMemoryFeatures protectedMemoryFeatures;
};
```
