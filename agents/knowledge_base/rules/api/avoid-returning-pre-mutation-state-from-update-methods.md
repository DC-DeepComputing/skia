---
name: skia/api/avoid-returning-pre-mutation-state-from-update-methods
description: When designing state mutators or update methods, avoid returning stale pre-mutation values; return void or new state and let callers query prior state directly
rubric: readability
severity: should_fix
---

**Rule.** Do not design mutator or update methods (such as update<Property>())
to return stale, pre-mutation values. Keep update methods as void (or return the
newly applied value) and let callers explicitly query existing state prior to
the update if delta tracking or prior state inspection is needed.

**Why.** Returning pre-mutation values from update or mutator methods violates
caller expectations and the principle of least astonishment, as callers expect
mutators to either return nothing or reflect the current updated state.
Returning old state hides state inspection from the call site and leads to
confusion or bugs when callers assume the return value reflects the new state.

## Examples

Bad:

```cpp
// Regardless if the size was updated or not, this function returns the previous size before this function was called.
size_t updateGpuMemorySize();
```

Good:

```cpp
void updateGpuMemorySize() { fGpuMemorySize = this->onUpdateGpuMemorySize(); }
virtual size_t onUpdateGpuMemorySize() { return fGpuMemorySize; }

// Caller queries old state beforehand if delta tracking is needed:
size_t oldSize = resource->gpuMemorySize();
resource->updateGpuMemorySize();
```
