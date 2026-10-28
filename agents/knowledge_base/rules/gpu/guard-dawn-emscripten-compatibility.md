---
name: skia/gpu/guard-dawn-emscripten-compatibility
description: 'When updating Graphite Dawn or WebGPU APIs, enums, or structs, guard newer Dawn features with #if defined(__EMSCRIPTEN__) to avoid breaking WASM builds'
rubric: correctness
severity: blocking
---

**Rule.** When updating Dawn or WebGPU types, enums, descriptors, or API calls
to track upstream Dawn, guard any symbols or values not yet supported by
Emscripten's built-in WebGPU headers with #if defined(__EMSCRIPTEN__) (or #if
!defined(__EMSCRIPTEN__)). Provide appropriate fallback behavior or legacy enum
values for WASM builds.

**Why.** Skia's WASM build compiles against the WebGPU headers provided by
Emscripten rather than upstream native Dawn. Because Emscripten updates its
bundled WebGPU headers on a lag, newly introduced Dawn symbols or changed enum
values break WASM compilation if referenced unconditionally.

## Examples

Bad:

```cpp
layout.arrayStride = 0;
layout.stepMode = wgpu::VertexStepMode::Undefined;
layout.attributeCount = 0;
```

Good:

```cpp
layout.arrayStride = 0;
#if defined(__EMSCRIPTEN__)
layout.stepMode = wgpu::VertexStepMode::VertexBufferNotUsed;
#else
layout.stepMode = wgpu::VertexStepMode::Undefined;
#endif
layout.attributeCount = 0;
```
