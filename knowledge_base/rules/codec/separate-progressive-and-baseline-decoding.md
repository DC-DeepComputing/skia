---
name: skia/codec/separate-progressive-and-baseline-decoding
description: When implementing image decoders supporting progressive and baseline streams, separate progressive and baseline control flow into distinct branches instead of multiplexing a shared loop
rubric: design
severity: should_fix
---

**Rule.** When implementing image decoders (such as SkJpegCodec or SkPngCodec)
that support both multi-pass progressive and single-pass baseline formats,
separate baseline and progressive decoding into dedicated control flow branches
or helper methods. Do not multiplex both modes within a shared loop using inner
conditional checks (such as if (fProgressive) or breaking early for
non-progressive streams).

**Why.** Progressive and baseline decoding differ fundamentally in loop
termination, pass tracking, and error-handling lifecycles (such as libjpeg's
start_output and finish_output). Multiplexing both modes inside a single loop
introduces complex conditional branching and risks inadvertently altering
behavior, degrading performance, or skipping decode steps on the baseline path.

## Examples

Bad:

```cpp
// Multiplexing progressive and baseline decoding in a single loop
while (!jpeg_input_complete(dinfo)) {
    if (fProgressive) {
        jpeg_start_output(dinfo, dinfo->input_scan_number);
    }
    this->readRows(dstInfo, dst, dstRowBytes, count, options, &rows);
    if (!fProgressive || !jpeg_finish_output(dinfo)) {
        break;
    }
    output_passes++;
}
```

Good:

```cpp
// Dedicated branches for progressive and baseline decode paths
if (fProgressive) {
    int output_passes = 0;
    while (!jpeg_input_complete(dinfo)) {
        jpeg_start_output(dinfo, dinfo->input_scan_number);
        this->readRows(dstInfo, dst, dstRowBytes, count, options, &rows);
        if (!jpeg_finish_output(dinfo)) {
            break;
        }
        output_passes++;
    }
} else {
    this->readRows(dstInfo, dst, dstRowBytes, count, options, &rows);
}
```
