---
name: skia/gpu/avoid-exact-sizing-hooks-on-staging-buffers
description: When testing GPU buffer allocations or padding in Graphite, do not add test-only options or hooks to force exact sizing on staging transfer buffers
rubric: design
severity: should_fix
---

**Rule.** Do not add test-only flags, options, or allocation hooks (such as
bypassing suballocation or forcing exact buffer sizes) to staging or transfer
buffer managers when verifying destination GPU buffer allocations. Rely on
standard suballocation for intermediate transfer buffers, keeping test-specific
sizing constraints isolated to destination resources.

**Why.** Staging and upload buffers are transient intermediate resources used
only to copy host data into destination GPU buffers. Suballocation or alignment
padding on transfer buffers does not impact destination buffer bounds, so adding
dedicated test options to transfer allocators introduces unnecessary complexity.

## Examples

Bad:

```cpp
#if defined(GPU_TEST_UTILS)
    fUseExactBuffSizes ? fUploadManager->makeBindInfoNoReuse(
            info->fBuffer->size(),
            fCaps->requiredTransferBufferAlignment(),
            "TransferForDataBuffer") :
#endif
    fUploadManager->makeBindInfo(info->fBuffer->size(),
                                 fCaps->requiredTransferBufferAlignment(),
                                 "TransferForDataBuffer");
```

Good:

```cpp
    fUploadManager->makeBindInfo(info->fBuffer->size(),
                                 fCaps->requiredTransferBufferAlignment(),
                                 "TransferForDataBuffer");
```
