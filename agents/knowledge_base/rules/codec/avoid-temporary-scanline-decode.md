---
name: skia/codec/avoid-temporary-scanline-decode
description: When implementing new SkCodec subclasses, omit optional scanline decoding overrides (onStartScanlineDecode, onGetScanlines) and rely on default base stubs unless specifically required
rubric: design
severity: should_fix
---

**Rule.** When implementing new SkCodec subclasses, do not implement optional
scanline decoding methods (onStartScanlineDecode, onGetScanlines,
onSkipScanlines) unless specifically required by the target platform. Rely on
default kUnimplemented stubs in base SkCodec rather than writing unused scanline
boilerplate or introducing temporary non-streaming scanline decoding; implement
onGetPixels for full-frame decoding and add
onStartIncrementalDecode/onIncrementalDecode when streaming is needed.

**Why.** Scanline decoding is an optional decoding mode that modern consumers
(such as Chromium and Blink) do not use, relying instead on full-frame pixel
decoding or push-model incremental streaming. Implementing unused scanline
methods introduces substantial dead code and state tracking, while temporary
non-streaming scanline decoders fail to satisfy streaming requirements.

## Examples

Bad:

```cpp
// Implementing optional or temporary scanline decoding in new SkCodec
class SkExampleCodec : public SkCodec {
    Result onStartScanlineDecode(const SkImageInfo& dstInfo,
                                 const Options& options) override;
    int onGetScanlines(void* dst, int count, size_t rowBytes) override;
    bool onSkipScanlines(int count) override;
};
```

Good:

```cpp
// Rely on base SkCodec defaults (returning kUnimplemented) for scanlines
class SkExampleCodec : public SkCodec {
    Result onGetPixels(const SkImageInfo& info,
                       void* dst,
                       size_t dstRowStride,
                       const Options& options,
                       int* rowsDecoded) override;
    // Implement onStartIncrementalDecode / onIncrementalDecode if streaming is needed.
};
```
