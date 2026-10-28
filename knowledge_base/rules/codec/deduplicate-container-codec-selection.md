---
name: skia/codec/deduplicate-container-codec-selection
description: In container codecs delegating to embedded sub-codecs, factor candidate selection, options normalization, and sub-codec dispatch into a helper or lambda across lookups and decode entry points
rubric: design
severity: should_fix
---

**Rule.** In container or composite codecs that delegate to embedded sub-codecs,
factor candidate selection, options normalization (such as resetting fFrameIndex
= 0), and sub-codec invocation boilerplate into a shared helper method or local
lambda. Do not duplicate delegation setup and decode calls across direct-frame
index lookups and fallback search loops, or across separate decode entry points
(onGetPixels, onStartIncrementalDecode).

**Why.** Repeating embedded codec delegation boilerplate—such as copying
options, resetting frame index for single-frame sub-codecs, and setting rows
decoded—clutters dispatch methods and risks subtle discrepancies in fallback
behavior, dimension validation, or error return handling between lookup paths
and decode modes.

## Examples

Bad:

```cpp
// Duplicated across direct lookup and search loop in decode entry points:
if (opts.fFrameIndex >= 0 && opts.fFrameIndex < fEmbeddedCodecs->size()) {
    SkCodec* embeddedCodec = fEmbeddedCodecs->at(opts.fFrameIndex).get();
    if (embeddedCodec->dimensions() == dstInfo.dimensions()) {
        Options embeddedOpts = opts;
        embeddedOpts.fFrameIndex = 0;
        SkCodec::Result r = embeddedCodec->getPixels(dstInfo, dst, dstRowBytes, &embeddedOpts);
        if (r == kSuccess || r == kIncompleteInput) { *rowsDecoded = dstInfo.height(); }
        return r;
    }
}
int index = 0;
while ((index = this->chooseCodec(dstInfo.dimensions(), index)) >= 0) {
    SkCodec* embeddedCodec = fEmbeddedCodecs->at(index++).get();
    Options embeddedOpts = opts;
    embeddedOpts.fFrameIndex = 0;
    SkCodec::Result r = embeddedCodec->getPixels(dstInfo, dst, dstRowBytes, &embeddedOpts);
    if (r == kSuccess || r == kIncompleteInput) { *rowsDecoded = dstInfo.height(); return r; }
}
```

Good:

```cpp
// Factored into a local lambda or helper across lookups and entry points:
Options embeddedOpts = opts;
embeddedOpts.fFrameIndex = 0;
auto tryDecode = [&](SkCodec* codec) -> SkCodec::Result {
    SkCodec::Result r = codec->getPixels(dstInfo, dst, dstRowBytes, &embeddedOpts);
    if (r == kSuccess || r == kIncompleteInput) {
        *rowsDecoded = dstInfo.height();
    }
    return r;
};

if (opts.fFrameIndex >= 0 && opts.fFrameIndex < fEmbeddedCodecs->size()) {
    SkCodec* embeddedCodec = fEmbeddedCodecs->at(opts.fFrameIndex).get();
    if (embeddedCodec->dimensions() == dstInfo.dimensions()) {
        return tryDecode(embeddedCodec);
    }
}
return this->selectAndDispatch(dstInfo.dimensions(), tryDecode);
```
