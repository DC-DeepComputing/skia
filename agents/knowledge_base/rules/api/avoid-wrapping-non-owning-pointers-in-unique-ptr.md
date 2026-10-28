---
name: skia/api/avoid-wrapping-non-owning-pointers-in-unique-ptr
description: When caching or wrapping heap allocations, adopt raw pointers into owning smart pointers immediately upon acquisition and keep borrowed wrapper references as non-owning raw pointers
rubric: safety
severity: blocking
---

**Rule.** When caching or wrapping heap-allocated objects, separate owned
storage from borrowed references and establish RAII immediately at creation.
Transfer raw pointers returned from heap-allocating factory or virtual creation
methods into dedicated owning smart pointers (such as std::unique_ptr)
immediately upon acquisition rather than holding them in temporary raw pointers
across intermediate operations. Never construct a std::unique_ptr from an
externally managed or borrowed raw pointer, and retain wrapper or cached
references as non-owning raw pointers.

**Why.** Holding an owning raw pointer across intermediate calls creates leak
risks if intermediate logic fails or early-exits. Conversely, constructing a
std::unique_ptr from a borrowed raw pointer creates duplicate ownership that
triggers double-free crashes upon destruction, whereas omitting owning storage
for allocated objects causes memory leaks.

## Examples

Bad:

```cpp
// Bad: holding raw allocation across branching, or wrapping borrowed pointer
auto baseCanvas = this->onNewCanvas();
if (auto captureCanvas = this->baseRecorder()->makeCaptureCanvas(baseCanvas)) {
    fCachedCanvas = std::unique_ptr<SkCanvas>(captureCanvas); // Double-free: borrowed pointer
} else {
    fCachedCanvas = baseCanvas;
}
fOwnedBaseCanvas = std::unique_ptr<SkCanvas>(baseCanvas); // Delayed RAII transfer
```

Good:

```cpp
// Good: immediate RAII adoption into smart pointer, non-owning raw pointer for cache
fOwnedBaseCanvas = std::unique_ptr<SkCanvas>(this->onNewCanvas());
if (this->baseRecorder()) {
    fCachedCanvas = this->baseRecorder()->makeCaptureCanvas(fOwnedBaseCanvas.get());
}
if (!fCachedCanvas) {
    fCachedCanvas = fOwnedBaseCanvas.get();
}
return fCachedCanvas;
```
