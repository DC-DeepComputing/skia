---
name: skia/codec/query-frame-fully-received-in-streaming-codecs
description: When implementing SkCodec::onGetFrameInfo in streaming or incremental decoders, query whether frame data is complete rather than hardcoding FrameInfo::fFullyReceived to true
rubric: correctness
severity: should_fix
---

**Rule.** When implementing SkCodec::onGetFrameInfo in codecs supporting
streaming, partial, or incremental decoding, dynamically query the underlying
reader or stream to determine whether the frame or image payload has been
completely received. Do not hardcode info->fFullyReceived = true
unconditionally, which falsely signals to callers that truncated frames are
complete.

**Why.** In streaming and incremental decoding environments (such as web
browsers), image streams arrive in progressive chunks. Hardcoding fFullyReceived
= true misleads callers like animated image decoders and render pipelines into
treating incomplete or truncated frames as final, preventing subsequent updates
when remaining data arrives.

## Examples

Bad:

```cpp
bool SkBmpRustCodec::onGetFrameInfo(int index, FrameInfo* info) const {
    if (index != 0) {
        return false;
    }
    if (info) {
        info->fFullyReceived = true;
    }
    return true;
}
```

Good:

```cpp
bool SkBmpRustCodec::onGetFrameInfo(int index, FrameInfo* info) const {
    if (index != 0) {
        return false;
    }
    if (info) {
        info->fFullyReceived = fReader->image_data_loaded();
    }
    return true;
}
```
