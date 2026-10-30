# Skia Core 2D Engine & CPU Backend (`src/core/`)

## Overview & Scope

`src/core/` is the heart of Skia's 2D graphics engine. It implements the public API declared in [`include/core/`](../../include/core/) and provides the shared infrastructure used across the CPU rasterizer and GPU backends (Ganesh and Graphite).

Its responsibilities span:
1. **Frontend Canvas & Device Dispatch**: Managing transform/clip stacks, layer orchestration, and routing draw calls from `SkCanvas` to backend-specific `SkDevice` subclasses.
2. **Display-List Recording & Serialization**: Capturing draw commands into `SkPicture` (`SkRecord`) and serializing `SkFlattenable` objects (`SkReadBuffer`, `SkWriteBuffer`).
3. **Geometry, Paths & Scan Conversion**: Bézier math, path stroking (`SkStroke`), clipping, and CPU scanline/analytic anti-aliased rasterization (`SkScan`).
4. **CPU Shading & Blitting**: `SkRasterPipeline` (stage-based SIMD interpreter) and `SkBlitter` subclasses that write shaded coverage into destination pixel buffers.
5. **Typography & Strike Caching**: Converting positioned glyph runs into masks, paths, or drawables via `SkScalerContext`, `SkStrike`, and `SkStrikeCache`.
6. **Image Filtering & Blur Engines**: Core coordinate-space math and DAG evaluation for image filters (`skif::Context`, `SkBlurEngine`).

Headers in `src/core/` are strictly internal to Skia. Headers listed in `SEMI_PUBLIC_HEADERS` in [`BUILD.bazel`](BUILD.bazel) are exposed to other `src/` modules via `//src/core:core_priv`, while `PRIVATE_HEADERS` are restricted to `src/core/`, tests, and GMs.

## Key Abstractions & Entrypoints

### 1. Canvas -> Device -> Draw Control Flow
* [`SkCanvas.cpp`](SkCanvas.cpp) / [`SkCanvasPriv.h`](SkCanvasPriv.h) (implements [`include/core/SkCanvas.h`](../../include/core/SkCanvas.h)): Maintains the save/restore stack, current transformation matrix (CTM), quick-reject bounds, and layer stack. Delegates rendering to the top `SkDevice` on its stack.
* [`SkDevice.h`](SkDevice.h) / [`SkDevice.cpp`](SkDevice.cpp): Abstract backend target backing an `SkCanvas` or `saveLayer`. Subclassed by [`SkBitmapDevice.h`](SkBitmapDevice.h) (CPU raster), `skgpu::ganesh::Device` (Ganesh), `skgpu::graphite::Device` (Graphite), `SkClipStackDevice`, and `SkPDFDevice`.
* [`SkDraw.h`](SkDraw.h) / [`SkDraw.cpp`](SkDraw.cpp): Stateless helper invoked by `SkBitmapDevice` that binds the destination `SkPixmap`, matrix, `SkRasterClip`, and `SkPaint` to execute CPU geometry, vertices, atlas, and glyph draws.

### 2. CPU Rasterization (`SkScan`, `SkBlitter`, `SkRasterPipeline`)
* [`SkScan.h`](SkScan.h) (`SkScan_Path.cpp`, `SkScan_AntiPath.cpp`, `SkScan_AAAPath.cpp`, `SkScan_Hairline.cpp`): Rasterizes geometric primitives and `SkPath` edges ([`SkEdge.h`](SkEdge.h), [`SkAnalyticEdge.h`](SkAnalyticEdge.h)) into horizontal spans or anti-aliased coverage masks (`SkMask`), forwarding coverage to an `SkBlitter`.
* [`SkBlitter.h`](SkBlitter.h) / [`SkAutoBlitterChoose.h`](SkAutoBlitterChoose.h): Consumes scanline segments `(x, y, width, alpha)` and composites pixels into destination memory. Specialized implementations include `SkBlitter_ARGB32.cpp`, [`SkBlitter_A8.h`](SkBlitter_A8.h), [`SkSpriteBlitter.h`](SkSpriteBlitter.h), and `SkRasterPipelineBlitter.cpp`.
* [`SkRasterPipeline.h`](SkRasterPipeline.h) / [`SkRasterPipelineOpList.h`](SkRasterPipelineOpList.h): Skia's primary high-precision CPU pixel processor. Chains composable stages (`SkShaderBase::appendStages`, `SkColorFilterBase::appendStages`, `SkBlenderBase`, color space transforms, and destination reads/writes) into a compact array of function pointers executed via tail-call SIMD routines.

