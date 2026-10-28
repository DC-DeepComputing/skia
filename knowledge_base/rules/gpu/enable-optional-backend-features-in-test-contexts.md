---
name: skia/gpu/enable-optional-backend-features-in-test-contexts
description: When adding GPU backend support for optional features or extensions, update test and window context harnesses to detect, chain, and allowlist them
rubric: correctness
severity: should_fix
---

**Rule.** When adding GPU backend caps or driver support for an optional
feature, extension, or alignment capability (such as a wgpu::FeatureName or
Vulkan extension), update counterpart test and window context harnesses (such as
AddPreferredFeatures, GraphiteDawnTestContext, VkTestUtils, or
VulkanPreferredFeatures). Ensure test context factories detect device support,
chain or request the feature struct during device creation, and add new
extension names to test extension allowlists (such as should_include_extension()
/ kExtensionsForTests in VkTestUtils) so unit tests and interactive tools
exercise the new path in CI.

**Why.** Skia test harnesses (DM, nanobench) and interactive tools (Viewer)
initialize backend devices only with optional features and extensions explicitly
requested at context creation. Without updating context creation helpers to
probe and enable the feature and permit the extension in test allowlists, test
suites run without the capability enabled, leaving new driver extensions and
backend code paths unexercised in CI.

## Examples

Bad:

```cpp
// In DawnCaps.cpp or GrVkCaps.cpp:
if (backendContext.fDevice.HasFeature(wgpu::FeatureName::DawnTexelCopyBufferRowAlignment)) {
    fTextureDataRowBytesAlignment = alignmentLimits.minTexelCopyBufferRowAlignment;
}
if (extensions->hasExtension(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME, 1)) {
    fPipelineCreationCacheControlSupport = true;
}
// In VkTestUtils.cpp setup_features(): chaining cacheControlFeatures
// but omitting VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME from should_include_extension().
```

Good:

```cpp
// In tools/graphite/dawn/GraphiteDawnToggles.cpp (AddPreferredFeatures):
if (adapter.HasFeature(wgpu::FeatureName::DawnTexelCopyBufferRowAlignment)) {
    features.push_back(wgpu::FeatureName::DawnTexelCopyBufferRowAlignment);
}

// In tools/gpu/vk/VkTestUtils.cpp (chaining feature AND allowlisting extension):
static bool should_include_extension(const char* extensionName) {
    const char* kExtensionsForTests[] = {
        ...
        VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME,
    };
    ...
}
```
