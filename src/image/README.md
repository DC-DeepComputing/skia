# Skia Image and Surface Implementations (`src/image/`)

## Overview

`src/image/` implements the internal base classes and CPU/deferred subclasses for `SkImage` (immutable pixel or texture sources) and `SkSurface` (drawing destinations):

* **Internal Base Classes**: `SkImage_Base` and `SkSurface_Base` define the virtual dispatch table (`as_IB(image)` and `asSB(surface)`) used across Skia to abstract over CPU and GPU backing stores.
* **CPU and Deferred Implementations**: Raster, lazy (generator-backed), picture-backed, and null implementations live in `src/image/`, while GPU-backed subclasses live in [`../gpu/ganesh/`](../gpu/ganesh/) and [`../gpu/graphite/`](../gpu/graphite/).

## Key Classes and Entrypoints

### Image Hierarchy
* [`SkImage_Base.h`](SkImage_Base.h): Internal base class (`as_IB`) defining hooks for pixel readback (`onReadPixels`, `onPeekPixels`), subsetting, mipmap management, and async rescaling.
* [`SkImage_Raster.h`](SkImage_Raster.h): CPU-backed `SkImage` wrapping an immutable or copy-on-write `SkBitmap` (`SkPixelRef`).
* [`SkImage_Lazy.h`](SkImage_Lazy.h): Deferred-decoding `SkImage` backed by a thread-safe `SkImageGenerator` (such as an image codec), decoding pixels on demand for CPU draws or GPU texture uploads.
* [`SkImage_Picture.h`](SkImage_Picture.h) & [`SkPictureImageGenerator.h`](SkPictureImageGenerator.h): Deferred `SkImage` backed by an `SkPicture` that rasterizes on demand.

### Surface Hierarchy
* [`SkSurface_Base.h`](SkSurface_Base.h): Internal base class (`asSB`) managing the surface's cached `SkCanvas` and copy-on-write `SkImage` snapshots (`aboutToDraw`, `onNewImageSnapshot`).
* [`SkSurface_Raster.h`](SkSurface_Raster.h): CPU-backed `SkSurface` that draws via `SkBitmapDevice` and duplicates pixel memory on write if a snapshot `SkImage_Raster` is still alive.
* [`SkSurface_Null.cpp`](SkSurface_Null.cpp): Lightweight headless `SkSurface` that discards draw output without allocating pixel memory.

### Rescaling and Tiling Utilities
* [`SkRescaleAndReadPixels.h`](SkRescaleAndReadPixels.h): Shared multi-step rescaling helper for gamma-correct image scaling and readback.
* [`SkTiledImageUtils.cpp`](SkTiledImageUtils.cpp): Breaks large image draws into tiles that fit within GPU maximum texture dimension limits.
