---
name: skia/codec/omit-scanline-decode-fallback-in-container-codecs
description: When delegating incremental decoding in container codecs, do not add fallback branches calling startScanlineDecode for sub-codecs that implement incremental decoding rather than scanline decoding
rubric: readability
severity: should_fix
---

**Rule.** When delegating incremental decoding in container image decoders (such
as SkIcoRustCodec), do not add fallback branches invoking startScanlineDecode on
kUnimplemented results from sub-codecs. Verify whether the underlying sub-codec
(such as SkBmpRustCodec) supports incremental decoding directly and lacks
scanline decoding before carrying over legacy C++ fallback branches.

**Why.** Legacy C++ codecs like SkBmpCodec only supported scanline decoding,
requiring callers to fall back from incremental decode attempts. Modern
replacements like SkBmpRustCodec implement startIncrementalDecode directly and
do not support scanline decoding, making scanline fallback branches unreachable
or broken dead code.

## Examples

Bad:

```cpp
// Obsolete fallback from legacy SkBmpCodec that only supported scanline decoding
SkCodec::Result r = embeddedCodec->startIncrementalDecode(dstInfo, pixels, rowBytes, &opts);
if (r == SkCodec::kUnimplemented) {
    if (embeddedCodec->startScanlineDecode(dstInfo) == SkCodec::kSuccess) {
        return SkCodec::kUnimplemented;
    }
}
```

Good:

```cpp
// SkBmpRustCodec directly implements incremental decoding; omit dead scanline fallback
SkCodec::Result r = embeddedCodec->startIncrementalDecode(dstInfo, pixels, rowBytes, &opts);
if (r == SkCodec::kSuccess) {
    fCurrCodec = embeddedCodec;
    return SkCodec::kSuccess;
}
```
