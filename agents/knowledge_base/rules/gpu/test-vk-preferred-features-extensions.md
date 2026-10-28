---
name: skia/gpu/test-vk-preferred-features-extensions
description: When adding Vulkan extensions or feature structs in VulkanPreferredFeatures, update tests/VkPreferredFeaturesTest.cpp to verify feature negotiation and promotion across Vulkan versions
rubric: testing
severity: should_fix
---

**Rule.** When adding support for a new Vulkan extension or feature struct in
VulkanPreferredFeatures, update tests/VkPreferredFeaturesTest.cpp to verify that
the extension and its features are negotiated correctly. Test that the extension
and struct are enabled when available, omitted when unavailable, and properly
promoted or suppressed across all supported Vulkan versions and client-provided
pNext feature chains.

**Why.** VulkanPreferredFeatures dynamically negotiates extension and feature
struct chains based on device support, Vulkan core version promotions, and
client overrides. Updating tests/VkPreferredFeaturesTest.cpp ensures that new
extensions and feature structs are enabled or promoted without violating Vulkan
specification rules against duplicate or conflicting pNext structures.

## Examples

Bad:

```cpp
// In src/gpu/vk/VulkanPreferredFeatures.cpp:
// Added support for VK_EXT_frame_boundary and VkPhysicalDeviceFrameBoundaryFeaturesEXT
// without updating tests/VkPreferredFeaturesTest.cpp.
```

Good:

```cpp
// In tests/VkPreferredFeaturesTest.cpp:
struct VulkanExts {
    ...
    bool fFrameBoundaryEXT = true;
};
...
DEF_TEST(VkPreferredFeaturesTest_BasicVulkan11, reporter) {
    ...
    CHECK_EXT_ENABLED(VK_EXT_FRAME_BOUNDARY);
    CHECK_FEATURE_ENABLED(FRAME_BOUNDARY_FEATURES, FrameBoundaryFeatures, frameBoundary);
}
```
