# Ganesh GPU Backend (`src/gpu/ganesh/`)

## Overview & Scope

`src/gpu/ganesh/` implements **Ganesh**, Skia's longstanding GPU acceleration backend (`SK_GANESH`). Ganesh translates `SkCanvas` 2D drawing calls into retained Directed Acyclic Graphs (DAGs) of render tasks (`GrRenderTask`) and batched draw operations (`GrOp`), which are then compiled into native GPU API commands during flush.

Ganesh supports **OpenGL / OpenGL ES / WebGL** (`gl/`), **Vulkan** (`vk/`), **Metal** (`mtl/`), **Direct3D 12** (`d3d/`), and a CPU-only **Mock** backend (`mock/`) for testing pipeline and op batching logic without a physical GPU.

Public headers reside in `include/gpu/ganesh/` (and backend subdirectories `gl/`, `vk/`, `mtl/`, `d3d/`, `mock/`), Chromium-specific deferred display list headers live in `include/private/chromium/`, and shared `skgpu` utilities live in [`src/gpu/`](../README.md).

## Public API Entrypoints & Key Internal Abstractions

### Public Headers (`include/gpu/ganesh/`)
- **`GrDirectContext.h`**: Represents the active connection to the underlying 3D API context/device (`GrDirectContext`). Owns the `GrGpu`, `GrResourceCache`, and `GrResourceProvider`, and drives flushing and GPU submission. Backend-specific factory headers live in `gl/GrGLDirectContext.h`, `vk/GrVkDirectContext.h`, `mtl/GrMtlDirectContext.h`, and `d3d/GrD3DDirectContext.h`.
- **`GrRecordingContext.h`**: Base context (`GrRecordingContext`) capable of recording draw operations into an `SkSurface` or `GrDeferredDisplayList` without requiring immediate access to the physical `GrGpu`.
- **`GrBackendSurface.h` / `GrBackendSemaphore.h`**: Wrappers (`GrBackendTexture`, `GrBackendRenderTarget`, `GrBackendFormat`, `GrBackendSemaphore`) encapsulating native GPU textures, render targets, and sync primitives imported from or exported to client applications.
- **`SkSurfaceGanesh.h` / `SkImageGanesh.h` / `SkMeshGanesh.h`**: Factory functions (`SkSurfaces::RenderTarget`, `SkSurfaces::WrapBackendTexture`, `SkImages::BorrowTextureFrom`, `SkImages::CrossContextTexture`) tying core `SkSurface` and `SkImage` types to Ganesh.

### Internal Recording & Execution Pipeline (`src/gpu/ganesh/`)
1. **Canvas to Device (`skgpu::ganesh::Device`)**: `Device.h` subclasses `SkDevice`, intercepting `SkCanvas` calls, managing clip state (`ClipStack.h`), converting `SkPaint` to `GrPaint` (`SkGr.h`), and routing draws to a `SurfaceDrawContext`.
2. **Surface Contexts (`SurfaceContext`, `SurfaceFillContext`, `SurfaceDrawContext`)**: `SurfaceDrawContext` represents a renderable surface target. It selects path renderers (`PathRendererChain`, `PathRenderer`) and appends `GrOp` instances into the current `OpsTask`.
3. **Operations (`GrOp`, `GrDrawOp`, `GrMeshDrawOp` in `ops/`)**: Fine-grained, self-contained drawing commands (e.g., `FillRectOp`, `AtlasTextOp`, `TextureOp`, `PathTessellateOp`). Adjacent or non-overlapping ops of the same type can merge (`CombineIfPossible`) to minimize draw calls.
4. **Processors & Shaders (`GrGeometryProcessor`, `GrFragmentProcessor`, `GrXferProcessor`, `GrPipeline`)**:
   - `GrGeometryProcessor` defines vertex attributes, positioning, and analytic coverage.
   - `GrFragmentProcessor` (`GrFragmentProcessors.cpp`, `effects/`) forms a composable tree representing `SkShader`, `SkColorFilter`, and `SkMaskFilter` stages, emitting modular SkSL via `glsl/GrGLSLFragmentShaderBuilder.h`.
   - `GrXferProcessor` handles blending and destination reads.
   - `GrProgramInfo` and `GrProgramDesc` combine processor keys and `GrPipeline` state to look up or compile backend shaders.
