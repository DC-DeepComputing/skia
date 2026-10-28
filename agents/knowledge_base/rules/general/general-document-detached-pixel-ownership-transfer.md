---
name: skia/general-document-detached-pixel-ownership-transfer
description: When transferring pixel buffer ownership from a scoped container into SkBitmap via installPixels with a release callback, document the transfer semantics
rubric: readability
severity: nit
---

**Rule.** When transferring pixel buffer ownership from a scoped container (such
as via SkAutoPixmapStorage::detachPixels()) into an SkBitmap using
installPixels() with a custom release callback (e.g. sk_free), add an
explanatory comment documenting the ownership transfer. Clarify that the bitmap
takes ownership of the detached storage and that the release callback frees the
memory upon pixel ref destruction.

**Why.** By default, SkBitmap::installPixels() only borrows external memory
without taking ownership, risking use-after-free if the source storage is
destroyed before the bitmap. Explicitly detaching storage and supplying a custom
release callback inverses this default behavior; documenting the handoff
clarifies lifetime and destruction semantics for maintainers.

## Examples

Bad:

```cpp
SkAutoPixmapStorage dst;
draw_clip_mask_to_pixmap(elements, bounds, &dst);

SkBitmap bm;
SkAssertResult(bm.installPixels(dst.info(), dst.detachPixels(), dst.rowBytes(),
                                [](void* addr, void* context) { sk_free(addr); },
                                nullptr));
bm.setImmutable();
return bm;
```

Good:

```cpp
SkAutoPixmapStorage dst;
draw_clip_mask_to_pixmap(elements, bounds, &dst);

SkBitmap bm;
// Bitmap takes ownership of pixel buffer: detach from SkAutoPixmapStorage
// so it does not free upon destruction, and provide sk_free release callback.
SkAssertResult(bm.installPixels(dst.info(), dst.detachPixels(), dst.rowBytes(),
                                [](void* addr, void* context) { sk_free(addr); },
                                nullptr));
bm.setImmutable();
return bm;
```
