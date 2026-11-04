# Skia Shared GPU Utilities (`src/gpu/`)

## Overview

`src/gpu/` contains foundational GPU abstractions, algorithms, and backend utilities (in the `skgpu` namespace) shared between Skia's two GPU rendering backends: [`ganesh/`](ganesh/) and [`graphite/`](graphite/). Code in `src/gpu/` has no dependency on either Ganesh (`Gr*`) or Graphite (`skgpu::graphite::*`) internals:

* **Resource Keys & Caching Primitives**: Variable-length hashable keys (`skgpu::UniqueKey`, `skgpu::ScratchKey`) and builders (`skgpu::KeyBuilder`) used by resource caches across both backends.
* **Blending & Swizzling**: Hardware blend equations, Porter-Duff/advanced blend coefficient selection, shader blend formula analysis (`skgpu::BlendFormula`), and texture channel swizzling (`skgpu::Swizzle`).
* **Geometry Streaming & Atlas Packing**: Alignment-aware GPU buffer writers (`skgpu::BufferWriter`, `skgpu::VertexWriter`, `skgpu::IndexWriter`) and 2D rectangle bin-packing (`skgpu::Rectanizer`).
* **Path & Stroke Tessellation**: Backend-agnostic Bézier subdivision math (Wang's formula), curve/wedge/stroke patch writers, and polygon triangulation in [`tessellate/`](tessellate/).
* **Shader & Texture Utilities**: Common `SkSL::Compiler` backend adapter (`skgpu::SkSLToBackend`), Gaussian blur kernels, dithering tables, gradient ramps, tiled image drawing, and compressed texture utilities.
* **Shared Driver Infrastructure**: Common Vulkan, Metal, and Android `AHardwareBuffer` interfaces, memory allocators, and feature negotiation in [`vk/`](vk/), [`mtl/`](mtl/), and [`android/`](android/).

## Subdirectories

| Directory | Description |
| :--- | :--- |
| [`ganesh/`](ganesh/) | Skia's immediate/deferred-op GPU backend supporting OpenGL/ES, Vulkan, Metal, Direct3D 12, and Mock contexts. |
| [`graphite/`](graphite/) | Skia's multithreaded recorder/recording GPU backend supporting Dawn (WebGPU), Metal, and Vulkan. |
| [`tessellate/`](tessellate/) | Backend-agnostic path and stroke tessellation math, Wang's formula, patch writers, and polygon triangulation. |
| [`vk/`](vk/) | Shared Vulkan function table, memory management, mutable texture state, and feature negotiation. |
| [`mtl/`](mtl/) | Shared Metal Objective-C++ utilities and memory allocator implementation. |
| [`android/`](android/) | Shared Android `AHardwareBuffer` format/usage conversion utilities and Vulkan memory allocation helpers. |

## Key Classes and Entrypoints

The primary classes and entrypoints in `src/gpu/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Resource Keys** | [`ResourceKey.h`](ResourceKey.h), [`KeyBuilder.h`](KeyBuilder.h) | Variable-length cache keys (`skgpu::ResourceKey`, `skgpu::UniqueKey`, `skgpu::ScratchKey`) and bit/word packing (`skgpu::KeyBuilder`). |
| **Blending & Swizzling** | [`Blend.h`](Blend.h), [`BlendFormula.h`](BlendFormula.h), [`Swizzle.h`](Swizzle.h) | Blend equations/coefficients (`skgpu::BlendEquation`, `skgpu::BlendCoeff`), coverage-aware blend analysis (`skgpu::BlendFormula`), and RGBA channel swizzling (`skgpu::Swizzle`). |
| **Buffers & Atlas Packing** | [`BufferWriter.h`](BufferWriter.h), [`Rectanizer.h`](Rectanizer.h), [`RectanizerSkyline.h`](RectanizerSkyline.h), [`RectanizerPow2.h`](RectanizerPow2.h) | Bounds-checked vertex/index/upload buffer writers (`skgpu::VertexWriter`, `skgpu::IndexWriter`, `skgpu::TextureUploadWriter`) and 2D atlas rectangle packers (`skgpu::Rectanizer`). |
| **Path & Stroke Tessellation** | [`Tessellation.h`](tessellate/Tessellation.h), [`WangsFormula.h`](tessellate/WangsFormula.h), [`PatchWriter.h`](tessellate/PatchWriter.h), [`StrokeIterator.h`](tessellate/StrokeIterator.h), [`MiddleOutPolygonTriangulator.h`](tessellate/MiddleOutPolygonTriangulator.h) | Parametric curve segment calculation (`skgpu::wangs_formula`), GPU patch streaming (`skgpu::tess::PatchWriter`), stroke contour iteration, and middle-out fan triangulation in [`tessellate/`](tessellate/). |
| **Shading, Blur & Texture Helpers** | [`SkSLToBackend.h`](SkSLToBackend.h), [`BlurUtils.h`](BlurUtils.h), [`DitherUtils.h`](DitherUtils.h), [`GradientBitmap.h`](GradientBitmap.h), [`TiledTextureUtils.h`](TiledTextureUtils.h), [`DataUtils.h`](DataUtils.h) | SkSL-to-backend compiler wrapper (`skgpu::SkSLToBackend`), Gaussian blur kernels/profiles, dither LUTs, gradient color/offset F16 textures, large-image tiling (`skgpu::TiledTextureUtils`), and compressed texture block sizing. |
| **Shared Backend Utilities** | [`VulkanInterface.h`](vk/VulkanInterface.h), [`VulkanMemory.h`](vk/VulkanMemory.h), [`MtlUtilsPriv.h`](mtl/MtlUtilsPriv.h), [`MtlMemoryAllocatorImpl.h`](mtl/MtlMemoryAllocatorImpl.h) | Shared Vulkan function table (`skgpu::VulkanInterface`) and allocator helpers (`skgpu::VulkanMemory`) in [`vk/`](vk/), plus shared Metal utilities and `skgpu::MtlMemoryAllocatorImpl` in [`mtl/`](mtl/). |

## Related Directories

| Directory | Description |
| :--- | :--- |
| [`../../include/gpu/`](../../include/gpu/) | Public GPU types, mutable texture state, shader error handling, and backend context headers. |
| [`../text/gpu/`](../text/gpu/) | Shared GPU glyph caching, strike serialization (`Slug`), and text atlas vertex generation. |
