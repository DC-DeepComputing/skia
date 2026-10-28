---
name: skia/codec/require-complete-initial-progressive-pass
description: When handling progressive or multi-pass image decoding, fail with kIncompleteInput if the initial pass fails rather than treating incomplete first-pass data as usable
rubric: correctness
severity: should_fix
---

**Rule.** When implementing progressive or multi-pass image decoding (such as
progressive JPEG), return SkCodec::kIncompleteInput if the initial output pass
fails to finish or decode completely. Only allow breaking early or treating a
truncated stream as partially usable after at least one full scan pass has
successfully completed.

**Why.** Progressive decoding refines an image over successive passes across the
entire frame. If the very first pass fails to finish, no valid baseline frame
has been decoded, so treating the buffer as valid produces corrupted or
uninitialized output rather than reporting an incomplete input error.

## Examples

Bad:

```cpp
// Breaking or continuing when pass 0 fails leaves an incomplete, unusable image
if (!jpeg_finish_output(dinfo)) {
    break;
}
output_passes++;
```

Good:

```cpp
// Require at least one complete pass before accepting partial stream data
if (!jpeg_finish_output(dinfo)) {
    if (output_passes > 0) {
        break;
    }
    return fDecoderMgr->returnFailure("Incomplete image data", kIncompleteInput);
}
output_passes++;
```
