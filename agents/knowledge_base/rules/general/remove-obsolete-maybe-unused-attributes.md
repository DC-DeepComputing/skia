---
name: skia/remove-obsolete-maybe-unused-attributes
description: When refactoring code or adding preprocessor guards that ensure a declaration is always referenced, remove obsolete [[maybe_unused]] attributes from classes, functions, and variables
rubric: readability
severity: nit
---

**Rule.** Do not retain [[maybe_unused]] attributes on class, struct, function,
or variable declarations when refactoring or conditional compilation (#if)
guards guarantee the entity is always referenced in every build configuration
where it is compiled. Remove stale [[maybe_unused]] attributes once active call
sites or preprocessor branches are established so compiler diagnostics can
detect future unreferenced code.

**Why.** Retaining [[maybe_unused]] on symbols that are unconditionally
referenced within their active compilation configuration adds visual clutter and
misleads readers about how the code is used. Furthermore, suppressing unused
warnings on symbols that should be active risks masking future dead code or
broken wiring if call sites are later eliminated.

## Examples

Bad:

```cpp
#if defined(__EMSCRIPTEN__)
// DawnWorkSubmission with AsyncWait for wasm builds where wgpu::Future is unavailable.
class [[maybe_unused]] DawnWorkSubmissionWithAsyncWait final : public GpuWorkSubmission {
public:
    DawnWorkSubmissionWithAsyncWait(std::unique_ptr<CommandBuffer> cmdBuffer,
                                    const DawnQueueManager* queueManager);
    ...
};
#endif
```

Good:

```cpp
#if defined(__EMSCRIPTEN__)
// DawnWorkSubmission with AsyncWait for wasm builds where wgpu::Future is unavailable.
class DawnWorkSubmissionWithAsyncWait final : public GpuWorkSubmission {
public:
    DawnWorkSubmissionWithAsyncWait(std::unique_ptr<CommandBuffer> cmdBuffer,
                                    const DawnQueueManager* queueManager);
    ...
};
#endif
```
