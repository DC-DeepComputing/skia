---
name: skia/gpu/use-nullptr-for-vulkan-pointers
description: When initializing, assigning, or checking pointer members in Vulkan structs, use nullptr instead of VK_NULL_HANDLE, reserving VK_NULL_HANDLE strictly for opaque object handles
rubric: readability
severity: should_fix
---

**Rule.** Use nullptr instead of VK_NULL_HANDLE for pointer members in Vulkan
structures (such as pNext, pBindings, or array pointers) and function arguments.
Reserve VK_NULL_HANDLE strictly for opaque Vulkan object handles (such as
VkBuffer, VkImage, VkPipeline, or VkDeviceMemory).

**Why.** In the Vulkan API, VK_NULL_HANDLE is specifically typed for Vulkan
dispatchable and non-dispatchable object handles, which can be defined as
integers (such as uint64_t) rather than pointers on 32-bit platforms. Pointer
members (conventionally prefixed with 'p', like pNext and pBindings) are
standard C++ pointers; using nullptr conveys pointer semantics unambiguously,
prevents handle-versus-pointer type confusion, and avoids compiler warnings or
type errors across build targets.

## Examples

Bad:

```cpp
VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
layoutCreateInfo.pBindings = bindingLayouts.empty() ? VK_NULL_HANDLE : bindingLayouts.data();
layoutCreateInfo.pNext = VK_NULL_HANDLE;
```

Good:

```cpp
VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
layoutCreateInfo.pBindings = bindingLayouts.data();
layoutCreateInfo.pNext = nullptr;
```
