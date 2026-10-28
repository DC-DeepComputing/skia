---
name: skia/document-temporary-surface-image-returns
description: When helper functions return an SkImage created via SkSurface::makeTemporaryImage(), document in header declarations that the returned image is a temporary surface image
rubric: readability
severity: nit
---

**Rule.** When declaring or implementing helper functions that return an SkImage
produced via SkSurface::makeTemporaryImage() rather than an immutable snapshot,
document in the header declaration comment that the returned image is a
temporary image created from the surface. State clearly that the returned image
is only valid while the underlying surface is not mutated.

**Why.** SkSurface::makeTemporaryImage() avoids copy-on-write allocation
overhead, but the resulting SkImage directly references the surface's backing
store and its contents become undefined if the surface is mutated. Documenting
this contract at the helper's declaration ensures callers know not to modify or
write to the source surface while referencing the returned temporary image.

## Examples

Bad:

```cpp
// In SkPictureShader.h
struct CachedImageInfo {
    ...
    sk_sp<SkImage> makeImage(sk_sp<SkSurface> surf, const SkPicture* pict) const;
};

// In SkPictureShader.cpp
sk_sp<SkImage> CachedImageInfo::makeImage(sk_sp<SkSurface> surf, const SkPicture* pict) const {
    ...
    return surf->makeTemporaryImage();
}
```

Good:

```cpp
// In SkPictureShader.h
struct CachedImageInfo {
    ...
    // Returns a temporary image created from the surface; callers must not mutate the surface.
    sk_sp<SkImage> makeImage(sk_sp<SkSurface> surf, const SkPicture* pict) const;
};

// In SkPictureShader.cpp
sk_sp<SkImage> CachedImageInfo::makeImage(sk_sp<SkSurface> surf, const SkPicture* pict) const {
    ...
    return surf->makeTemporaryImage();
}
```
