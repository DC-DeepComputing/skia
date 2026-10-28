---
name: skia/gpu/use-vendor-suffix-for-device-extensions-flags
description: When toggling extension flags in VulkanPreferredFeatures's DeviceExtensions, assign to vendor-suffixed boolean members rather than string member names with the Extension suffix
rubric: correctness
severity: blocking
---

**Rule.** When marking or querying extension availability flags on
`DeviceExtensions` helper structs in `VulkanPreferredFeatures`, assign to the
boolean member named with the extension's vendor suffix (such as `f...EXT` or
`f...KHR`). Do not reference the outer class's extension name string pointer
(such as `f...Extension`), which does not exist on `DeviceExtensions`.

**Why.** `DeviceExtensions` stores extension support flags as booleans named
with vendor suffixes (e.g., `fQueueFamilyForeignEXT`), whereas
`VulkanPreferredFeatures` stores extension string names ending in `Extension`
(e.g., `fQueueFamilyForeignExtension`). Referencing the string name on
`DeviceExtensions` causes compilation errors that may go unnoticed during host
testing if enclosed in platform guards like `#if defined(SK_BUILD_FOR_ANDROID)`.

## Examples

Bad:

```cpp
// On DeviceExtensions exts:
#if defined(SK_BUILD_FOR_ANDROID)
    exts.fQueueFamilyForeignExtension = true;
#endif
```

Good:

```cpp
// On DeviceExtensions exts:
#if defined(SK_BUILD_FOR_ANDROID)
    exts.fQueueFamilyForeignEXT = true;
#endif
```
