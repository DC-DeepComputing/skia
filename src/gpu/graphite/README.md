# Graphite GPU Backend (`src/gpu/graphite/`)

## Overview & Scope

`src/gpu/graphite/` implements **Graphite**, Skia's modern GPU rendering backend (`SK_GRAPHITE`, namespace `skgpu::graphite`). Graphite is designed for modern explicit graphics APIs—**Dawn / WebGPU** (`dawn/`), **Metal** (`mtl/`), and **Vulkan** (`vk/`)—with a focus on:
- **Multithreaded Recording**: Decoupling thread-local `Recorder` instances from the central `Context` so multiple threads can record `Recording` command sequences concurrently without lock contention.
- **Pipeline Precompilation & Reduced Stutter**: Separating geometry rendering techniques (`RenderStep`) from shading trees (`PaintParamsKey` / `ShaderCodeDictionary`), allowing applications to pre-generate pipelines ahead of time via `Precompile` APIs.
- **Unified Draw Ordering & Batching**: Using painter's order depth-buffer testing (`DrawOrder`) and instanced/dynamic vertex buffers (`DrawWriter`, `BufferManager`) to reorder and batch draws across an entire render pass without heavy CPU op-merging overhead.

Public headers reside in `include/gpu/graphite/` (with backend headers in `dawn/`, `mtl/`, `vk/`, and precompilation headers in `precompile/`). Shared GPU utilities live in [`src/gpu/`](../README.md).

## Public API Entrypoints & Key Internal Abstractions

### Public Headers (`include/gpu/graphite/`)
- **`Context.h` / `ContextOptions.h`**: The main GPU engine handle (`skgpu::graphite::Context`), created via backend factories (`ContextFactory::MakeDawn`, `MakeMetal`, `MakeVulkan`). Manages the `QueueManager`, `SharedContext`, `GlobalCache`, and GPU work submission (`Context::insertRecording`, `Context::submit`).
- **`Recorder.h` / `Recording.h`**: `Recorder` (`Context::makeRecorder`) provides a thread-safe context for creating Graphite-backed `SkSurface`s, caching thread-local resources (`ResourceProvider`, `DrawBufferManager`, `AtlasProvider`), and snapping an immutable `Recording` (`Recorder::snap`) ready for insertion on the `Context`.
- **`BackendTexture.h` / `BackendSemaphore.h` / `TextureInfo.h`**: Backend-agnostic wrappers (`BackendTexture`, `BackendSemaphore`, `TextureInfo`) for importing/exporting native Dawn, Metal, or Vulkan textures and synchronization primitives.
- **`Surface.h` / `Image.h` / `ImageProvider.h`**: Public factories (`SkSurfaces::RenderTarget`, `SkSurfaces::WrapBackendTexture`, `SkImages::TextureFromImage`, `SkImages::WrapTexture`) and the client-extensible `ImageProvider` interface for resolving non-texture images during recording.
- **`PrecompileContext.h` & `precompile/Precompile.h`**: Public API (`skgpu::graphite::Precompile`, `PaintOptions`, `PrecompileShader`, `PrecompileBlender`, `PrecompileColorFilter`, `PrecompileImageFilter`, `PrecompileMaskFilter`) enabling offline or startup compilation of `GraphicsPipeline` permutations.

### Internal Recording & Execution Pipeline (`src/gpu/graphite/`)
1. **Canvas to Device (`Device`)**: `Device.h` subclasses `SkDevice`. It tracks clipping (`ClipStack.h`, `ClipAtlasManager.h`), selects a `Renderer` (from `RendererProvider`) and a path atlas (`PathAtlas`, `ComputePathAtlas`, `RasterPathAtlas`) for each draw, extracts `PaintParams`, and records into the surface's `DrawContext`.
2. **Draw Recording (`DrawContext`, `DrawList`, `DrawPass` in `draw/`)**:
   - `DrawContext` accumulates draws into a `DrawList` while assigning monotonically increasing painter's order / depth values (`DrawOrder`, `CompressedPaintersOrder`, `DisjointStencilIndex`) via `BoundsManager`.
   - When a render pass is flushed, `DrawPass::Make` sorts draws by pipeline and depth to maximize batching without violating painter's order, streaming vertex/instance data via `DrawWriter` and uniforms via `PipelineDataGatherer` / `UniformManager`.
3. **Renderers & RenderSteps (`Renderer`, `RenderStep` in `render/`)**: A `Renderer` executes 1 or more `RenderStep`s (e.g., `AnalyticRRectRenderStep`, `PerEdgeAAQuadRenderStep`, `TessellateCurvesRenderStep`, `MiddleOutFanRenderStep`, `CoverBoundsRenderStep`, `BitmapTextRenderStep`, `CoverageMaskRenderStep`). Each `RenderStep` defines fixed vertex/instance attributes, depth/stencil settings, and vertex/coverage SkSL snippets.
4. **Shading & Keys (`PaintParamsKey`, `KeyHelpers`, `ShaderCodeDictionary`, `ShaderInfo`)**:
   - `KeyHelpers.cpp` traverses `SkPaint` (`SkShader`, `SkColorFilter`, `SkBlender`) using `KeyContext` to build a compact `PaintParamsKey`.
   - `ShaderCodeDictionary` deduplicates keys into a `UniquePaintParamsID` and maps built-in or runtime-effect snippet IDs (`BuiltInCodeSnippetID.h`) to SkSL functions in `src/sksl/sksl_graphite_frag.sksl` and `sksl_graphite_vert.sksl`.
   - `GraphicsPipelineDesc` pairs a `RenderStep` ID with a `UniquePaintParamsID`; `ShaderInfo` stitches the final vertex and fragment SkSL AST/source for backend compilation.
