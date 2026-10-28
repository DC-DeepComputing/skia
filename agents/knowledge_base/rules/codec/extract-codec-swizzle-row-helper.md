---
name: skia/codec/extract-codec-swizzle-row-helper
description: When decoding image rows across full and scanline decode paths in image codecs, extract a swizzleRow helper encapsulating conditional xformOnDecode swizzling and color transformation
rubric: design
severity: should_fix
---

**Rule.** In image decoders performing row decoding and color conversion,
extract a swizzleRow(const uint8_t* srcRow, void* dstRow) helper method to
encapsulate conditional xformOnDecode() swizzling and color transformation. Call
this shared helper across full, scanline, and incremental decode loops rather
than duplicating fSwizzler->swizzle and applyColorXform branches across decode
paths.

**Why.** Row decoding logic combining SkSwizzler::swizzle and applyColorXform
based on xformOnDecode() is shared across full, scanline, and incremental decode
loops. Inlining the conditional branching across multiple decode paths
introduces maintenance overhead and risks behavioral discrepancies, such as
omitting fSwizzler->swizzleWidth() or mismanaging intermediate transform
buffers.

## Examples

Bad:

```cpp
// Duplicated in both performFullDecode and onGetScanlines:
if (this->xformOnDecode()) {
    fSwizzler->swizzle(fXformBuffer.get(), srcRow.data());
    this->applyColorXform(dstPtr, fXformBuffer.get(), width);
} else {
    fSwizzler->swizzle(dstPtr, srcRow.data());
}
```

Good:

```cpp
void SkBmpRustCodec::swizzleRow(const uint8_t* srcRow, void* dstRow) {
    if (this->xformOnDecode()) {
        const int32_t width = fSwizzler->swizzleWidth();
        fSwizzler->swizzle(fXformBuffer.get(), srcRow);
        this->applyColorXform(dstRow, fXformBuffer.get(), width);
    } else {
        fSwizzler->swizzle(dstRow, srcRow);
    }
}
// Both onGetPixels and onGetScanlines call this->swizzleRow(...)
```
