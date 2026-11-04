# Ganesh GPU Backend (`src/gpu/ganesh/`)

## Overview

`src/gpu/ganesh/` implements **Ganesh**, Skia's established GPU acceleration backend (`SK_GANESH`), implementing the public API in [`include/gpu/ganesh/`](../../../include/gpu/ganesh/). Ganesh translates `SkCanvas` 2D drawing calls into retained Directed Acyclic Graphs (DAGs) of render tasks and batched draw operations compiled into native GPU API commands during flush:

* **Canvas & Surface Contexts**: `skgpu::ganesh::Device` intercepts `SkCanvas` draws, manages clip state, converts `SkPaint` to `GrPaint`, and routes geometry into `SurfaceDrawContext`.
* **Deferred Operations & Batching**: Fine-grained drawing commands (`GrOp`, `GrDrawOp`, `GrMeshDrawOp`) merge across adjacent draws to minimize GPU state changes and draw calls.
* **Composable Processor Trees**: Geometry (`GrGeometryProcessor`), fragment shading (`GrFragmentProcessor`), and blending (`GrXferProcessor`) combine into `GrPipeline` and emit modular SkSL shaders.
* **Task Graph & Virtual Proxies**: Operations record against lazy `GrSurfaceProxy` handles; on flush, `GrDrawingManager` topologically sorts the `GrRenderTask` DAG and `GrResourceAllocator` assigns physical GPU textures and buffers.
* **Multi-API Backend Execution**: `GrGpu` and `GrOpsRenderPass` implementations lower tasks into native **OpenGL / OpenGL ES / WebGL** ([`gl/`](gl/)), **Vulkan** ([`vk/`](vk/)), **Metal** ([`mtl/`](mtl/)), **Direct3D 12** ([`d3d/`](d3d/)), or CPU-only **Mock** ([`mock/`](mock/)) commands.

## Subdirectories

| Directory | Description |
| :--- | :--- |
| [`ops/`](ops/) | `GrOp` / `GrMeshDrawOp` implementations, `OpsTask`, and `PathRenderer` subclasses (atlas, tessellation, triangulating, small, and software path renderers). |
| [`effects/`](effects/) | Built-in `GrFragmentProcessor`, `GrGeometryProcessor`, and `GrXferProcessor` implementations (runtime SkSL effects, textures, blends, analytic geometry). |
| [`glsl/`](glsl/) | Shader program builders (`GrGLSLProgramBuilder`, `GrGLSLFragmentShaderBuilder`, `GrGLSLVertexGeoBuilder`) that assemble SkSL from `GrProcessor` trees. |
| [`geometry/`](geometry/) | Geometry representations (`GrShape`, `GrStyledShape`, `GrQuad`) and CPU polygon/AA triangulators (`GrTriangulator`, `GrAATriangulator`). |
| [`tessellate/`](tessellate/) | Ganesh hardware path and stroke tessellation shaders and patch allocators built on [`../tessellate/`](../tessellate/). |
| [`gradients/`](gradients/) | Gradient fragment processor builders and color ramp bitmap caching. |
| [`image/`](image/) | Ganesh-backed `SkImage` subclasses (`SkImage_Ganesh`, `SkImage_GaneshYUVA`, `SkImage_LazyTexture`, `SkImage_RasterPinnable`). |
| [`surface/`](surface/) | Ganesh-backed `SkSurface` implementation (`SkSurface_Ganesh`) and platform surface factories. |
| [`text/`](text/) | Texture atlas management (`GrAtlasManager`) for glyph strikes. |
| [`gl/`](gl/) | OpenGL, OpenGL ES, and WebGL backend (`GrGLGpu`, `GrGLCaps`, `GrGLInterface`). |
| [`vk/`](vk/) | Vulkan backend (`GrVkGpu`, `GrVkCaps`, `GrVkCommandBuffer`, `GrVkResourceProvider`). |
| [`mtl/`](mtl/) | Metal backend (`GrMtlGpu`, `GrMtlCaps`, `GrMtlCommandBuffer`, `GrMtlResourceProvider`). |
| [`d3d/`](d3d/) | Direct3D 12 backend (`GrD3DGpu`, `GrD3DCaps`, `GrD3DCommandList`, `GrD3DResourceProvider`). |
| [`mock/`](mock/) | No-op GPU backend (`GrMockGpu`, `GrMockCaps`) for unit testing task graphs, resource allocation, and op combining without a physical GPU. |

## Key Classes and Entrypoints

