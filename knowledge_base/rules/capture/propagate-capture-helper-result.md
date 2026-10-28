---
name: skia/capture/propagate-capture-helper-result
description: When delegating work to value-returning capture helpers, return or consume the computed handle or picture rather than discarding it and returning a default
rubric: correctness
severity: should_fix
---

**Rule.** When delegating capture or resource generation to helper functions
(such as snapAndIncrement), verify that callers return or store the resulting
handle or picture object. Calling a value-returning helper as an unassigned
standalone expression discards the computed state and causes callers to
accidentally fall through to default-constructed or null returns.

**Why.** Discarding the return value of a value-generating helper drops the
produced capture or resource handle, leaving callers with a null or invalid
default value while hiding the underlying computation.

## Examples

Bad:

```cpp
sk_sp<SkPicture> SkCaptureManager::snapPicture(SkSurface* surface) {
    for (auto& canvas : fTrackedCanvases) {
        if (canvas && canvas->getBaseCanvasSurface() == surface) {
            this->snapAndIncrement(canvas.get());
        }
    }
    return nullptr;
}
```

Good:

```cpp
sk_sp<SkPicture> SkCaptureManager::snapPicture(SkSurface* surface) {
    for (auto& canvas : fTrackedCanvases) {
        if (canvas && canvas->getBaseCanvasSurface() == surface) {
            return this->snapAndIncrement(canvas.get());
        }
    }
    return nullptr;
}
```
