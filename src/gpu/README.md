# Skia Shared GPU Utilities (`src/gpu/`)

## Overview & Scope

`src/gpu/` contains foundational GPU abstractions, algorithms, and backend utilities shared between Skia's two GPU rendering backends:
- **[Ganesh](ganesh/README.md)** (`src/gpu/ganesh/`): Skia's established immediate/deferred-op GPU backend supporting OpenGL/ES, Vulkan, Metal, Direct3D 12, and Mock contexts.
- **[Graphite](graphite/README.md)** (`src/gpu/graphite/`): Skia's modern, multithreaded recorder/recording GPU architecture designed around low-overhead command submission and pipeline precompilation for Dawn (WebGPU), Metal, and Vulkan.

Code in `src/gpu/` (inside the `skgpu` namespace) has no dependency on either Ganesh (`Gr*`) or Graphite (`skgpu::graphite::*`) internals. Instead, both backends depend on the `//src/gpu:gpu` target for common math, tessellation, resource keys, blending equations, and low-level Vulkan/Metal/Android driver interfaces.

Public headers shared across GPU backends live in `include/gpu/` (`GpuTypes.h`, `MutableTextureState.h`, `ShaderErrorHandler.h`), with shared backend-specific public headers in `include/gpu/vk/` and `include/gpu/mtl/`.

## Public API Entrypoints & Key Internal Abstractions

### Public Headers (`include/gpu/`)
- **`GpuTypes.h`**: Core enums and types shared across Ganesh and Graphite (`skgpu::BackendApi`, `Mipmapped`, `Protected`, `Renderable`, `Budgeted`, `Origin`, `GpuStatsFlags`).
- **`MutableTextureState.h`**: Tracks mutable backend texture state (such as Vulkan `VkImageLayout` and queue family index) across external client and Skia handoffs.
- **`ShaderErrorHandler.h`**: Interface (`skgpu::ShaderErrorHandler`) for reporting shader compilation errors to the client application.
- **`include/gpu/vk/` & `include/gpu/mtl/`**: Shared backend context definitions, extension/feature negotiation (`VulkanBackendContext.h`, `VulkanExtensions.h`, `VulkanPreferredFeatures.h`), and memory allocator interfaces (`VulkanMemoryAllocator.h`, `MtlMemoryAllocator.h`).

### Key Shared Internal Abstractions (`src/gpu/`)
- **`ResourceKey.h` / `KeyBuilder.h`**: Generic, variable-length hashable keys (`skgpu::UniqueKey`, `skgpu::ScratchKey`) and builders (`skgpu::KeyBuilder`) used by resource caches in both backends.
- **`Blend.h` / `BlendFormula.h`**: Hardware blend equations, Porter-Duff/advanced blend coefficient selection, and shader blend formula analysis (`skgpu::BlendFormula`) used to optimize pipeline blending and coverage output.
- **`BlurUtils.h` / `DitherUtils.h` / `GradientBitmap.h`**: Shared 1D/2D Gaussian blur kernel/weight computation, dithering lookup tables, and gradient ramp generation.
- **`Swizzle.h`**: Texture channel swizzling (`skgpu::Swizzle`, e.g., `rgba`, `bgra`, `rrr1`) used to normalize format read/write mappings.
- **`BufferWriter.h`**: Type-safe, alignment-aware helpers (`skgpu::VertexWriter`, `UniformWriter`, `IndexWriter`) for streaming geometry and uniform data into mapped GPU buffers without bounds errors.
- **`Rectanizer.h` (`RectanizerSkyline.h`, `RectanizerPow2.h`)**: 2D rectangle bin-packing algorithms backing dynamic glyph and path atlases (`GrDrawOpAtlas` in Ganesh and `DrawAtlas` in Graphite).
- **`TiledTextureUtils.h` / `DataUtils.h`**: Large-image tiled drawing decomposition and compressed texture format utilities (ETC1, BC1).
- **`SkSLToBackend.h`**: Common adapter bridging `SkSL::Compiler` to backend shader generators (SPIR-V, MSL, WGSL, GLSL, HLSL).

## Subdirectories

| Directory | Purpose |
| :--- | :--- |
| [`ganesh/`](ganesh/README.md) | Skia's `GrDirectContext` / `GrRecordingContext` GPU backend (GL, Vulkan, Metal, D3D12, Mock). |
| [`graphite/`](graphite/README.md) | Skia's `skgpu::graphite::Context` / `Recorder` GPU backend (Dawn, Metal, Vulkan). |
| `tessellate/` | Backend-agnostic path and stroke tessellation math (`WangsFormula.h`, `PatchWriter.h`, `StrokeIterator.h`, `MiddleOutPolygonTriangulator.h`, `LinearTolerances.h`, `FixedCountBufferUtils.h`). |
| `vk/` | Shared Vulkan function table (`VulkanInterface`), memory management (`VulkanMemory`, `vulkanmemoryallocator/`), mutable image state, and feature negotiation (`VulkanPreferredFeatures`). |
| `mtl/` | Shared Metal Objective-C++ utilities (`MtlUtilsPriv.h`) and memory allocator implementation (`MtlMemoryAllocatorImpl`). |
| `android/` | Shared Android `AHardwareBuffer` format/usage conversion utilities and Vulkan memory allocation helpers. |

*(Note: Shared GPU text and glyph-cache infrastructure such as `SlugImpl`, `SubRunContainer`, `StrikeCache`, and `VertexFiller` resides in `src/text/gpu/` and is compiled alongside `src/gpu/`.)*

## Build & Test

### Build Files
- **Bazel**: `src/gpu/BUILD.bazel` (target `//src/gpu:gpu`), `src/gpu/tessellate/BUILD.bazel`, `src/gpu/vk/BUILD.bazel`, `src/gpu/mtl/BUILD.bazel`, `src/gpu/android/BUILD.bazel`, and `include/gpu/BUILD.bazel`.
- **GN**: `gn/gpu.gni` (`skia_shared_gpu_sources`, `skia_shared_vk_sources`, `skia_shared_mtl_sources`, `skia_shared_android_sources`) and `gn/graphite.gni`. Note that `.gni` files are generated from Bazel rules via `make -C bazel generate_gni`.

### Testing
- **Unit Tests**: Registered in `tests/` using `DEF_TEST` (for pure CPU math like `WangsFormula` or `Swizzle`), `DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS` (for Ganesh contexts), or `DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS` (for Graphite contexts).
- **DM (`dm`)**: Run GPU rendering and unit tests across backends using `--config`:
  - **Ganesh**: `dm --config gl`, `gles`, `vk`, `mtl`, `d3d`, `mock`
  - **Graphite**: `dm --config grmtl`, `grvk`, `grdawn` (or specific Dawn backends like `grdawn_vk`, `grdawn_mtl`, `grdawn_d3d12`, `grdawn_gl`)
