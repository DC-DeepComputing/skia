---
name: skia/populate-tarray-in-place-using-push-back-n
description: When querying C-style enumeration or query APIs into skia_private::TArray or STArray, expand the container in-place using push_back_n instead of allocating temporary heap buffers
rubric: safety
severity: should_fix
---

**Rule.** When populating skia_private::TArray or STArray from C-style
enumeration or query APIs (such as Vulkan or platform queries that take a
pointer and count to write into), do not allocate temporary raw heap buffers
(new[]/delete[]) or intermediate vectors before copying. Instead, grow the
container in-place using push_back_n(count) and pass the pointer to the newly
allocated elements directly to the C API.

**Why.** Allocating temporary heap buffers with new[] and delete[] introduces
unnecessary allocation churn, manual cleanup hazards on error paths, and
redundant copy loops. Calling push_back_n(count) allocates directly into
container storage and updates size() consistently for range iteration.

## Examples

Bad:

```cpp
// BAD: Allocates temporary heap array, requiring manual cleanup and redundant copying
auto tempExts = new VkExtensionProperties[extCount];
if (vkEnumerateInstanceExtensionProperties(layerName, &extCount, tempExts) != VK_SUCCESS) {
    delete[] tempExts;
    return false;
}
extensions.push_back_n(extCount, tempExts);
delete[] tempExts;
```

Good:

```cpp
// GOOD: Expands container in-place and passes pointer directly to C query API
VkExtensionProperties* newExts = extensions.push_back_n(extCount);
if (vkEnumerateInstanceExtensionProperties(layerName, &extCount, newExts) != VK_SUCCESS) {
    return false;
}
```