5. **Task Graph & Resource Allocation (`GrDrawingManager`, `GrRenderTask`, `GrSurfaceProxy`)**: During recording, operations reference lazy/virtual `GrSurfaceProxy` (`GrTextureProxy`, `GrRenderTargetProxy`) handles managed by `GrProxyProvider`. On flush, `GrDrawingManager` topologically sorts the `GrRenderTask` DAG (`OpsTask`, `GrCopyRenderTask`, `GrTextureResolveRenderTask`), uses `GrResourceAllocator` to assign physical `GrSurface` / `GrTexture` instances from `GrResourceProvider` and `GrResourceCache`, and executes each task via `GrOpFlushState`.
6. **Backend Execution (`GrGpu`, `GrOpsRenderPass`)**: Each backend implements `GrGpu` (`GrGLGpu`, `GrVkGpu`, `GrMtlGpu`, `GrD3DGpu`, `GrMockGpu`) and `GrOpsRenderPass` to allocate native resources, compile pipelines, upload vertex/uniform buffers (`GrBufferAllocPool`, `GrStagingBufferManager`), and submit native command buffers.

## Subdirectories

| Directory | Purpose |
| :--- | :--- |
| `ops/` | `GrOp` / `GrMeshDrawOp` implementations, `OpsTask`, and `PathRenderer` subclasses (`AtlasPathRenderer`, `TessellationPathRenderer`, `TriangulatingPathRenderer`, `SmallPathRenderer`, `SoftwarePathRenderer`). |
| `effects/` | Built-in `GrFragmentProcessor`, `GrGeometryProcessor`, and `GrXferProcessor` implementations (runtime SkSL effects `GrSkSLFP`, textures, Porter-Duff/custom blends, bezier/oval/rrect analytic effects). |
| `glsl/` | Shader program builders (`GrGLSLProgramBuilder`, `GrGLSLFragmentShaderBuilder`, `GrGLSLVertexGeoBuilder`, `GrGLSLUniformHandler`) that assemble SkSL from `GrProcessor` trees. |
| `geometry/` | Geometry representations and CPU triangulators (`GrShape`, `GrStyledShape`, `GrQuad`, `GrAATriangulator`, `GrTriangulator`). |
| `tessellate/` | Ganesh hardware path and stroke tessellation shaders (`GrPathTessellationShader`, `GrStrokeTessellationShader`) and patch allocators built on `src/gpu/tessellate/`. |
| `gradients/` | Gradient fragment processor builders (`GrGradientShader`) and color ramp bitmap caching (`GrGradientBitmapCache`). |
| `image/` | Ganesh-backed `SkImage` subclasses (`SkImage_Ganesh`, `SkImage_GaneshYUVA`, `SkImage_LazyTexture`, `SkImage_RasterPinnable`). |
| `surface/` | Ganesh-backed `SkSurface` implementation (`SkSurface_Ganesh`) and platform surface factories. |
| `text/` | Texture atlas management (`GrAtlasManager`) for glyph strikes, working alongside `src/text/gpu/` and `GrDrawOpAtlas`. |
| `gl/` | OpenGL, OpenGL ES, and WebGL backend (`GrGLGpu`, `GrGLCaps`, `GrGLInterface`, `builders/`). |
| `vk/` | Vulkan backend (`GrVkGpu`, `GrVkCaps`, `GrVkPipelineStateBuilder`, `GrVkCommandBuffer`, `GrVkResourceProvider`). |
| `mtl/` | Metal backend (`GrMtlGpu`, `GrMtlCaps`, `GrMtlCommandBuffer`, `GrMtlResourceProvider`). |
| `d3d/` | Direct3D 12 backend (`GrD3DGpu`, `GrD3DCaps`, `GrD3DCommandList`, `GrD3DResourceProvider`). |
| `mock/` | No-op GPU backend (`GrMockGpu`, `GrMockCaps`) for unit testing task graphs, resource allocation, and op combining. |

## Build & Test

### Build Files
- **Bazel**: `src/gpu/ganesh/BUILD.bazel` (target `//src/gpu/ganesh:ganesh`), backend targets in `gl/BUILD.bazel`, `vk/BUILD.bazel`, `mtl/BUILD.bazel`, `d3d/BUILD.bazel`, and public headers in `include/gpu/ganesh/BUILD.bazel`.
- **GN**: `gn/gpu.gni` (`skia_gpu_public`, `skia_ganesh_private`, `skia_gpu_gl_private`, `skia_gpu_vk_private`, `skia_gpu_metal_private`, `skia_ganesh_d3d_private`).

### Testing
- **Unit Tests**: Defined in `tests/` using `DEF_GANESH_TEST(name, reporter, options, CtsEnforcement)` or `DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(name, reporter, ctxInfo, CtsEnforcement)` (see `tests/Test.h`). Op and processor unit tests use helpers in `GrDrawOpTest.h` and `GrProcessorUnitTest.h`.
- **DM (`dm`)**: Run Ganesh GMs and unit tests with `--config`:
  - `gl`, `gles`, `glmsaa4`, `vk`, `vkmsaa4`, `mtl`, `mtlmsaa4`, `d3d`, `mock`
  - Example: `out/Release/dm --config vk --match ~^Gr`
