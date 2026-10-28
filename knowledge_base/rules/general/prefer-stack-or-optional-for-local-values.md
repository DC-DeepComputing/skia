---
name: skia/prefer-stack-or-optional-for-local-values
description: When managing local temporary or conditionally present values in function scopes, prefer stack variables or std::optional over heap allocation with new and delete
rubric: safety
severity: should_fix
---

**Rule.** Do not dynamically heap-allocate local temporary, scratch, or
conditionally present values with new and delete (or std::make_unique). Use
stack variables, or std::optional<T> when the value is conditionally
initialized, and pass the object to callees by pointer, reference, or value.

**Why.** Dynamically allocating local or optional variables on the heap
introduces avoidable allocator churn, memory fragmentation, and pointer
indirection overhead, while manual new/delete risks memory leaks on early
returns. Using stack variables or std::optional<T> keeps storage in the local
stack frame, guarantees automatic RAII lifetime cleanup, and eliminates heap
traffic.

## Examples

Bad:

```cpp
// Bad: Heap-allocating a local conditionally present value with new and delete.
StdSteadyClock::time_point* stopPurgingTime = nullptr;
if (stopPurgingPastDuration) {
    stopPurgingTime = new skgpu::StdSteadyClock::time_point(
            skgpu::StdSteadyClock::now() + *stopPurgingPastDuration);
}
fResourceProvider->purgeResourcesNotUsedSince(purgeTime, stopPurgingTime);
delete stopPurgingTime;
```

Good:

```cpp
// Good: Use std::optional on the stack to hold conditionally initialized values.
std::optional<StdSteadyClock::time_point> stopPurgingTime;
if (stopPurgingPastDuration.has_value()) {
    stopPurgingTime = { StdSteadyClock::now() + stopPurgingPastDuration.value() };
}
fResourceCache->purgeResourcesNotUsedSince(purgeTime, stopPurgingTime);
```
