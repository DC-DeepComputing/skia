---
name: skia/tools/overload-helpers-for-derived-context-types
description: When tool helpers accept base context pointers like GrRecordingContext*, provide explicit overloads for derived types like GrDirectContext* to prevent build failures when backends are disabled
rubric: correctness
severity: blocking
---

**Rule.** When defining shared tool or test utility functions that accept base
GPU context pointers (such as GrRecordingContext*), provide explicit overloads
for derived context types (such as GrDirectContext*). Encapsulate the
conditional downcast or null fallback within the implementation file guarded by
preprocessor macros (such as #if defined(SK_GANESH)), rather than relying on
implicit pointer conversions at call sites.

**Why.** When a GPU backend subsystem is disabled in certain build
configurations (such as building Graphite-only with skia_enable_ganesh = false),
its context classes are only forward-declared incomplete types. C++ prohibits
implicit pointer conversions between incomplete base and derived classes,
causing compilation errors at call sites passing derived pointers. Providing
explicit overloads centralizes backend preprocessor handling and allows caller
sites to compile unconditionally.

## Examples

Bad:

```cpp
// tools/gpu/YUVUtils.h
// Only accepts base context pointer:
sk_sp<SkImage> refImage(GrRecordingContext* rContext, Type type);

// gm/wacky_yuv_formats.cpp
// Fails to compile when skia_enable_ganesh = false because GrDirectContext
// and GrRecordingContext are incomplete types and cannot implicitly convert:
fImages[opaque][cs][format] = lazyYUV->refImage(dContext, fImageType);
```

Good:

```cpp
// tools/gpu/YUVUtils.h
sk_sp<SkImage> refImage(GrRecordingContext* rContext, Type type);
sk_sp<SkImage> refImage(GrDirectContext* dContext, Type type);

// tools/gpu/YUVUtils.cpp
sk_sp<SkImage> LazyYUVImage::refImage(GrDirectContext* dContext, Type type) {
#if defined(SK_GANESH)
    return this->refImage(static_cast<GrRecordingContext*>(dContext), type);
#else
    SkASSERT(!dContext);
    return this->refImage(static_cast<GrRecordingContext*>(nullptr), type);
#endif
}
```
