---
name: skia/gpu/avoid-gpu-test-utils-in-production
description: When defining test-only helpers, constructors, or call sites in include/ or src/gpu, guard them with GPU_TEST_UTILS instead of [[maybe_unused]] or unconditional exposure
rubric: design
severity: blocking
---

**Rule.** When defining test-only constructors, helper methods, hooks, or
verification accessors in public headers (under include/) or src/gpu, do not
expose them unconditionally to clients or silence unused warnings with
[[maybe_unused]]; guard their declarations with #if defined(GPU_TEST_UTILS) (or
GR_TEST_UTILS) or confine them to internal headers in src/. Additionally, do not
call test-only methods, accessors, or operators from production code without
guarding the call site under the matching test preprocessor condition (or making
the implementation available unconditionally if required in production).

**Why.** Public headers under include/ define Skia's public API contract;
exposing test-only constructors or internal helpers unconditionally leaks
unvetted behaviors and unstable interfaces to external embedders. Test utility
hooks and methods are excluded from production library builds, so unguarded
production call sites break non-test builds, while silencing unused-symbol
warnings with [[maybe_unused]] retains dead test code in release binaries.

## Examples

Bad:

```cpp
// include/utils/SkNWayCanvas.h - test-only constructor exposed unconditionally
class SK_API SkNWayCanvas : public SkCanvasVirtualEnforcer<SkNoDrawCanvas> {
public:
    SkNWayCanvas(int width, int height);
    SkNWayCanvas(SkCanvas*); // Bad: test-only constructor exposed to public clients
};

// Bad: test hook silenced with attribute instead of preprocessor guard
[[maybe_unused]]
SK_ALWAYS_INLINE void observeChunk(int32_t row, int32_t column, int32_t chunkSize) {
    fObserver(exactMask); // fObserver only exists under GPU_TEST_UTILS
}

// Bad: calling GPU_TEST_UTILS-only operator== without preprocessor guard
bool equal(const GrBackendTextureData* that) const override {
    if (auto otherD3D = static_cast<const GrD3DBackendTextureData*>(that)) {
        return fInfo == otherD3D->fInfo;
    }
    return false;
}
```

Good:

```cpp
// include/utils/SkNWayCanvas.h - test-only constructor guarded by test utility flag
class SK_API SkNWayCanvas : public SkCanvasVirtualEnforcer<SkNoDrawCanvas> {
public:
    SkNWayCanvas(int width, int height);
#if SK_SUPPORT_GPU && defined(GPU_TEST_UTILS)
    SkNWayCanvas(SkCanvas*); // Good: test-only constructor omitted from client builds
#endif
};

// Good: test hook conditionally compiled under GPU_TEST_UTILS
#if defined(GPU_TEST_UTILS)
SK_ALWAYS_INLINE void observeChunk(int32_t row, int32_t column, int32_t chunkSize) {
    fObserver(exactMask);
}
#endif

// Good: caller guarded by matching preprocessor condition
#if defined(GPU_TEST_UTILS)
bool equal(const GrBackendTextureData* that) const override {
    if (auto otherD3D = static_cast<const GrD3DBackendTextureData*>(that)) {
        return fInfo == otherD3D->fInfo;
    }
    return false;
}
#endif
```
