---
name: skia/codec/derive-intermediate-decode-buffer-metrics
description: When decoding or expanding image rows into intermediate buffers before deferred transformations, derive pixel stride and bytes-per-pixel from the intermediate format rather than dstInfo()
rubric: correctness
severity: should_fix
---

**Rule.** When decoding or expanding pixel rows into an intermediate buffer
prior to deferred transformations, color conversion, or subset extraction,
derive row stride and bytes-per-pixel from the intermediate buffer's format
(such as SkEncodedInfo or encoded row bytes) rather than final dstInfo(). Ensure
low-level decoding and row expansion routines receive the intermediate buffer's
actual format metrics.

**Why.** Intermediate buffers assembled before deferred transformations or
subsetting contain pixels in the encoded or source format. Querying destination
metrics such as dstInfo().bytesPerPixel() or destination row stride calculates
wrong byte offsets and strides during decoding, causing out-of-bounds writes or
corrupting intermediate image data.

## Examples

Bad:

```cpp
// Intermediate buffer holds raw encoded pixels before applyColorXform,
// but dstBytesPerPixel is queried from final dstInfo().
uint8_t dstBytesPerPixel = safe.castTo<uint8_t>(this->dstInfo().bytesPerPixel());
fReader->expand_last_interlaced_row(rust::Slice<uint8_t>(dstFrame),
                                    dstRowStride,
                                    rust::Slice<const uint8_t>(xformedInterlacedRow),
                                    dstBytesPerPixel * 8u);
```

Good:

```cpp
// Pass intermediate buffer's pixel metrics (encoded format) when transforms are deferred:
DecodingState intermediateDecodingState = {decodingState.fPreblendBuffer,
                                           this->getEncodedRowBytes(),
                                           this->getEncodedRowBytes(),
                                           {},
                                           encodedBytesPerPixel};
this->expandDecodedInterlacedRow(decodingState.fPreblendBuffer,
                                 decodedRow,
                                 intermediateDecodingState,
                                 /*xFormNeeded=*/false);
```
