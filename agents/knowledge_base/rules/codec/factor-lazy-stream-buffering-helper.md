---
name: skia/codec/factor-lazy-stream-buffering-helper
description: When image codecs lazily read remaining stream data to update parser or demuxer state, encapsulate stream copying and re-parsing in a shared helper
rubric: design
severity: should_fix
---

**Rule.** When multiple entry points in an image codec (such as onGetFrameCount
and onGetPixels) require reading the remainder of a stream and updating parser
or demuxer state, encapsulate the stream copy and re-parsing logic in a
dedicated helper method (e.g., ensureAllData()) returning a boolean status. Do
not duplicate stream buffering, parser state re-initialization, or partial parse
checks across callers.

**Why.** Duplicating multi-step stream copying and parser state
re-initialization across entry points creates maintenance overhead, increases
code bloat, and risks inconsistent error reporting or divergent demuxer state
between decode modes.

## Examples

Bad:

```cpp
// In onGetFrameCount():
if (fOnlyHeaderParsed) {
    SkDynamicMemoryWStream newData;
    newData.write(fData->data(), fData->size());
    SkStreamPriv::Copy(&newData, this->stream());
    fData = newData.detachAsData();
    fOnlyHeaderParsed = false;
    if (!reinitDemuxer()) {
        return 0;
    }
}
// Identical stream copy and reinit duplicated in onGetPixels()
```

Good:

```cpp
bool SkWebpCodec::ensureAllData() {
    if (fOnlyHeaderParsed) {
        SkDynamicMemoryWStream newData;
        newData.write(fData->data(), fData->size());
        SkStreamPriv::Copy(&newData, this->stream());
        fData = newData.detachAsData();
        fOnlyHeaderParsed = false;
        return reinitDemuxer();
    }
    return true;
}

// In callers (e.g. onGetFrameCount, onGetPixels):
if (!ensureAllData()) {
    return 0; // or return kInvalidInput;
}
```