### 3. Recording & Serialization (`SkPicture` & `SkRecord`)
* [`SkRecord.h`](SkRecord.h) / [`SkRecordCanvas.h`](SkRecordCanvas.h) / [`SkRecords.h`](SkRecords.h): Records `SkCanvas` commands into an arena-backed sequence of typed command structs, optimized by [`SkRecordOpts.h`](SkRecordOpts.h) and replayed via [`SkRecordDraw.h`](SkRecordDraw.h) or serialized via [`SkPictureData.h`](SkPictureData.h).
* [`SkReadBuffer.h`](SkReadBuffer.h) / [`SkWriteBuffer.h`](SkWriteBuffer.h): Safe binary serialization and deserialization streams for `SkPicture` and `SkFlattenable` subclasses.

### 4. Geometry, Paths, Text & Image Filters
* **Paths & Stroking**: [`SkPathPriv.h`](SkPathPriv.h), [`SkPathData.h`](SkPathData.h), [`SkPathRaw.h`](SkPathRaw.h), [`SkGeometry.h`](SkGeometry.h), and [`SkStroke.h`](SkStroke.h) implement path storage, Bézier chopping, and stroke expansion.
* **Text & Glyph Caching**: [`SkGlyphRunPainter.h`](SkGlyphRunPainter.h) works with `src/text/` (`GlyphRunList`), [`SkStrikeSpec.h`](SkStrikeSpec.h), [`SkStrike.h`](SkStrike.h), [`SkStrikeCache.h`](SkStrikeCache.h), and [`SkScalerContext.h`](SkScalerContext.h) (implemented by platform font ports in `src/ports/`) to rasterize and cache glyph metrics, bitmaps, and paths.
* **Image Filter Infrastructure**: [`SkImageFilter_Base.h`](SkImageFilter_Base.h) and [`SkImageFilterTypes.h`](SkImageFilterTypes.h) (`namespace skif`) provide strongly-typed coordinate spaces (`ParameterSpace`, `LayerSpace`, `DeviceSpace`), `skif::FilterResult`, and [`SkBlurEngine.h`](SkBlurEngine.h) used by [`../effects/imagefilters/`](../effects/README.md).
* **Memory & Concurrency**: [`SkArenaAlloc.h`](SkArenaAlloc.h), [`SkBlockAllocator.h`](SkBlockAllocator.h), [`SkTHash.h`](SkTHash.h), [`SkResourceCache.h`](SkResourceCache.h), and [`SkTaskGroup.h`](SkTaskGroup.h).

## Subdirectories & Related Modules

`src/core/` has no subdirectories, but directly aggregates and coordinates with sibling modules:
* **SIMD Optimizations (`src/opts/`)**: [`SkOpts.h`](SkOpts.h) and `Sk*_opts*.cpp` dispatch hot loops (`SkRasterPipeline` stages, `SkBlitRow`, `SkBlitMask`, `SkSwizzler`, `SkBitmapProcState`, `SkMemset`) to architecture-specific SIMD implementations (AVX2/AVX-512, SSSE3, ARM NEON, LoongArch LASX) in `src/opts/` based on runtime CPU detection ([`SkCpu.h`](SkCpu.h)).
* **Sibling Core Subsystems**:
  * [`../image/`](../image/README.md): `SkImage` and `SkSurface` base classes and raster/lazy/picture implementations.
  * [`../shaders/`](../shaders/README.md): `SkShaderBase` and built-in/gradient shaders.
  * [`../effects/`](../effects/README.md): Path effects, mask filters, color filters, and image filters.
  * `../text/` & `../sfnt/`: `GlyphRunBuilder`, `GlyphRunList`, and OpenType table utilities.
  * `../sksl/`: The SkSL compiler and `SkRasterPipeline` code generator (`//src/sksl/codegen:rasterpipeline`).
  * [`../gpu/ganesh/`](../gpu/ganesh/README.md) & [`../gpu/graphite/`](../gpu/graphite/README.md): GPU backend implementations.

## Build & Test

* **Bazel**: Defined in [`BUILD.bazel`](BUILD.bazel). Key targets:
  * `//src/core:core`: Primary library compiling `core_srcs` alongside sources from `src/image`, `src/shaders`, `src/effects`, `src/text`, `src/sksl`, `src/sfnt`, `src/utils`, and `src/capture`.
  * `//src/core:core_priv`: Exposes `SEMI_PUBLIC_HEADERS` (`core_priv_hdrs`) to internal Skia modules, `tests/`, `gm/`, and `bench/`.
  * `//src/core:core_skslc`: Minimal subset of core utilities (`SkArenaAlloc`, `SkString`, `SkData`, `SkStream`, `SkMathPriv`) linked into the standalone `skslc` compiler.
* **GN**: Exported to `gn/core.gni` (`skia_core_sources`). Run `make -C bazel generate_gni` after changing file lists in `BUILD.bazel`.
* **Testing**: Unit tests live in `tests/` (e.g., `CanvasTest.cpp`, `PathTest.cpp`, `MatrixTest.cpp`, `RecordDrawTest.cpp`, `SkRasterPipelineTest.cpp`, `StrikeForGPUTest.cpp`) and visual rendering tests in `gm/`.
