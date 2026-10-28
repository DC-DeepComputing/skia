---
name: skia/codec/use-restore-bgcolor-for-independent-frames
description: When implementing SkCodec::onGetFrameInfo for multi-frame containers or independent frames, set fDisposalMethod to kRestoreBGColor instead of kKeep
rubric: correctness
severity: should_fix
---

**Rule.** When implementing SkCodec::onGetFrameInfo() for multi-image containers
(such as ICO) or codecs with independent frames, set info->fDisposalMethod to
SkCodecAnimation::DisposalMethod::kRestoreBGColor rather than kKeep. Along with
setting fRequiredFrame = SkCodec::kNoFrame and fBlend =
SkCodecAnimation::Blend::kSrc, this explicitly signals that frames do not
composite over prior frames.

**Why.** kKeep specifies that the subsequent frame renders on top of the current
frame, forcing embedders (such as Chromium and Blink) to retain decoded frame
buffers in memory for composition. Setting kRestoreBGColor communicates that the
canvas is cleared between frames and prior frame buffers can be immediately
discarded or recycled.

## Examples

Bad:

```cpp
// Independent frames in a container or multi-frame codec
info->fRequiredFrame = SkCodec::kNoFrame;
info->fDisposalMethod = SkCodecAnimation::DisposalMethod::kKeep;
info->fBlend = SkCodecAnimation::Blend::kSrc;
```

Good:

```cpp
// Independent frames in a container or multi-frame codec
info->fRequiredFrame = SkCodec::kNoFrame;
info->fDisposalMethod = SkCodecAnimation::DisposalMethod::kRestoreBGColor;
info->fBlend = SkCodecAnimation::Blend::kSrc;
```