The primary classes and entrypoints in `src/gpu/ganesh/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Context & Device Dispatch** | [`GrDirectContextPriv.h`](GrDirectContextPriv.h), [`GrRecordingContextPriv.h`](GrRecordingContextPriv.h), [`Device.h`](Device.h), [`ClipStack.h`](ClipStack.h), [`SkGr.h`](SkGr.h) | Internal context accessors, `SkDevice` implementation (`skgpu::ganesh::Device`), GPU clip stack (`skgpu::ganesh::ClipStack`), and `SkPaint`-to-`GrPaint` conversion. |
| **Surface Contexts & Path Routing** | [`SurfaceContext.h`](SurfaceContext.h), [`SurfaceFillContext.h`](SurfaceFillContext.h), [`SurfaceDrawContext.h`](SurfaceDrawContext.h), [`PathRendererChain.h`](PathRendererChain.h), [`PathRenderer.h`](PathRenderer.h) | Surface read/write/fill/draw wrappers (`skgpu::ganesh::SurfaceDrawContext`) and strategy selection (`skgpu::ganesh::PathRendererChain`) for routing paths to specialized `PathRenderer`s. |
| **Draw Operations & Tasks** | [`GrOp.h`](ops/GrOp.h), [`GrDrawOp.h`](ops/GrDrawOp.h), [`GrMeshDrawOp.h`](ops/GrMeshDrawOp.h), [`OpsTask.h`](ops/OpsTask.h) | Batchable draw command hierarchy (`GrOp`, `GrDrawOp`, `GrMeshDrawOp`) and render pass task container (`skgpu::ganesh::OpsTask`) in [`ops/`](ops/). |
| **Processors & Program Building** | [`GrProcessor.h`](GrProcessor.h), [`GrGeometryProcessor.h`](GrGeometryProcessor.h), [`GrFragmentProcessor.h`](GrFragmentProcessor.h), [`GrXferProcessor.h`](GrXferProcessor.h), [`GrPipeline.h`](GrPipeline.h), [`GrProgramInfo.h`](GrProgramInfo.h), [`GrGLSLProgramBuilder.h`](glsl/GrGLSLProgramBuilder.h) | Vertex/coverage generation (`GrGeometryProcessor`), composable fragment shading trees (`GrFragmentProcessor`), blending (`GrXferProcessor`), fixed-function state (`GrPipeline`), and SkSL shader assembly (`GrGLSLProgramBuilder`). |
| **DAG Scheduling & Proxies** | [`GrDrawingManager.h`](GrDrawingManager.h), [`GrRenderTask.h`](GrRenderTask.h), [`GrSurfaceProxy.h`](GrSurfaceProxy.h), [`GrProxyProvider.h`](GrProxyProvider.h), [`GrResourceAllocator.h`](GrResourceAllocator.h) | Render task DAG construction and topological sorting (`GrDrawingManager`, `GrRenderTask`), deferred surface handles (`GrSurfaceProxy`, `GrTextureProxy`, `GrRenderTargetProxy`), and physical surface assignment (`GrResourceAllocator`). |
| **Backend GPU & Resource Cache** | [`GrGpu.h`](GrGpu.h), [`GrOpsRenderPass.h`](GrOpsRenderPass.h), [`GrResourceCache.h`](GrResourceCache.h), [`GrResourceProvider.h`](GrResourceProvider.h), [`GrCaps.h`](GrCaps.h) | Abstract 3D API interface (`GrGpu`), render pass command encoding (`GrOpsRenderPass`), budgeted/scratch GPU resource caching (`GrResourceCache`, `GrResourceProvider`), and hardware capability queries (`GrCaps`). |
| **Ganesh Surfaces & Images** | [`SkSurface_Ganesh.h`](surface/SkSurface_Ganesh.h), [`SkImage_GaneshBase.h`](image/SkImage_GaneshBase.h), [`SkImage_Ganesh.h`](image/SkImage_Ganesh.h), [`SkImage_GaneshYUVA.h`](image/SkImage_GaneshYUVA.h) | Ganesh-backed `SkSurface` (`SkSurface_Ganesh`) and `SkImage` (`SkImage_Ganesh`, `SkImage_GaneshYUVA`) implementations in [`surface/`](surface/) and [`image/`](image/). |

## Related Directories

| Directory | Description |
| :--- | :--- |
| [`../../../include/gpu/ganesh/`](../../../include/gpu/ganesh/) | Public `GrDirectContext`, `GrRecordingContext`, `GrBackendSurface`, and Ganesh surface/image factory headers. |
| [`../`](../) | Shared GPU utilities (`skgpu`), tessellation math, resource keys, and shared Vulkan/Metal/Android helpers. |
| [`../../text/gpu/`](../../text/gpu/) | Shared GPU text blob, glyph run, and atlas subrun infrastructure. |
