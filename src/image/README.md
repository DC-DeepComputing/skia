# Skia Image & Surface Implementations (`src/image/`)

## Overview & Scope

`src/image/` implements the core base classes and CPU/backend-agnostic subclasses for [`SkImage`](../../include/core/SkImage.h) (immutable pixel/texture sources) and [`SkSurface`](../../include/core/SkSurface.h) (drawing destinations).

Architecturally, `src/image/` bridges the public API in [`include/core/`](../../include/README.md) (`SkImage.h`, `SkSurface.h`, `SkImageGenerator.h`, `SkTiledImageUtils.h`) with the rendering engines in [`src/core/`](../core/README.md) and the GPU backends (`src/gpu/ganesh/` and `src/gpu/graphite/`):
* **Public API**: Clients interact exclusively with `sk_sp<SkImage>` and `sk_sp<SkSurface>`, creating instances via factory namespaces (`SkImages::*`, `SkSurfaces::*`).
* **Internal Base Classes**: `SkImage_Base` and `SkSurface_Base` define the internal virtual dispatch table used across Skia (`as_IB(image)` and `as_SB(surface)`), hiding backend storage details from callers.
* **CPU & Deferred Implementations**: Raster, lazy (generator-backed), picture-backed, and null implementations live directly in `src/image/`, whereas GPU-backed subclasses live in `src/gpu/ganesh/image/` (`SkImage_Ganesh`, `SkSurface_Ganesh`) and `src/gpu/graphite/` (`skgpu::graphite::Image`, `skgpu::graphite::Surface`).

## Key Abstractions & Entrypoints

### 1. `SkImage` Hierarchy
* [`SkImage.cpp`](SkImage.cpp) & [`SkImage_Base.h`](SkImage_Base.h) / [`SkImage_Base.cpp`](SkImage_Base.cpp): Implements public `SkImage` entrypoints and defines `SkImage_Base`. Key internal hooks on `SkImage_Base` include `onReadPixels()`, `onPeekPixels()`, `getROPixels()`, `onMakeSubset()`, `onAsyncRescaleAndReadPixels()`, and mipmap management (`onPeekMips()`, `refMips()`).
* [`SkImage_Raster.h`](SkImage_Raster.h) / [`SkImage_Raster.cpp`](SkImage_Raster.cpp) / [`SkImage_RasterFactories.cpp`](SkImage_RasterFactories.cpp): CPU-backed `SkImage` wrapping an immutable or copy-on-write `SkBitmap` (`SkPixelRef`). Supports `SkImages::RasterFromBitmap`, `RasterFromPixmap`, `RasterFromData`, and `RasterFromCompressedTextureData`.
* [`SkImage_Lazy.h`](SkImage_Lazy.h) / [`SkImage_Lazy.cpp`](SkImage_Lazy.cpp) / [`SkImage_LazyFactories.cpp`](SkImage_LazyFactories.cpp): Deferred-decoding `SkImage` backed by a thread-safe `SharedGenerator` wrapping an [`SkImageGenerator`](../../include/core/SkImageGenerator.h) (such as an `SkCodec`). Decodes pixels on demand into `SkBitmapCache` for CPU draws or uploads directly to GPU textures when sampled by Ganesh/Graphite.
* [`SkImage_Picture.h`](SkImage_Picture.h) / [`SkImage_Picture.cpp`](SkImage_Picture.cpp) & [`SkPictureImageGenerator.h`](SkPictureImageGenerator.h): Deferred `SkImage_Lazy` subclass backed by an `SkPicture` (`SkImages::DeferredFromPicture`), replay-rasterizing the picture when pixels are requested.

### 2. `SkSurface` Hierarchy
* [`SkSurface.cpp`](SkSurface.cpp) & [`SkSurface_Base.h`](SkSurface_Base.h) / [`SkSurface_Base.cpp`](SkSurface_Base.cpp): Implements `SkSurface` and defines `SkSurface_Base`. Manages the cached `SkCanvas` (`getCanvas()` -> `onNewCanvas()`), copy-on-write image snapshots (`fCachedImage` / `aboutToDraw(ContentChangeMode)` / `onNewImageSnapshot()`), and capabilities.
* [`SkSurface_Raster.h`](SkSurface_Raster.h) / [`SkSurface_Raster.cpp`](SkSurface_Raster.cpp): CPU-backed `SkSurface` (`SkSurfaces::Raster` and `SkSurfaces::WrapPixels`). Owns or wraps an `SkPixelRef`, creates an `SkCanvas` backed by an `SkBitmapDevice`, and triggers copy-on-write pixel duplication in `onCopyOnWrite()` if a live `SkImage_Raster` snapshot is outstanding when a new draw occurs.
* [`SkSurface_Null.cpp`](SkSurface_Null.cpp): Implements `SkSurfaces::Null(width, height)` for lightweight headless/testing canvases that discard draw output without allocating pixel memory.

### 3. Rescaling & Tiled Drawing Utilities
* [`SkRescaleAndReadPixels.h`](SkRescaleAndReadPixels.h) / [`SkRescaleAndReadPixels.cpp`](SkRescaleAndReadPixels.cpp): Shared multi-step rescaling helper (`SkRescaleAndReadPixels`) used by `SkImage_Base` and `SkSurface_Base` fallback paths to perform gamma-correct (`RescaleGamma`) and filter-controlled (`RescaleMode`) image scaling and readback.
* [`SkTiledImageUtils.cpp`](SkTiledImageUtils.cpp) (implements [`include/core/SkTiledImageUtils.h`](../../include/core/SkTiledImageUtils.h)): Breaks large `SkImage` or `SkBitmap` draws into smaller tiles (`DrawImageRect`, `DrawImage`) so they fit within GPU maximum texture dimension limits.

## Subdirectories & Related Modules

`src/image/` contains no subdirectories, and interacts closely with:
* [`../core/`](../core/README.md): Consumes `SkBitmap`, `SkPixmap`, `SkBitmapDevice`, `SkMipmap`, and `SkBitmapCache`.
* [`../shaders/`](../shaders/README.md): `SkImageShader` samples `SkImage_Base` instances across CPU (`SkRasterPipeline`) and GPU pipelines.
* [`../gpu/ganesh/`](../gpu/ganesh/README.md) & [`../gpu/graphite/`](../gpu/graphite/README.md): Implement GPU-backed `SkImage_Base` and `SkSurface_Base` subclasses.
* `../codec/`: Provides `SkCodecImageGenerator` backing `SkImages::DeferredFromEncodedData`.

## Build & Test

* **Bazel**: [`BUILD.bazel`](BUILD.bazel) defines `image_srcs` and `image_hdrs` via `split_srcs_and_hdrs(name = "image")`, which are compiled directly into `//src/core:core` and exposed to internal modules via `//src/core:core_priv`. Android-specific factory stubs (`SkImage_AndroidFactories.cpp`) are grouped in `//src/image:android_srcs`.
* **GN**: Exported into `gn/core.gni` (`skia_core_sources`). Run `make -C bazel generate_gni` after modifying `BUILD.bazel`.
* **Testing**: Unit tests live in `tests/` (`ImageTest.cpp`, `SurfaceTest.cpp`, `ImageGeneratorTest.cpp`, `ReadWritePixelsGpuTest.cpp`, `TiledImageUtilsTest.cpp`) and visual tests in `gm/` (`image.cpp`, `surface.cpp`, `asyncrescaleandread.cpp`).
