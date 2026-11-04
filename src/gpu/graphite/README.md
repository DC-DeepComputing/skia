# Graphite GPU Backend (`src/gpu/graphite/`)

## Overview

`src/gpu/graphite/` implements **Graphite**, Skia's modern GPU rendering backend (`SK_GRAPHITE`, namespace `skgpu::graphite`), implementing the public API in [`include/gpu/graphite/`](../../../include/gpu/graphite/). Graphite targets modern explicit graphics APIs—**Dawn / WebGPU** ([`dawn/`](dawn/)), **Metal** ([`mtl/`](mtl/)), and **Vulkan** ([`vk/`](vk/))—with an architecture centered on:

* **Multithreaded Recording**: Decouples thread-local `Recorder` instances from the central `Context` so multiple threads can record `Recording` command sequences concurrently without lock contention.
* **Pipeline Precompilation**: Separates geometry rendering techniques (`Renderer` / `RenderStep`) from shading trees (`PaintParamsKey` / `ShaderCodeDictionary`), allowing applications to pre-generate pipelines ahead of time via `Precompile` APIs.
* **Unified Draw Ordering & Batching**: Uses painter's order depth-buffer testing (`DrawOrder`) and dynamic vertex/instance streaming (`DrawWriter`, `DrawBufferManager`) to reorder and batch draws across an entire render pass without heavy CPU op-merging overhead.
* **Explicit Task & Compute Graphs**: Organizes GPU work into explicit `TaskList` graphs (`RenderPassTask`, `UploadTask`, `CopyTask`, `ComputeTask`) lowered into native command buffers by `QueueManager`.

## Subdirectories

| Directory | Description |
| :--- | :--- |
| [`draw/`](draw/) | Draw recording, ordering, and render-pass sorting (`DrawContext`, `DrawListBase`, `DrawList`, `DrawListLayer`, `DrawPass`, `DrawWriter`, `DrawOrder`). |
| [`render/`](render/) | `RenderStep` implementations for analytic primitives, hardware tessellation, stencil-then-cover, text, vertices, and coverage masks. |
| [`geom/`](geom/) | Geometric primitives and spatial indexing (`Shape`, `Rect`, `Transform`, `EdgeAAQuad`, `BoundsManager`, `IntersectionTree`, `AnalyticBlurMask`). |
| [`task/`](task/) | Execution task graph nodes (`Task`, `TaskList`, `RenderPassTask`, `DrawTask`, `UploadTask`, `ComputeTask`, `CopyTask`). |
| [`compute/`](compute/) | GPU compute pipeline abstractions (`ComputeStep`, `DispatchGroup`) and optional Vello compute path stages (`VelloRenderer`, `VelloComputeSteps`). |
| [`precompile/`](precompile/) | Combinatorial `PaintOptions` and `Precompile` hierarchy for ahead-of-time shader and pipeline compilation. |
| [`sparse_strips/`](sparse_strips/) | Experimental CPU/SIMD sparse-strip path rasterization and wide-tile rendering. |
| [`text/`](text/) | Text atlas management (`TextAtlasManager`, `TextStrike`) working with `DrawAtlas` and [`../../text/gpu/`](../../text/gpu/). |
| [`surface/`](surface/) | Platform-specific `SkSurface` factories (such as Android `AHardwareBuffer` surface creation). |
| [`dawn/`](dawn/) | Dawn / WebGPU backend (`DawnSharedContext`, `DawnCommandBuffer`, `DawnGraphicsPipeline`, `DawnResourceProvider`, `DawnCaps`). |
| [`mtl/`](mtl/) | Metal backend (`MtlSharedContext`, `MtlCommandBuffer`, `MtlGraphicsPipeline`, `MtlResourceProvider`, `MtlCaps`). |
| [`vk/`](vk/) | Vulkan backend (`VulkanSharedContext`, `VulkanCommandBuffer`, `VulkanGraphicsPipeline`, `VulkanResourceProvider`, `VulkanCaps`). |

## Key Classes and Entrypoints

