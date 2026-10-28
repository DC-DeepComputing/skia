---
name: skia/gpu/describe-vulkan-extension-features-as-main-not-mandatory
description: When commenting on Vulkan capability dependencies in Caps, describe extension-provided features as main features rather than mandatory features to avoid implying client requirements
rubric: readability
severity: nit
---

**Rule.** When documenting Vulkan capability dependencies in capability
initialization (such as VulkanCaps), avoid describing extension features as
'mandatory'. Refer to them as the 'main feature' of the extension to distinguish
Vulkan driver specification requirements from client application requirements in
Skia.

**Why.** Describing a Vulkan extension feature as 'mandatory' misleadingly
implies that Skia requires client applications to enable the feature. In Skia,
capability initialization only checks whether the client opted to enable the
feature on the device; 'main feature' accurately identifies the primary feature
provided by an extension without implying a mandatory client requirement.

## Examples

Bad:

```cpp
// Vertex input dynamic state depends on the mandatory feature of
// VK_EXT_vertex_input_dynamic_state.
fUseVertexInputDynamicState = enabledFeatures.fVertexInputDynamicState;
```

Good:

```cpp
// Vertex input dynamic state depends on the main feature of
// VK_EXT_vertex_input_dynamic_state.
fUseVertexInputDynamicState = enabledFeatures.fVertexInputDynamicState;
```
