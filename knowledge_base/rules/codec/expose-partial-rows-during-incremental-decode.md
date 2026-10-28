---
name: skia/codec/expose-partial-rows-during-incremental-decode
description: When implementing SkCodec::onIncrementalDecode, copy partially decoded rows to the destination buffer and set rowsDecodedPtr before returning kIncompleteInput
rubric: correctness
severity: should_fix
---

**Rule.** When implementing `SkCodec::onIncrementalDecode` (or its internal
decoding helpers), copy or swizzle all newly decoded rows into the destination
buffer and update `*rowsDecodedPtr` with the cumulative decoded row count before
returning `kIncompleteInput`. Do not return `kIncompleteInput` with
`*rowsDecodedPtr = 0` or without transferring available scanlines to the
destination buffer, which prevents clients from progressively rendering partial
stream data.

**Why.** `onIncrementalDecode` enables progressive rendering while image streams
arrive in chunks. If an implementation returns `kIncompleteInput` without
populating the destination buffer and reporting the number of initialized rows
via `rowsDecodedPtr`, callers cannot display partially received images and may
fail to initialize remaining unrendered rows.

## Examples

Bad:

```cpp
if (readResult == rust_bmp::DecodingResult::IncompleteInput) {
    // Bad: Returns kIncompleteInput without copying decoded rows or reporting progress
    if (rowsDecodedPtr) {
        *rowsDecodedPtr = 0;
    }
    return kIncompleteInput;
}
```

Good:

```cpp
if (readResult == rust_bmp::DecodingResult::IncompleteInput) {
    const int32_t rowsAvailable = static_cast<int32_t>(fReader->rows_decoded());
    // Copy newly available decoded rows to the destination buffer
    for (int32_t y = state.fTotalRowsDecoded; y < rowsAvailable; ++y) {
        void* dstRow = dst.fDst.data() + (y * dst.fDstRowStride);
        this->swizzleRow(srcImage.subspan(y * srcRowBytes, srcRowBytes).data(), dstRow);
    }
    state.fTotalRowsDecoded = rowsAvailable;
    if (rowsDecodedPtr) {
        *rowsDecodedPtr = state.fTotalRowsDecoded;
    }
    return kIncompleteInput;
}
```
