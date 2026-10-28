---
name: skia/preserve-skimage-immutability-at-construction
description: When instantiating or subclassing SkImage, pass all components and metadata directly to constructors or factories instead of mutating internal fields post-instantiation
rubric: design
severity: should_fix
---

**Rule.** Do not mutate the internal structure or member fields of an SkImage
after instantiation; pass all components (such as mipmaps, pixel buffers, or
subsets) directly into constructor parameters or internal Make factory functions
so the image is fully initialized and immutable upon creation. Avoid downcasting
an SkImage to access and modify internal member variables post-construction.

**Why.** SkImage objects are architecturally designed as immutable snapshots of
pixel data so they can be safely cached and shared across threads without
synchronization. Mutating internal fields post-construction breaks this
immutability contract, inviting data races and unpredictable behavior across
concurrent consumers.

## Examples

Bad:

```cpp
// Mutating SkImage internals post-construction breaks immutability
auto img = SkImages::RasterFromPixmapCopy(pmap);
auto imgRaster = static_cast<SkImage_Raster*>(img.get());
imgRaster->fMips = std::move(mips);
return img;
```

Good:

```cpp
// Pass all state directly into constructor or internal factory
sk_sp<SkData> data(SkData::MakeWithCopy(bm.getPixels(), size));
return sk_sp<SkImage>(new SkImage_Raster(bm.info(),
                                         std::move(data),
                                         bm.info().minRowBytes(),
                                         std::move(mips),
                                         kNeedNewImageUniqueID));
```
