---
name: skia/api/inherit-backend-recorders-from-skrecorder
description: When introducing backend recorders or surface creation, inherit from SkRecorder, use Recorder member methods instead of SkSurfaces factories, and name them makeBitmapSurface
rubric: readability
severity: should_fix
---

**Rule.** When defining backend-specific recording contexts (such as CPU,
Ganesh, or Graphite recorders), inherit from the common abstract SkRecorder base
class so backend-agnostic APIs accept SkRecorder polymorphically. Expose
recorder-backed surface creation as member methods on the concrete Recorder
class rather than standalone factory functions in namespace SkSurfaces.
Additionally, name these backend-specific surface factories explicitly after the
underlying storage (e.g., makeBitmapSurface or makeTextureSurface); reserve
generic makeSurface() for compatible surface allocation on polymorphic base
classes (like SkCanvas and SkSurface), avoiding abbreviations like
makeCPUSurface.

**Why.** Deriving backend-specific recorders from SkRecorder decouples image
transformations and fallback rendering from concrete backend types, while
placing surface creation on the Recorder class binds surface lifetime to that
instance. Reserving generic makeSurface() for compatible surface allocation
prevents semantic collision with SkCanvas/SkSurface conventions, and explicit
names like makeBitmapSurface prevent visual confusion from abbreviations like
CPU and GPU.

## Examples

Bad:

```cpp
// Bad: recorder not derived from SkRecorder, factory in SkSurfaces, or generic/confusing name
namespace SkSurfaces {
SK_API sk_sp<SkSurface> Raster(SkRasterRecorder* recorder, const SkImageInfo& info,
                               size_t rowBytes, const SkSurfaceProps* props);
}

class SK_API SkCPURecorder : public SkRecorder {
public:
    // Bad: makeSurface is for compatible surfaces; makeCPUSurface abbreviation is confusing
    sk_sp<SkSurface> makeSurface(const SkImageInfo& info, size_t rowBytes, const SkSurfaceProps* props);
};
```

Good:

```cpp
// Good: inherits from SkRecorder, exposes surface creation as member method with explicit name
class SK_API Recorder : public SkRecorder {
public:
    SkRecorder::Type type() const final { return SkRecorder::Type::kCPU; }

    // Good: explicit storage name makeBitmapSurface; makeSurface reserved for compatible surfaces
    sk_sp<SkSurface> makeBitmapSurface(const SkImageInfo& imageInfo,
                                       size_t rowBytes,
                                       const SkSurfaceProps* surfaceProps);
};
```
