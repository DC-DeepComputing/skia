---
name: skia/codec/balance-jpeg-start-finish-output
description: When using libjpeg buffered-image mode for progressive decoding, balance every jpeg_start_output call with jpeg_finish_output on all early return and error paths
rubric: correctness
severity: should_fix
---

**Rule.** When decoding progressive JPEGs in libjpeg buffered-image mode, ensure
every call to jpeg_start_output is balanced by a call to jpeg_finish_output on
all early exit, break, and error return paths. Do not return or exit the decode
loop without completing the output pass, ensuring the decompressor state remains
consistent.

**Why.** Calling jpeg_start_output transitions libjpeg's decompression engine
into an active output pass. Returning early without calling jpeg_finish_output
leaves the decompressor struct in an incomplete pass state, corrupting
subsequent decode passes or decoder teardown.

## Examples

Bad:

```cpp
// Returning early between jpeg_start_output and jpeg_finish_output leaves decompressor in partial pass state
jpeg_start_output(dinfo, scan_number);
SkCodec::Result result = this->readRows(dstInfo, dst, dstRowBytes, count, options, &rows);
if (result != kSuccess) {
    return fDecoderMgr->returnFailure("readRows", result);
}
jpeg_finish_output(dinfo);
```

Good:

```cpp
// Balance jpeg_start_output with jpeg_finish_output before checking results or returning early
jpeg_start_output(dinfo, scan_number);
SkCodec::Result result = this->readRows(dstInfo, dst, dstRowBytes, count, options, &rows);
jpeg_finish_output(dinfo);
if (result != kSuccess) {
    return fDecoderMgr->returnFailure("readRows", result);
}
```
