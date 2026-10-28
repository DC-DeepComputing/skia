---
name: skia/assert-and-return-false-in-unsupported-platform-stubs
description: When stubbing out methods in preprocessor conditionals for unsupported platforms, return failure like false or E_NOTIMPL instead of silently returning success
rubric: correctness
severity: should_fix
---

**Rule.** When providing fallback or stub implementations in preprocessor
conditional blocks (#if/#else) for unsupported platforms or disabled
configurations, do not silently return success (such as true or S_OK). Return an
explicit failure status (such as false, nullptr, or E_NOTIMPL) and assert
SkASSERT(false) in debug builds where appropriate so callers detect the
unsupported capability instead of proceeding with uninitialized state.

**Why.** Returning success (true or S_OK) from a platform stub when the
operation is unsupported masks missing functionality and allows execution to
proceed with uninitialized state or null output pointers. Returning an explicit
failure code propagates the error to callers in release builds, while asserting
with SkASSERT(false) ensures unsupported invocations fail fast in debug builds.

## Examples

Bad:

```cpp
// Bad: returning true or S_OK masks unsupported operations on disabled platforms
bool DawnCommandBuffer::updateIntrinsicUniformsAsPushConstant(UniformDataBlock uniformData) {
#if !defined(__EMSCRIPTEN__)
    fActiveRenderPassEncoder.SetImmediateData(0, uniformData.data(), uniformData.size());
#endif
    return true;
}

#if defined(SK_WINUWP)
*image = nullptr;
return S_OK;
#endif
```

Good:

```cpp
// Good: asserts and returns false or an explicit error code like E_NOTIMPL
bool DawnCommandBuffer::updateIntrinsicUniformsAsPushConstant(UniformDataBlock uniformData) {
#if !defined(__EMSCRIPTEN__)
    fActiveRenderPassEncoder.SetImmediateData(0, uniformData.data(), uniformData.size());
    return true;
#else
    SkASSERT(false); // Push constants unsupported in WASM
    return false;
#endif
}

#if defined(SK_WINUWP)
return E_NOTIMPL;
#endif
```
