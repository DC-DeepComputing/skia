# Skia Core 2D Engine (`src/core/`)

## Overview

`src/core/` is the heart of Skia's 2D graphics engine. It implements the core public API declared in [`include/core/`](../../include/core/) and provides the shared infrastructure used across the CPU rasterizer and GPU backends:

* **Canvas & Device Dispatch**: Manages transform/clip stacks, layer orchestration, and routing draw calls from `SkCanvas` to backend-specific `SkDevice` subclasses.
* **Display-List Recording & Serialization**: Captures draw commands into `SkPicture` (`SkRecord`) and serializes `SkFlattenable` objects.
* **Geometry, Paths & Scan Conversion**: Bézier math, path stroking, clipping, and CPU scanline/analytic anti-aliased rasterization.
* **CPU Shading & Blitting**: Stage-based SIMD pixel processing (`SkRasterPipeline`) and blitters that composite shaded coverage into destination memory.
* **Typography & Strike Caching**: Converts positioned glyph runs into masks, paths, or drawables via platform font scalers and glyph caches.
* **Image Filtering Infrastructure**: Coordinate-space math and DAG evaluation for multi-pass image filters and blurs.

## Key Classes and Entrypoints

### Canvas and Device Dispatch
* [`SkCanvas.cpp`](SkCanvas.cpp) / [`SkCanvasPriv.h`](SkCanvasPriv.h): Maintains the save/restore stack, current transformation matrix (CTM), quick-reject bounds, and layer stack, delegating rendering to the top `SkDevice`.
* [`SkDevice.h`](SkDevice.h): Abstract backend target backing an `SkCanvas` or offscreen layer. Subclassed by [`SkBitmapDevice.h`](SkBitmapDevice.h) for CPU rasterization, as well as GPU, PDF, and clip-tracking devices.
* [`SkDraw.h`](SkDraw.h): Defines `skcpu::Draw`, which binds the destination `SkPixmap`, matrix, and clip (`SkRasterClip`) to execute CPU geometry, vertices, atlas, and glyph draws.

### CPU Rasterization
* [`SkScan.h`](SkScan.h): Rasterizes geometric primitives and path edges into horizontal spans or anti-aliased coverage masks (`SkMask`), forwarding coverage to an `SkBlitter`.
* [`SkBlitter.h`](SkBlitter.h): Consumes scanline segments `(x, y, width, alpha)` and composites pixels into destination memory.
* [`SkRasterPipeline.h`](SkRasterPipeline.h): Skia's primary high-precision CPU pixel processor. Chains composable shader, color filter, blender, and color-space stages into a compact sequence of SIMD routines.
* [`SkOpts.h`](SkOpts.h) & [`SkCpu.h`](SkCpu.h): Runtime CPU feature detection and dispatch table connecting hot loops to architecture-specific SIMD implementations in [`../opts/`](../opts/).

### Recording and Serialization
* [`SkRecord.h`](SkRecord.h) & [`SkRecordCanvas.h`](SkRecordCanvas.h): Records `SkCanvas` draw calls into an arena-backed sequence of command structs for `SkPicture` optimization and playback.
* [`SkReadBuffer.h`](SkReadBuffer.h) & [`SkWriteBuffer.h`](SkWriteBuffer.h): Safe binary serialization and deserialization streams for `SkPicture` and `SkFlattenable` objects.

### Paths, Text, and Image Filters
* **Paths & Stroking**: [`SkPathPriv.h`](SkPathPriv.h), [`SkGeometry.h`](SkGeometry.h), and [`SkStroke.h`](SkStroke.h) handle internal path storage, Bézier math, and stroke expansion.
* **Text & Glyph Caching**: [`SkGlyphRunPainter.h`](SkGlyphRunPainter.h), [`SkStrikeCache.h`](SkStrikeCache.h), and [`SkScalerContext.h`](SkScalerContext.h) rasterize and cache glyph metrics, bitmaps, and paths.
* **Image Filter Framework**: [`SkImageFilter_Base.h`](SkImageFilter_Base.h), [`SkImageFilterTypes.h`](SkImageFilterTypes.h) (`namespace skif`), and [`SkBlurEngine.h`](SkBlurEngine.h) provide strongly-typed coordinate spaces and filter evaluation used by [`../effects/imagefilters/`](../effects/imagefilters/).
* **Memory & Caching**: [`SkArenaAlloc.h`](SkArenaAlloc.h), [`SkTHash.h`](SkTHash.h), and [`SkResourceCache.h`](SkResourceCache.h).

## Related Directories

| Directory | Description |
| :--- | :--- |
| [`../opts/`](../opts/) | Architecture-specific SIMD implementations (AVX2/AVX-512, SSSE3, ARM NEON, LoongArch LASX) dispatched via [`SkOpts.h`](SkOpts.h). |
| [`../image/`](../image/) | `SkImage` and `SkSurface` base classes and CPU/deferred implementations. |
| [`../shaders/`](../shaders/) | `SkShaderBase` and built-in/gradient shader implementations. |
| [`../effects/`](../effects/) | Path effects, mask filters, color filters, and image filters. |
| [`../text/`](../text/) | Glyph run building and GPU text atlas management. |
