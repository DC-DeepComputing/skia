---
name: skia/prefer-skbitmap-erase-helpers
description: When filling or clearing an SkBitmap or SkPixmap with a uniform color or alpha, use eraseColor or eraseARGB instead of looping over getAddr pixels
rubric: readability
severity: should_fix
---

**Rule.** When filling or clearing an SkBitmap or SkPixmap with a uniform color
or alpha value, use eraseColor() or eraseARGB() (or SkPixmap::erase()) instead
of manually iterating over coordinates with getAddr*(). Pass the uniform color
or channel components directly to the erase helper.

**Why.** Skia's built-in erase methods replace verbose nested coordinate loops
with optimized fill routines. They also correctly handle color type channel
masking (such as ignoring RGB channels for kAlpha_8_SkColorType) and alpha
premultiplication.

## Examples

Bad:

```cpp
for (int y = 0; y < bitmap.height(); y++) {
    for (int x = 0; x < bitmap.width(); x++) {
        uint8_t* addr = bitmap.getAddr8(x, y);
        *addr = alpha;
    }
}
```

Good:

```cpp
bitmap.eraseARGB(alpha, 0, 0, 0);
```
