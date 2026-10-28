---
name: skia/codec/codec-name-chunked-streams-partial-not-progressive
description: When naming streams, test cases, or variables representing chunked or incomplete input data, use 'partial' or 'streaming' instead of 'progressive'
rubric: readability
severity: should_fix
---

**Rule.** Do not use 'progressive' (such as in class names like
ProgressiveStream or test names like IncrementalDecode_ProgressiveStream) to
describe streams, variables, or tests dealing with chunked or partially
available input data. Use 'partial' or 'streaming' (such as HaltingStream,
PartialStream, or PartialStreaming) instead, reserving 'progressive' strictly
for multi-pass image encoding formats.

**Why.** In image decoding, 'progressive' specifically denotes multi-pass image
encoding formats (such as progressive JPEG or interlaced PNG/GIF) where
successive scans refine the whole image. Using 'progressive' for data that
arrives incrementally or in chunks conflates stream transport mechanics with
multi-pass image formats, creating ambiguity in codec and test implementations.

## Examples

Bad:

```cpp
// Bad: Using 'progressive' to describe chunked or partial input streaming
DEF_TEST(Codec_Bmp_Rust_IncrementalDecode_ProgressiveStream, r) {
    sk_sp<SkData> data = GetResourceAsData("images/test.bmp");
    auto stream = std::make_unique<HaltingStream>(data, data->size() / 2);
    // ...
}
```

Good:

```cpp
// Good: Using 'partial' or 'streaming' for chunked stream arrival
DEF_TEST(RustBmpCodec_IncrementalDecode_PartialStreaming, r) {
    sk_sp<SkData> data = GetResourceAsData("images/test.bmp");
    auto stream = std::make_unique<HaltingStream>(data, data->size() / 2);
    // ...
}
```