5. **Task Graph (`Task`, `TaskList` in `task/`)**: `Recorder` builds a `TaskList` graph comprising `DrawTask`, `RenderPassTask`, `UploadTask`, `CopyTask`, `ComputeTask` (driving `ComputeStep` / `DispatchGroup`), `ClearBuffersTask`, and `SynchronizeToCpuTask`.
6. **Resources & Submission (`SharedContext`, `ResourceProvider`, `CommandBuffer`, `QueueManager`)**: On `Context::insertRecording`, tasks prepare pipelines and resources via `ResourceProvider` / `ResourceCache` and encode native GPU commands into a backend `CommandBuffer` (`DawnCommandBuffer`, `MtlCommandBuffer`, `VulkanCommandBuffer`), which `QueueManager` submits to the GPU queue as a `GpuWorkSubmission`.

## Subdirectories

| Directory | Purpose |
| :--- | :--- |
| `draw/` | Draw recording and sorting pipeline (`DrawContext`, `DrawList`, `DrawPass`, `DrawWriter`, `DrawOrder`, `DrawParams`). |
| `render/` | `RenderStep` implementations for analytic primitives, hardware tessellation, stencil-then-cover, text, vertices, and coverage masks. |
| `geom/` | Geometric primitives and spatial indexing (`Shape`, `Rect`, `Transform`, `EdgeAAQuad`, `BoundsManager`, `IntersectionTree`, `AnalyticBlurMask`). |
| `task/` | Execution task graph nodes (`Task`, `TaskList`, `RenderPassTask`, `DrawTask`, `UploadTask`, `ComputeTask`, `CopyTask`). |
| `compute/` | GPU compute pipeline abstractions (`ComputeStep`, `DispatchGroup`) and optional Vello compute path stages (`VelloRenderer`, `VelloComputeSteps`). |
| `precompile/` | Combinatorial `PaintOptions` / `PrecompileBase` hierarchy for ahead-of-time shader and pipeline compilation. |
| `sparse_strips/` | Experimental CPU/SIMD sparse-strip path rasterization and wide-tile rendering (`StripGenerator`, `Tiler`, `AlphaAtlasManager`). |
| `text/` | Text atlas management (`TextAtlasManager`, `TextStrike`, `GlyphData`) working with `DrawAtlas` and `src/text/gpu/`. |
| `surface/` | Platform-specific `SkSurface` factories (e.g., Android `AHardwareBuffer` surface creation). |
| `dawn/` | Dawn / WebGPU backend (`DawnSharedContext`, `DawnCommandBuffer`, `DawnGraphicsPipeline`, `DawnResourceProvider`, `DawnCaps`). |
| `mtl/` | Metal backend (`MtlSharedContext`, `MtlCommandBuffer`, `MtlGraphicsPipeline`, `MtlResourceProvider`, `MtlCaps`). |
| `vk/` | Vulkan backend (`VulkanSharedContext`, `VulkanCommandBuffer`, `VulkanGraphicsPipeline`, `VulkanResourceProvider`, `VulkanCaps`). |

## Build & Test

### Build Files
- **Bazel**: `src/gpu/graphite/BUILD.bazel` (target `//src/gpu/graphite:graphite`), backend targets in `dawn/BUILD.bazel`, `mtl/BUILD.bazel`, `vk/BUILD.bazel`, `precompile/BUILD.bazel`, and public headers in `include/gpu/graphite/BUILD.bazel`.
- **GN**: `gn/graphite.gni` (`skia_graphite_public`, `skia_graphite_sources`, `skia_graphite_dawn_sources`, `skia_graphite_mtl_sources`, `skia_graphite_vk_sources`, `skia_graphite_precompile_sources`).

### Testing
- **Unit Tests**: Defined in `tests/graphite/` using `DEF_GRAPHITE_TEST(name, reporter, CtsEnforcement)` or `DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(name, reporter, context, testContext, CtsEnforcement)` (see `tests/Test.h`). Precompile tests live in `tests/graphite/precompile/`.
- **DM (`dm`)**: Run Graphite GMs and unit tests with `--config`:
  - `grmtl`, `grvk`, `grdawn_vk`, `grdawn_mtl`, `grdawn_d3d11`, `grdawn_d3d12`, `grdawn_gl`, `grdawn_gles`
  - Example: `out/Release/dm --config grmtl --match Graphite`
