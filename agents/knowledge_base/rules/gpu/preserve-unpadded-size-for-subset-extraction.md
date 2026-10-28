---
name: skia/gpu/preserve-unpadded-size-for-subset-extraction
description: When allocating padded backing storage and extracting an inner subset, compute padded dimensions into a separate variable rather than mutating the unpadded size parameter in place
rubric: correctness
severity: blocking
---

**Rule.** When allocating padded backing storage (such as an SkBitmap or
SkPixmap) and extracting an inner subset, compute padded dimensions into a
distinct SkISize variable. Do not mutate the input size parameter in place to
add padding, which corrupts subsequent subset rectangle calculations like
SkIRect::MakePtSize({padding, padding}, size).

**Why.** Mutating the input size in place causes subsequent subset extraction to
compute an oversized inner rectangle from the already-expanded dimensions. This
leads to improper clipping, out-of-bounds subset extraction failures, or
uninitialized border padding in the resulting pixmap.

## Examples

Bad:

```cpp
// Mutating size in place expands it twice when calculating the subset rect
size.fWidth += 2 * padding;
size.fHeight += 2 * padding;
SkBitmap bitmap;
bitmap.allocPixels(SkImageInfo::MakeA8(size));

const SkPixmap outerPM = bitmap.pixmap();
SkPixmap innerPM;
outerPM.extractSubset(&innerPM, SkIRect::MakePtSize({padding, padding}, size));
```

Good:

```cpp
// Preserve unpadded size and compute padded dimensions into a separate variable
SkISize paddedSize{size.width() + 2 * padding, size.height() + 2 * padding};
SkBitmap bitmap;
bitmap.allocPixels(SkImageInfo::MakeA8(paddedSize));

const SkPixmap outerPM = bitmap.pixmap();
SkPixmap innerPM;
outerPM.extractSubset(&innerPM, SkIRect::MakePtSize({padding, padding}, size));
```
