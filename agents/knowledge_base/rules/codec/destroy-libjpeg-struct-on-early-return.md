---
name: skia/codec/destroy-libjpeg-struct-on-early-return
description: When managing manual libjpeg decompressor or compressor structs, ensure jpeg_destroy_decompress or jpeg_destroy_compress is invoked on all early return and error paths
rubric: safety
severity: should_fix
---

**Rule.** When initializing a `jpeg_decompress_struct` or `jpeg_compress_struct`
without an RAII wrapper (such as `SkJpegDecoderMgr` or `SkJpegEncoderMgr`),
ensure `jpeg_destroy_decompress` or `jpeg_destroy_compress` is called on all
control-flow paths, including early returns and error branches. Alternatively,
wrap the struct in an RAII guard to guarantee destruction on scope exit.

**Why.** `jpeg_create_decompress` and `jpeg_create_compress` allocate internal
memory structures within libjpeg's memory manager. Exiting early without
invoking the matching destroy routine leaks those internal resources.

## Examples

Bad:

```cpp
jpeg_decompress_struct dinfo;
jpeg_create_decompress(&dinfo);
...
while (dinfo.output_width != dstWidth || dinfo.output_height != dstHeight) {
    if (1 == num || dstWidth > dinfo.output_width || dstHeight > dinfo.output_height) {
        return false; // Leaks internal libjpeg memory allocations
    }
    ...
}
jpeg_destroy_decompress(&dinfo);
```

Good:

```cpp
jpeg_decompress_struct dinfo;
jpeg_create_decompress(&dinfo);
...
while (dinfo.output_width != dstWidth || dinfo.output_height != dstHeight) {
    if (1 == num || dstWidth > dinfo.output_width || dstHeight > dinfo.output_height) {
        jpeg_destroy_decompress(&dinfo);
        return false;
    }
    ...
}
jpeg_destroy_decompress(&dinfo);
```
