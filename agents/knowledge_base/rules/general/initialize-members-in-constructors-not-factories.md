---
name: skia/initialize-members-in-constructors-not-factories
description: When instantiating classes in factory methods or creation helpers, initialize member variables inside constructors rather than mutating newly allocated instances post-construction
rubric: readability
severity: should_fix
---

**Rule.** Initialize class member variables and internal sub-objects directly
inside the class constructor rather than mutating newly allocated instances
post-construction in static factory methods (such as Make() or Create()) or
allocation helpers. Pass configuration parameters and dependencies to the
constructor to ensure full encapsulation and uniform object initialization,
reserving factory functions for input validation and parameter preprocessing.

**Why.** Mutating member fields on an instance immediately after construction
breaks encapsulation, exposes mutable internal fields that could otherwise be
const, and leaves instances incompletely initialized if constructed via other
paths or subclasses. Encapsulating all initialization logic inside constructors
guarantees class invariants are established uniformly at creation time.

## Examples

Bad:

```cpp
// Mutating instance fields post-construction in Make() or Create() factory
sk_sp<Device> Device::Make(Recorder* recorder, const SkISize& deviceSize) {
    sk_sp<Device> device{new Device(recorder, std::move(dc))};
    device->fStripGenerator = std::make_unique<StripGenerator>(
            deviceSize.width(), deviceSize.height(), recorder);
    return device;
}

auto blitter = alloc->make<SkRasterPipelineBlitter>(dst, alloc);
blitter->fCanDirectBlit = can_direct_blit(paint);
blitter->fPaintColor = paint.getColor4f();
```

Good:

```cpp
// Encapsulating member initialization within the constructor
Device::Device(Recorder* recorder, sk_sp<DrawContext> dc)
        : SkDevice(dc->imageInfo(), dc->surfaceProps())
        , fRecorder(recorder)
        , fDC(std::move(dc)) {
    fStripGenerator = std::make_unique<StripGenerator>(
            this->width(), this->height(), recorder);
}

auto blitter = alloc->make<SkRasterPipelineBlitter>(dst, paint, alloc);

SkRasterPipelineBlitter::SkRasterPipelineBlitter(SkPixmap dst, const SkPaint& paint, SkArenaAlloc* alloc)
        : fDst(std::move(dst))
        , fAlloc(alloc)
        , fCanDirectBlit(can_direct_blit(paint))
        , fDirectBlitPaintColor(paint.getColor4f()) {}
```
