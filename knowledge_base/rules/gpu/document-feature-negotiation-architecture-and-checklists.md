---
name: skia/gpu/document-feature-negotiation-architecture-and-checklists
description: When implementing modules managing complex feature negotiation and API version promotions, document internal architecture and maintainer checklists for adding features or updating versions
rubric: readability
severity: should_fix
---

**Rule.** In classes or modules managing multi-stage feature negotiation,
version promotions, and pNext chains, provide top-level architecture
documentation in the implementation file. Detail the internal discovery and
enablement pipeline, and include explicit step-by-step checklists for
maintainers adding support for new extensions or updating supported core API
versions.

**Why.** Multi-stage feature negotiation involves interdependent data
structures, version-dependent promotion logic, and validation layer constraints.
Top-level architectural overviews and step-by-step maintainer runbooks prevent
developers from missing required touchpoints—such as updating query structs,
enablement routines, and test coverage—when introducing new extensions or
bumping API versions.

## Examples

Bad:

```cpp
// src/gpu/vk/VulkanPreferredFeatures.cpp
// Implementation file defining multi-stage feature negotiation without top-level
// architecture notes or maintainer checklists for adding extensions and versions.
void VulkanPreferredFeatures::addFeaturesToQuery(...) { ... }
```

Good:

```cpp
// src/gpu/vk/VulkanPreferredFeatures.cpp
/*
 * # Overview
 * VulkanPreferredFeatures modifies requested extensions and device features...
 *
 * # How It Works
 * - Add instance extensions, query device features, enable device features...
 *
 * # How to Add Support for New Extensions
 * - Add flag to DeviceExtensions.
 * - Add feature struct to VulkanPreferredFeatures.
 * - Update addFeaturesToQuery() and addFeaturesToEnable().
 * - Update tests in tests/VkPreferredFeaturesTest.cpp.
 *
 * # How to Add Support for New Vulkan Versions
 * - Add VkPhysicalDeviceVulkanXXFeatures struct.
 * - Check Vulkan version in get_features_to_query().
 */
```
