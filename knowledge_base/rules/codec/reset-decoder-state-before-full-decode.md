---
name: skia/codec/reset-decoder-state-before-full-decode
description: When implementing image codec onGetPixels full decodes, reset reader state and stream cursors to row 0 before decoding to ensure kIncompleteInput retries decode correctly
rubric: correctness
severity: should_fix
---

**Rule.** When implementing full image decoding (`onGetPixels`), explicitly
reset the underlying reader, decode cursor, or stream position back to the
beginning of the image or frame before starting decode operations. Ensure that
if a prior call aborted midway and returned `kIncompleteInput`, retrying
`onGetPixels` with additional streamed data restarts decoding from row 0 rather
than appending subsequent rows into the start of the destination buffer.

**Why.** `onGetPixels` always writes into the destination pixel buffer starting
at row 0. If partial input causes an initial decode attempt to return
`kIncompleteInput` and the decoder does not reset its internal cursor or reader
state, a subsequent call after receiving more stream bytes will resume reading
at an intermediate row while writing into destination row 0, corrupting the
image.

## Examples

Bad:

```cpp
SkCodec::Result SkBmpRustCodec::onGetPixels(const SkImageInfo& info, void* dst,
                                            size_t dstRowBytes, const Options& opts,
                                            int* rowsDecoded) {
    // BUG: Missing reset_decode_state() leaves reader at previous row if retried
    rust_bmp::DecodingResult result = fReader->read_image_data();
    if (result == rust_bmp::DecodingResult::IncompleteInput) {
        return kIncompleteInput;
    }
    return fReader->get_next_rows(dst) ? kSuccess : kErrorInInput;
}
```

Good:

```cpp
SkCodec::Result SkBmpRustCodec::onGetPixels(const SkImageInfo& info, void* dst,
                                            size_t dstRowBytes, const Options& opts,
                                            int* rowsDecoded) {
    // Reset reader to row 0 so retries after kIncompleteInput write from the start
    fReader->reset_decode_state();
    rust_bmp::DecodingResult result = fReader->read_image_data();
    if (result == rust_bmp::DecodingResult::IncompleteInput) {
        return kIncompleteInput;
    }
    return fReader->get_next_rows(dst) ? kSuccess : kErrorInInput;
}
```
