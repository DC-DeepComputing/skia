---
name: skia/prefer-concrete-member-over-base-downcast
description: When implementing operations on concrete derived classes, define instance member functions rather than free or static helpers that accept and downcast abstract base pointers
rubric: design
severity: should_fix
---

**Rule.** When implementing operations on concrete derived classes (such as
Graphite's Surface/Device or SkImage_Raster), implement them as instance member
functions on the concrete class rather than free functions or static class
helpers that accept abstract base pointers (such as SkSurface* or SkImage*),
assert the subtype, and downcast. Callers holding or verifying the concrete
derived type should invoke instance member functions directly rather than
relying on downcasting wrappers.

**Why.** Free or static helpers that take abstract base pointers only to assert
the concrete subtype and downcast add unnecessary wrapper indirection,
boilerplate assertions, and friend declarations. Exposing operations as instance
member functions directly on the concrete derived class cleanly encapsulates
subsystem logic, provides compile-time type safety, and simplifies call sites.

## Examples

Bad:

```cpp
// Free helper taking base pointer and downcasting:
void FlushToDrawContext(SkSurface* surface, DrawContext* drawContext) {
    if (!surface || !asSB(surface)->isGraphiteBacked()) return;
    static_cast<Surface*>(surface)->fDevice->flushToDrawContext(drawContext);
}

// Static helper on derived class taking base pointer:
const SkPixelRef* SkImage_Raster::GetPixelRef(const SkImage* image) {
    SkASSERT(as_IB(image)->type() == SkImage_Base::Type::kRaster);
    return static_cast<const SkImage_Raster*>(image)->getPixelRef();
}
```

Good:

```cpp
// Instance member function on concrete derived class:
void Surface::flushToDrawContext(DrawContext* drawContext) {
    fDevice->flushPendingWork(drawContext);
}
surface->flushToDrawContext(drawContext);

// Instance member function invoked directly on concrete derived instance:
SkPixelRef* SkImage_Raster::getPixelRef() const { return fBitmap.pixelRef(); }
if (static_cast<const SkImage_Raster*>(image)->getPixelRef() == fBitmap.pixelRef()) {
```
