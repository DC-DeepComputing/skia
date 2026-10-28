---
name: skia/image/ignore-gpu-recorder-in-cpu-image-transforms
description: When implementing SkImage transformation methods on CPU-backed or lazy images, do not reject non-null GPU contexts or recorders; leave the parameter unnamed and ignored
rubric: correctness
severity: should_fix
---

**Rule.** When implementing virtual transformation or surface creation methods
(such as onMakeSurface, onMakeSubset, or onMakeColorSpace) on CPU-backed SkImage
subclasses (like SkImage_Raster or SkImage_Lazy), do not reject non-null GPU
context or Recorder arguments by returning nullptr. Leave the context or
recorder parameter unnamed and ignore it so callers can pass their ambient
context or recorder polymorphically across all image types without checking
image backing.

**Why.** Callers operating with a GPU context or Graphite recorder pass their
context or recorder uniformly to high-level SkImage transformation APIs (such as
makeSubset, makeColorSpace, or makeScaled). If CPU-backed implementations reject
non-null contexts or recorders, callers are forced to inspect whether each image
is texture-backed before transforming it, breaking polymorphic image handling
and causing unexpected failures on CPU images.

## Examples

Bad:

```cpp
sk_sp<SkSurface> SkImage_Lazy::onMakeSurface(skgpu::graphite::Recorder* recorder,
                                             const SkImageInfo& info) const {
    if (recorder) {
        return nullptr; // rejects GPU recorder on CPU image
    }
    const SkSurfaceProps* props = nullptr;
    return SkSurfaces::Raster(info, 0, props);
}
```

Good:

```cpp
sk_sp<SkSurface> SkImage_Lazy::onMakeSurface(skgpu::graphite::Recorder*,
                                             const SkImageInfo& info) const {
    const SkSurfaceProps* props = nullptr;
    return SkSurfaces::Raster(info, 0, props);
}
```
