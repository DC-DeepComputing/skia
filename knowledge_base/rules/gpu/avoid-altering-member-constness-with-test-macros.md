---
name: skia/gpu/avoid-altering-member-constness-with-test-macros
description: When inspecting const members of production types in GPU unit tests, do not weaken member constness with GPU_TEST_UTILS; use const_cast locally in tests instead
rubric: readability
severity: should_fix
---

**Rule.** Do not modify core production types or alter member constness using
test-only preprocessor macros (such as `#if defined(GPU_TEST_UTILS)`) to
accommodate test inspection. Keep production struct layouts and const-qualifiers
consistent across all build configurations, and perform a local `const_cast`
within the unit test when mutable access to inspect or map a resource is
necessary.

**Why.** Altering member constness with test-only macros creates divergent type
signatures and API contracts between test and release builds, leaking test
concerns into core definitions. Performing const_cast locally within the test
preserves the integrity of production types while isolating test-specific
mutability to test files.

## Examples

Bad:

```cpp
// In src/gpu/graphite/ResourceTypes.h:
struct BindBufferInfo {
#if defined(GPU_TEST_UTILS)
    Buffer* fBuffer = nullptr;
#else
    const Buffer* fBuffer = nullptr;
#endif
    uint32_t fOffset = 0;
    uint32_t fSize = 0;
};
```

Good:

```cpp
// In src/gpu/graphite/ResourceTypes.h:
struct BindBufferInfo {
    const Buffer* fBuffer = nullptr;
    uint32_t fOffset = 0;
    uint32_t fSize = 0;
};

// In tests/graphite/VerticesPaddingTest.cpp:
map_and_readback_buffer_sync(
        context,
        testContext,
        mappableDrawBuffers ? const_cast<Buffer*>(bindBuffers[i].fBuffer) :
                              copyBuffers[i].get(),
        readbackData,
        kReadbackSize);
```
