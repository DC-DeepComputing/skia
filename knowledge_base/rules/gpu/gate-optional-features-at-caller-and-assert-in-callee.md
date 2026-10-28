---
name: skia/gpu/gate-optional-features-at-caller-and-assert-in-callee
description: When delegating optional feature operations to internal GPU backend methods, gate checks on feature pointers at the caller and assert non-null with SkASSERT in callees
rubric: readability
severity: nit
---

**Rule.** When delegating optional feature operations (such as persistent
pipeline caching or storage) from frontend contexts to internal backend methods,
gate the call at the caller by checking the feature pointer. In the callee
backend implementation, enforce the non-null invariant with SkASSERT rather than
adding defensive early-return null checks.

**Why.** Gating optional feature checks at the outer caller avoids unnecessary
virtual dispatch or delegation when the feature is disabled, clarifies the
interface contract, and allows backend callees to enforce non-null preconditions
with debug assertions rather than carrying redundant runtime bailout branches.

## Examples

Bad:

```cpp
// In Context.cpp:
void Context::syncPipelineData(size_t maxSize) {
    fSharedContext->syncPipelineData(fPersistentPipelineStorage, maxSize);
}

// In VulkanSharedContext.cpp:
void VulkanSharedContext::syncPipelineData(PersistentPipelineStorage* persistentPipelineStorage,
                                           size_t maxSize) {
    if (!persistentPipelineStorage) {
        return;
    }
    if (fPipelineCache == VK_NULL_HANDLE) {
        return;
    }
    ...
}
```

Good:

```cpp
// In Context.cpp:
void Context::syncPipelineData(size_t maxSize) {
    if (fPersistentPipelineStorage) {
        fSharedContext->syncPipelineData(fPersistentPipelineStorage, maxSize);
    }
}

// In VulkanSharedContext.cpp:
void VulkanSharedContext::syncPipelineData(PersistentPipelineStorage* persistentPipelineStorage,
                                           size_t maxSize) {
    SkASSERT(persistentPipelineStorage);
    if (fPipelineCache == VK_NULL_HANDLE) {
        return;
    }
    ...
}
```
