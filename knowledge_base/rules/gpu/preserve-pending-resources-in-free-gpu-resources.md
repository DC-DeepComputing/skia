---
name: skia/gpu/preserve-pending-resources-in-free-gpu-resources
description: In GPU resource managers, preserve pending command-buffer resources during cache purges, and actively release them upon destruction instead of asserting empty
rubric: correctness
severity: should_fix
---

**Rule.** In GPU resource managers and cache providers, do not release pending
or in-flight command-buffer resources during intermediate cache purges or
freeGpuResources(). Conversely, in manager destructors, actively release any
remaining pending resources (e.g., via releasePending*() helpers) rather than
asserting that pending queues are empty (e.g.,
SkASSERT(fPendingBuffers.isEmpty())), safely handling early context destruction
before command buffers finish.

**Why.** Cache purges can occur while a command buffer is actively recording,
where prematurely releasing pending resources risks use-after-free or data
corruption when GPU commands execute. Conversely, a GPU Context can be destroyed
while command buffers remain unsubmitted or in flight without finishEncoding()
running; asserting empty queues in the destructor causes assertion failures on
valid teardown, while omitting cleanup leaks resources.

## Examples

Bad:

```cpp
// Bad: releasing pending in-flight buffers during cache purges
void freeGpuResources() {
    this->purgeResourcesNotUsedSince(skgpu::StdSteadyClock::now());
    this->releasePendingIntrinsicBuffers();
}

// Bad: asserting pending queues are empty in destructor on context teardown
~IntrinsicConstantsManager() {
    this->purgeBuffersIf([](IntrinsicBuffer*) { return true; });
    SkASSERT(fPendingIntrinsicBuffers.isEmpty());
}
```

Good:

```cpp
// Good: defer releasing pending resources during cache purges
void freeGpuResources() {
    this->purgeResourcesNotUsedSince(skgpu::StdSteadyClock::now());
}

// Good: actively release remaining pending resources in destructor
~IntrinsicConstantsManager() {
    this->purgeBuffersIf([](IntrinsicBuffer*) { return true; });
    this->releasePendingIntrinsicBuffers();
    SkASSERT(fIntrinsicBuffersLRU.isEmpty());
}
```