The primary classes and entrypoints in `src/gpu/graphite/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Context, Recorder & Recording** | [`ContextPriv.h`](ContextPriv.h), [`SharedContext.h`](SharedContext.h), [`RecorderPriv.h`](RecorderPriv.h), [`RecordingPriv.h`](RecordingPriv.h), [`QueueManager.h`](QueueManager.h) | Central GPU state (`Context`, `SharedContext`), thread-local recording context (`Recorder`), snapped command package (`Recording`), and GPU queue submission (`QueueManager`). |
| **Device, Surfaces & Images** | [`Device.h`](Device.h), [`ClipStack.h`](ClipStack.h), [`Surface_Graphite.h`](Surface_Graphite.h), [`Image_Base_Graphite.h`](Image_Base_Graphite.h), [`Image_Graphite.h`](Image_Graphite.h), [`Image_YUVA_Graphite.h`](Image_YUVA_Graphite.h) | `SkDevice` subclass (`skgpu::graphite::Device`), clipping (`ClipStack`), and Graphite-backed `SkSurface` / `SkImage` implementations. |
| **Draw Recording & Sorting** | [`DrawContext.h`](draw/DrawContext.h), [`DrawListBase.h`](draw/DrawListBase.h), [`DrawList.h`](draw/DrawList.h), [`DrawListLayer.h`](draw/DrawListLayer.h), [`DrawPass.h`](draw/DrawPass.h), [`DrawWriter.h`](draw/DrawWriter.h), [`DrawOrder.h`](draw/DrawOrder.h) | Surface draw accumulation (`DrawContext`, `DrawList`, `DrawListLayer`), depth-based painter's order (`DrawOrder`), sorted render-pass generation (`DrawPass`), and GPU vertex/instance streaming (`DrawWriter`) in [`draw/`](draw/). |
| **Renderers & Atlases** | [`Renderer.h`](Renderer.h), [`RendererProvider.h`](RendererProvider.h), [`AtlasProvider.h`](AtlasProvider.h), [`DrawAtlas.h`](DrawAtlas.h), [`PathAtlas.h`](PathAtlas.h), [`RasterPathAtlas.h`](RasterPathAtlas.h), [`ComputePathAtlas.h`](ComputePathAtlas.h) | Multi-step geometry techniques (`Renderer`, `RenderStep`) in [`render/`](render/) and dynamic texture atlases for glyphs, clip masks, and raster/compute paths. |
| **Shading, Keys & Precompilation** | [`PaintParams.h`](PaintParams.h), [`PaintParamsKey.h`](PaintParamsKey.h), [`KeyContext.h`](KeyContext.h), [`KeyHelpers.h`](KeyHelpers.h), [`ShaderCodeDictionary.h`](ShaderCodeDictionary.h), [`ShaderInfo.h`](ShaderInfo.h), [`PipelineManager.h`](PipelineManager.h) | `SkPaint` extraction (`PaintParams`), compact shader tree serialization (`PaintParamsKey`, `KeyHelpers`), snippet deduplication and SkSL generation (`ShaderCodeDictionary`, `ShaderInfo`), and pipeline creation/precompilation (`PipelineManager`, [`precompile/`](precompile/)). |
| **Tasks, Compute & Resources** | [`Task.h`](task/Task.h), [`TaskList.h`](task/TaskList.h), [`RenderPassTask.h`](task/RenderPassTask.h), [`ComputeStep.h`](compute/ComputeStep.h), [`ResourceProvider.h`](ResourceProvider.h), [`ResourceCache.h`](ResourceCache.h), [`BufferManager.h`](BufferManager.h), [`CommandBuffer.h`](CommandBuffer.h), [`Caps.h`](Caps.h) | Task graph execution (`TaskList`, `RenderPassTask`) in [`task/`](task/), compute dispatches (`ComputeStep`) in [`compute/`](compute/), resource caching/allocation (`ResourceProvider`, `ResourceCache`, `DrawBufferManager`), and backend command encoding (`CommandBuffer`). |

## Related Directories

| Directory | Description |
| :--- | :--- |
| [`../../../include/gpu/graphite/`](../../../include/gpu/graphite/) | Public `Context`, `Recorder`, `Recording`, `BackendTexture`, `Precompile`, and Graphite surface/image factory headers. |
| [`../`](../) | Shared GPU utilities (`skgpu`), tessellation math, resource keys, and shared Vulkan/Metal/Android helpers. |
| [`../../sksl/`](../../sksl/) | SkSL compiler and Graphite built-in vertex/fragment shader module snippets. |
| [`../../text/gpu/`](../../text/gpu/) | Shared GPU text blob, glyph run, and atlas subrun infrastructure. |
