---
name: skia/pdf/match-color-space-with-channel-count
description: When embedding image or JPEG streams in PDF, do not assume RGB; inspect encoded metadata to pair DeviceRGB with 3 channels and DeviceGray with 1
rubric: correctness
severity: should_fix
---

**Rule.** When embedding image streams (such as JPEG DCT streams) in PDF, do not
assume encoded data is RGB or hardcode channel counts. Inspect the encoded
stream info (e.g. via SkEncodedInfo) to determine the color format, and ensure
the declared PDF color space strictly matches the component channel count: use
DeviceRGB for 3-channel data (such as YUV/YCbCr) and DeviceGray for 1-channel
data. Never configure DeviceRGB with 1 channel.

**Why.** Standard JPEG encoders compress color images as YUV/YCbCr or grayscale,
never raw RGB. PDF image dictionaries and embedded ICC profiles require
component channel counts to match the declared color space (/N entry);
specifying DeviceRGB with 1 channel creates malformed PDF structures and causes
ICC profile channel mismatch failures in PDF consumers.

## Examples

Bad:

```cpp
// Bad: assuming JPEG encodes as RGB and hardcoding mismatched channel counts
if (SkJpegEncoder::Encode(&stream, pm, jOpts)) {
    write_data_to_stream_as_jpeg(stream.detachAsData(),
                                 pm.colorSpace(),
                                 doc,
                                 ref,
                                 pm.dimensions(),
                                 SkPDFUnion::Name("DeviceRGB"),
                                 /*channels=*/1);
}
```

Good:

```cpp
// Good: inspect encoded stream info and match color space with channel count
if (SkJpegEncoder::Encode(&stream, pm, jOpts)) {
    auto data = stream.detachAsData();
    bool yuv = (jpegColorType == SkEncodedInfo::kYUV_Color);
    int channels = yuv ? 3 : 1;
    SkPDFUnion colorSpace = yuv ? SkPDFUnion::Name("DeviceRGB")
                                : SkPDFUnion::Name("DeviceGray");
    write_data_to_stream_as_jpeg(std::move(data), pm.colorSpace(), doc, ref,
                                 pm.dimensions(), std::move(colorSpace), channels);
}
```
