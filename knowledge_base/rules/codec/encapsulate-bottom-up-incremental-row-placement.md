---
name: skia/codec/encapsulate-bottom-up-incremental-row-placement
description: When implementing incremental decoding for inverted row orientations, encapsulate scanline placement and track progress by accumulating decoded rows rather than row offsets
rubric: correctness
severity: should_fix
---

**Rule.** When implementing incremental or progressive decoding for images with
inverted row orientations (such as bottom-up BMPs), calculate source buffer
slice ranges and destination row placement based on scanline orientation rather
than assuming top-down ordering from offset 0. Encapsulate row placement in the
reader or decode state (e.g., returning dst_row_start), and avoid manually
inverting destination row coordinates against sequential source slices.
Additionally, report incremental progress (*rowsDecoded) by accumulating total
rows decoded in the decode state rather than deriving progress from dstRowStart
+ rowCount.

**Why.** In bottom-up formats, decoded rows are written towards the end of the
buffer from bottom to top. Assuming top-down ordering slices uninitialized
memory or inverts partially decoded scanlines, and deriving progress from
destination offsets (dstRowStart + rowCount) produces out-of-bounds or
inaccurate decoded row counts.

## Examples

Bad:

```cpp
// Rust reader: assuming top-down slices uninitialized memory for bottom-up images
let valid_bytes = (self.rows_decoded() as usize) * row_bytes;
*buffer = &self.image_data[..valid_bytes];

// C++ caller: inverting destination rows and deriving progress from destination offsets
for (int32_t y = state.fRowsAlreadyCopied; y < rowsAvailable; ++y) {
    int32_t dstY = fIsBottomUp ? (height - 1 - y) : y;
    this->swizzleRow(srcImage.subspan(y * srcRowBytes, srcRowBytes).data(),
                     dst.fDst.data() + (dstY * dst.fDstRowStride));
}
if (rowsDecodedPtr) {
    *rowsDecodedPtr = static_cast<int>(dstRowStart + rowCount);
}
```

Good:

```cpp
// Rust reader: calculate slice range and destination start based on orientation
let (buf_start, dst_row_start) = if is_top_down {
    ((already_consumed as usize) * row_bytes, already_consumed)
} else {
    ((height as usize - current_rows as usize) * row_bytes, height - current_rows)
};
*buffer = &self.image_data[buf_start..buf_start + new_bytes];

// C++ caller: place rows using encapsulated dst_row_start and accumulate decoded count
rust_bmp::DecodedRowsInfo rowsInfo = fReader->get_next_rows(imageData);
for (uint32_t i = 0; i < rowsInfo.row_count; ++i) {
    uint32_t dstY = rowsInfo.dst_row_start + i;
    this->swizzleRow(srcImage.subspan(i * srcRowBytes, srcRowBytes).data(),
                     state.fDst.data() + (dstY * state.fDstRowStride));
}
state.fTotalRowsDecoded += rowsInfo.row_count;
if (rowsDecodedPtr) {
    *rowsDecodedPtr = state.fTotalRowsDecoded;
}
```
