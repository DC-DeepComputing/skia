---
name: skia/capture/deduplicate-batch-and-single-item-operations
description: When implementing batch and single-item operations on tracked objects, factor the common per-item processing logic into a shared helper function
rubric: design
severity: should_fix
---

**Rule.** When implementing batch collection traversal and single-item lookup
methods that process tracked objects (such as capturing canvases), factor the
common per-item execution and state-tracking logic into a shared helper
function. Do not duplicate per-item mutations, counter increments, or recording
calls across iteration loops and individual lookup paths.

**Why.** Duplicating per-item execution and state-tracking sequences across
batch loops and single-item queries increases maintenance overhead and risks
diverging behavior or missed state updates when processing logic changes.

## Examples

Bad:

```cpp
void SkCaptureManager::snapPictures() {
    for (auto& canvas : fTrackedCanvases) {
        if (canvas) {
            if (auto picture = canvas->snapPicture()) {
                uint32_t surfaceID = asSB(canvas->getBaseCanvasSurface())->getPixelStorageID();
                ++fSurfaceContentCounters[surfaceID];
                fPictures.emplace_back(std::move(picture));
            }
        }
    }
}

sk_sp<SkPicture> SkCaptureManager::snapPicture(SkSurface* surface) {
    for (auto& canvas : fTrackedCanvases) {
        if (canvas && canvas->getBaseCanvasSurface() == surface) {
            if (auto picture = canvas->snapPicture()) {
                uint32_t surfaceID = asSB(canvas->getBaseCanvasSurface())->getPixelStorageID();
                ++fSurfaceContentCounters[surfaceID];
                fPictures.emplace_back(std::move(picture));
                return picture;
            }
            return nullptr;
        }
    }
    return nullptr;
}
```

Good:

```cpp
sk_sp<SkPicture> SkCaptureManager::snapAndIncrement(SkCaptureCanvas* canvas) {
    auto picture = canvas->snapPicture();
    if (picture) {
        uint32_t surfaceID = asSB(canvas->getBaseCanvasSurface())->getPixelStorageID();
        ++fSurfaceContentCounters[surfaceID];
        fPictures.emplace_back(picture);
    }
    return picture;
}

void SkCaptureManager::snapPictures() {
    for (auto& canvas : fTrackedCanvases) {
        if (canvas) {
            this->snapAndIncrement(canvas.get());
        }
    }
}

sk_sp<SkPicture> SkCaptureManager::snapPicture(SkSurface* surface) {
    for (auto& canvas : fTrackedCanvases) {
        if (canvas && canvas->getBaseCanvasSurface() == surface) {
            return this->snapAndIncrement(canvas.get());
        }
    }
    return nullptr;
}
```
