---
name: skia/gpu/use-vulkan-call-nocheck-for-non-success-status
description: When invoking Vulkan APIs with acceptable non-success status codes, use NOCHECK call macros, ensure checkVkResult tracks device loss, and validate status directly
rubric: correctness
severity: blocking
---

**Rule.** When invoking Vulkan driver entry points that can legitimately return
non-error or non-fatal status codes (such as VK_INCOMPLETE, VK_NOT_READY,
VK_TIMEOUT, or VK_PIPELINE_COMPILE_REQUIRED), use GR_VK_CALL_RESULT_NOCHECK (in
Ganesh) or VULKAN_CALL_RESULT_NOCHECK (in Graphite) instead of standard checked
macros. Ensure checkVkResult(result) is invoked on the GPU or
VulkanSharedContext to update device-lost and OOM tracking, but do not rely on
its boolean return value; directly validate acceptable status codes and log
unexpected errors before bailing out.

**Why.** Standard CALL_RESULT macros assert that the return code is VK_SUCCESS
or VK_ERROR_DEVICE_LOST, causing false assertions when APIs return valid
non-success codes. Meanwhile, checkVkResult updates device-lost state and
triggers out-of-memory callbacks but returns false for any code other than
VK_SUCCESS; calling checkVkResult while ignoring its boolean return preserves
device health tracking without falsely treating valid non-error statuses as
failures.

## Examples

Bad:

```cpp
// Bad: standard macro asserts on non-success codes like VK_INCOMPLETE
GR_VK_CALL_RESULT(fGpu, result,
                  GetPipelineCacheData(fGpu->device(), this->pipelineCache(),
                                       &dataSize, (void*)data.get()));
if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
    return;
}

// Bad: Graphite NOCHECK omits checkVkResult, missing device-lost/OOM tracking
VULKAN_CALL_RESULT_NOCHECK(fSharedContext->interface(), err,
                           GetFenceStatus(fSharedContext->device(), fence));
if (err != VK_SUCCESS && err != VK_NOT_READY) {
    SKGPU_LOG_E("vkGetFenceStatus failed (%d)", err);
}
```

Good:

```cpp
// Good: use NOCHECK, ensure checkVkResult is called, and validate status directly
GR_VK_CALL_RESULT_NOCHECK(fGpu, result,
                          GetPipelineCacheData(fGpu->device(), this->pipelineCache(),
                                               &dataSize, (void*)data.get()));
fGpu->checkVkResult(result);
if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
    SkDebugf("%s: vkGetPipelineCacheData failed (%d)\n", __func__, result);
    return;
}

// Good: in Graphite, explicitly invoke checkVkResult alongside NOCHECK
VULKAN_CALL_RESULT_NOCHECK(fSharedContext->interface(), err,
                           GetFenceStatus(fSharedContext->device(), fence));
fSharedContext->checkVkResult(err);
if (err != VK_SUCCESS && err != VK_NOT_READY) {
    SKGPU_LOG_E("vkGetFenceStatus failed (%d)", err);
}
```
