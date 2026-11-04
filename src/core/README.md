# Skia Core 2D Engine (`src/core/`)

## Overview

`src/core/` is the heart of Skia's 2D graphics engine.
It implements the core public API declared in [`include/core/`](../../include/core/) and provides the shared infrastructure used across the CPU rasterizer and GPU backends:

* **Canvas & Device Dispatch**: Manages transform/clip stacks, layer orchestration, and routing draw calls from `SkCanvas` to backend-specific `SkDevice` subclasses.
* **Display-List Recording & Serialization**: Captures draw commands into `SkPicture` (`SkRecord`) and serializes `SkFlattenable` objects.
* **Geometry, Paths & Scan Conversion**: Bézier math, path stroking, clipping, and CPU scanline/analytic anti-aliased rasterization.
* **CPU Shading & Blitting**: Stage-based SIMD pixel processing (`SkRasterPipeline`) and blitters that composite shaded coverage into destination memory.
* **Typography & Strike Caching**: Converts positioned glyph runs into masks, paths, or drawables via platform font scalers and glyph caches.
* **Image Filtering Infrastructure**: Coordinate-space math and DAG evaluation for multi-pass image filters and blurs.

## Key Classes and Entrypoints

The primary classes and entrypoints in `src/core/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Canvas & Device Dispatch** | [`SkCanvas.cpp`](SkCanvas.cpp), [`SkCanvasPriv.h`](SkCanvasPriv.h), [`SkDevice.h`](SkDevice.h), [`SkBitmapDevice.h`](SkBitmapDevice.h), [`SkDraw.h`](SkDraw.h) | Transform/clip/layer stack management (`SkCanvas`), abstract backend target (`SkDevice`), CPU raster device (`SkBitmapDevice`), and CPU draw state (`skcpu::Draw`). |
| **CPU Rasterization** | [`SkScan.h`](SkScan.h), [`SkBlitter.h`](SkBlitter.h), [`SkRasterPipeline.h`](SkRasterPipeline.h), [`SkOpts.h`](SkOpts.h), [`SkCpu.h`](SkCpu.h) | Scanline/anti-aliased path rasterization (`SkScan`), pixel compositing (`SkBlitter`), SIMD stage pipeline (`SkRasterPipeline`), and runtime CPU feature dispatch (`SkOpts`). |
| **Recording & Serialization** | [`SkRecord.h`](SkRecord.h), [`SkRecordCanvas.h`](SkRecordCanvas.h), [`SkReadBuffer.h`](SkReadBuffer.h), [`SkWriteBuffer.h`](SkWriteBuffer.h) | Arena-backed display-list recording for `SkPicture` (`SkRecord`, `SkRecordCanvas`) and safe binary serialization for `SkFlattenable` objects. |
| **Paths & Stroking** | [`SkPathPriv.h`](SkPathPriv.h), [`SkGeometry.h`](SkGeometry.h), [`SkStroke.h`](SkStroke.h) | Internal path storage, Bézier curve math, and stroke expansion. |
| **Text & Glyph Caching** | [`SkGlyphRunPainter.h`](SkGlyphRunPainter.h), [`SkStrikeCache.h`](SkStrikeCache.h), [`SkScalerContext.h`](SkScalerContext.h) | Glyph run rasterization (`skcpu::GlyphRunListPainter`), strike caching (`SkStrikeCache`), and platform font scaler interface (`SkScalerContext`). |
| **Image Filter Framework** | [`SkImageFilter_Base.h`](SkImageFilter_Base.h), [`SkImageFilterTypes.h`](SkImageFilterTypes.h), [`SkBlurEngine.h`](SkBlurEngine.h) | Strongly-typed coordinate spaces (`namespace skif`), filter graph evaluation, and shared blur engine used by [`../effects/imagefilters/`](../effects/imagefilters/). |
| **Memory & Caching** | [`SkArenaAlloc.h`](SkArenaAlloc.h), [`SkTHash.h`](SkTHash.h), [`SkResourceCache.h`](SkResourceCache.h) | Arena allocation, hash containers, and global resource caching. |

## Related Directories

| Directory | Description |
| :--- | :--- |
| [`../opts/`](../opts/) | Architecture-specific SIMD implementations (AVX2/AVX-512, SSSE3, ARM NEON, LoongArch LASX) dispatched via [`SkOpts.h`](SkOpts.h). |
| [`../image/`](../image/) | `SkImage` and `SkSurface` base classes and CPU/deferred implementations. |
| [`../shaders/`](../shaders/) | `SkShaderBase` and built-in/gradient shader implementations. |
| [`../effects/`](../effects/) | Path effects, mask filters, color filters, and image filters. |
| [`../text/`](../text/) | Glyph run building and GPU text atlas management. |
