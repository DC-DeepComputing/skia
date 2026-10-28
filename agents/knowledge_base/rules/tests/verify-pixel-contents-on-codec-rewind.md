---
name: skia/tests/verify-pixel-contents-on-codec-rewind
description: When testing image decoders or codec operations, verify decoded pixel contents via spot checks or ComparePixels rather than only checking success, non-null buffers, or dimensions
rubric: testing
severity: should_fix
---

**Rule.** When testing image decoders or codec operations (such as getPixels,
getImage, or incremental decoding), do not rely solely on return status,
non-null buffer pointers (e.g., pixmap.addr() != nullptr), or image dimension
assertions (dimensions() == dimensions()); verify that decoding produced correct
pixel content by spot-checking specific pixel colors (e.g., via
assert_pixel_color or pixmap.getColor) or comparing against expected reference
data. When testing codec reuse, rewind, or decode equivalence, verify that
decoded pixel contents match using ComparePixels (from tests/ComparePixels.h)
with zero tolerance across all color channels.

**Why.** A decoder can return success and report expected dimensions while
producing all-zero, blank, inverted, or corrupted pixel data due to flawed
scanline indexing, row strides, or corrupted stream resets. Verifying actual
pixel values—via spot checks, getColor(), or ComparePixels—ensures byte-for-byte
visual correctness and catches incorrect channel ordering or pixel generation.

## Examples

Bad:

```cpp
// Bad: only asserting success status, dimensions, or non-null buffer pointers
DEF_TEST(RustBmpCodec_getPixels, r) {
    std::unique_ptr<SkCodec> codec = decode_bmp(r, "images/randPixels.bmp");
    SkBitmap bitmap;
    bitmap.allocPixels(codec->getInfo().makeColorType(kN32_SkColorType));
    REPORTER_ASSERT(r, codec->getPixels(bitmap.pixmap()) == SkCodec::kSuccess);
    REPORTER_ASSERT(r, bitmap.dimensions() == SkISize::Make(8, 8));
    REPORTER_ASSERT(r, bitmap.pixmap().addr() != nullptr, "Pixmap should have valid pixel data");
}

// Bad: testing reuse/rewind without comparing pixel contents
auto [image2, result2] = codec->getImage();
REPORTER_ASSERT(r, result2 == SkCodec::kSuccess);
REPORTER_ASSERT(r, image1->dimensions() == image2->dimensions());
```

Good:

```cpp
// Good: spot-checking pixel colors on decode
DEF_TEST(RustBmpCodec_getPixels, r) {
    std::unique_ptr<SkCodec> codec = decode_bmp(r, "images/randPixels.bmp");
    SkBitmap bitmap;
    bitmap.allocPixels(codec->getInfo().makeColorType(kN32_SkColorType));
    REPORTER_ASSERT(r, codec->getPixels(bitmap.pixmap()) == SkCodec::kSuccess);
    REPORTER_ASSERT(r, bitmap.dimensions() == SkISize::Make(8, 8));

    SkPixmap pixmap = bitmap.pixmap();
    assert_pixel_color(r, pixmap, 0, 0, 0xFFBBA570, "pixel (0,0)");
    assert_pixel_color(r, pixmap, 1, 0, 0xFF395F5D, "pixel (1,0)");
}

// Good: verifying reuse/rewind equivalence with ComparePixels
auto [image2, result2] = codec->getImage();
REPORTER_ASSERT(r, result2 == SkCodec::kSuccess);
const float tols[4] = {0, 0, 0, 0};
auto error = std::function<ComparePixmapsErrorReporter>([&](int x, int y, const float diffs[4]) {
    ERRORF(r, "Pixels differ at (%d, %d) after rewind.", x, y);
});
ComparePixels(bm1.pixmap(), bm2.pixmap(), tols, error);
```
