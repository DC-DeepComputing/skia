---
name: skia/gpu/log-gpu-allocation-and-binding-failures
description: When backend driver resource creation, memory allocation, or binding fails in GPU code, log an immediate warning or error rather than failing silently
rubric: readability
severity: should_fix
---

**Rule.** When GPU driver calls or backend helpers fail during resource
creation, device memory allocation, or resource binding (such as
CreatePipelineCache, AllocateMemory, or BindImageMemory), emit an immediate
warning or error log (SKIA_LOG_W, SKGPU_LOG_W, or SKIA_LOG_E) with relevant
return codes or context at the failure site. Do not return null or false
silently, and do not rely solely on downstream callers to diagnose the failure.

**Why.** Driver resource creation and device memory allocation can fail due to
driver bugs, memory exhaustion, or incompatible parameters. Logging immediately
at the failure point provides exact diagnostic visibility into the root cause
and prevents silent bailouts or duplicate downstream warnings.

## Examples

Bad:

```cpp
// Case 1: Backend driver resource creation
VkPipelineCache pipelineCache = VK_NULL_HANDLE;
VULKAN_CALL_RESULT(this, result,
                   CreatePipelineCache(this->device(), &createInfo, nullptr, &pipelineCache));
if (VK_SUCCESS != result) {
    return VK_NULL_HANDLE;
}

// Case 2: Backend memory allocation or binding
result = SHARED_GR_VULKAN_CALL(interface, AllocateMemory(device, &allocInfo, nullptr, &memory));
if (result != VK_SUCCESS) {
    return false;
}
```

Good:

```cpp
// Case 1: Backend driver resource creation
VkPipelineCache pipelineCache = VK_NULL_HANDLE;
VULKAN_CALL_RESULT(this, result,
                   CreatePipelineCache(this->device(), &createInfo, nullptr, &pipelineCache));
if (VK_SUCCESS != result) {
    SKGPU_LOG_W("CreatePipelineCache failed");
    return VK_NULL_HANDLE;
}

// Case 2: Backend memory allocation or binding
result = SHARED_GR_VULKAN_CALL(interface, AllocateMemory(device, &allocInfo, nullptr, &memory));
if (result != VK_SUCCESS) {
    SKIA_LOG_E("Failed to allocate device memory for AHB (result: %d)", result);
    return false;
}
```
