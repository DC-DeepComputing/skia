# Skia Image and Surface Implementations (`src/image/`)

## Overview

`src/image/` implements the internal base classes and CPU/deferred subclasses for `SkImage` (immutable pixel or texture sources) and `SkSurface` (drawing destinations):

* **Internal Base Classes**: `SkImage_Base` and `SkSurface_Base` define the virtual dispatch table (`as_IB(image)` and `asSB(surface)`) used across Skia to abstract over CPU and GPU backing stores.
* **CPU and Deferred Implementations**: Raster, lazy (generator-backed), picture-backed, and null implementations live in `src/image/`, while GPU-backed subclasses live in [`../gpu/ganesh/`](../gpu/ganesh/) and [`../gpu/graphite/`](../gpu/graphite/).

## Key Classes and Entrypoints

The primary classes and entrypoints in `src/image/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Image Hierarchy** | [`SkImage_Base.h`](SkImage_Base.h), [`SkImage_Raster.h`](SkImage_Raster.h), [`SkImage_Lazy.h`](SkImage_Lazy.h), [`SkImage_Picture.h`](SkImage_Picture.h), [`SkPictureImageGenerator.h`](SkPictureImageGenerator.h) | Base image dispatch (`SkImage_Base` / `as_IB`), CPU bitmap images (`SkImage_Raster`), deferred generator-backed images (`SkImage_Lazy`), and deferred picture-backed images (`SkImage_Picture`). |
| **Surface Hierarchy** | [`SkSurface_Base.h`](SkSurface_Base.h), [`SkSurface_Raster.h`](SkSurface_Raster.h), [`SkSurface_Null.cpp`](SkSurface_Null.cpp) | Base surface dispatch and copy-on-write snapshot management (`SkSurface_Base` / `asSB`), CPU raster surfaces (`SkSurface_Raster`), and headless no-op surfaces (`SkSurface_Null`). |
| **Rescaling & Tiling** | [`SkRescaleAndReadPixels.h`](SkRescaleAndReadPixels.h), [`SkTiledImageUtils.cpp`](SkTiledImageUtils.cpp) | Shared gamma-correct multi-step image rescaling/readback and tiled drawing for large images that exceed GPU texture limits. |
