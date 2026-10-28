---
name: skia/gpu/check-gpu-buffer-map-for-null
description: When mapping GPU or transfer buffers via buffer->map(), check for null at runtime, log a warning, and bail out gracefully instead of relying on SkASSERT
rubric: correctness
severity: should_fix
---

**Rule.** Do not rely on SkASSERT to validate pointers returned by GPU buffer
mapping (such as buffer->map()). Always check the mapped pointer for null at
runtime, emit an informative warning log (SKGPU_LOG_W or SKIA_LOG_W), and bail
out gracefully.

**Why.** GPU buffer mapping can fail at runtime due to driver errors, device
loss, allocation limits, or host out-of-memory conditions, even for valid
buffers. Because SkASSERT is compiled out in release builds, asserting non-null
leaves production code vulnerable to null pointer dereference crashes; checking
at runtime and logging a warning preserves diagnostic visibility while safely
skipping or aborting the operation.

## Examples

Bad:

```cpp
uint64_t* results = static_cast<uint64_t*>(buffer->map());
SkASSERT(results);
if (results[1] < results[0]) {
    return {};
}
```

Good:

```cpp
uint64_t* results = static_cast<uint64_t*>(buffer->map());
if (!results) {
    SKGPU_LOG_W("Failed to get timer query results because buffer couldn't be mapped.");
    return {};
}
if (results[1] < results[0]) {
    return {};
}
```
