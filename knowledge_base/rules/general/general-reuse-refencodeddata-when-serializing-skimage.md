---
name: skia/general-reuse-refencodeddata-when-serializing-skimage
description: When serializing or exporting an SkImage, check image->refEncodedData() before rasterizing and re-encoding pixel data to reuse existing compressed streams
rubric: safety
severity: should_fix
---

**Rule.** When serializing, exporting, or writing an SkImage (such as in
serialization buffers, export devices, or custom image serialization callbacks),
check image->refEncodedData() before falling back to rasterizing and re-encoding
the image. Directly reuse the existing encoded data stream when available to
bypass redundant pixel extraction, decompression, and recompression.

**Why.** Re-encoding an image that already originated from an encoded data
stream (such as JPEG, PNG, or WEBP) incurs substantial CPU and memory overhead
from decompressing pixels and compressing them again. It can also cause
generational compression loss or increase serialized file size. Calling
refEncodedData() allows codec-backed and lazy images to share their original
compressed bytes directly with zero re-encoding overhead.

## Examples

Bad:

```cpp
static sk_sp<const SkData> serialize_image(const SkImage* image, SkSerialProcs procs) {
    if (procs.fImageProc) {
        if (auto data = procs.fImageProc(const_cast<SkImage*>(image), procs.fImageCtx)) {
            return data;
        }
    }
    SkBitmap bm;
    if (!as_IB(image)->getROPixels(nullptr, &bm)) {
        return nullptr;
    }
    return SkPngEncoder::Encode(nullptr, bm.pixmap(), {});
}
```

Good:

```cpp
static sk_sp<const SkData> serialize_image(const SkImage* image, SkSerialProcs procs) {
    if (procs.fImageProc) {
        if (auto data = procs.fImageProc(const_cast<SkImage*>(image), procs.fImageCtx)) {
            return data;
        }
    }
    // Check if the image already originated from an encoded data stream.
    if (auto data = image->refEncodedData()) {
        return data;
    }
    SkBitmap bm;
    if (!as_IB(image)->getROPixels(nullptr, &bm)) {
        return nullptr;
    }
    return SkPngEncoder::Encode(nullptr, bm.pixmap(), {});
}
```
