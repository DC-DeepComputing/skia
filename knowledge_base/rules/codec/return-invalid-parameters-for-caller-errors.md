---
name: skia/codec/return-invalid-parameters-for-caller-errors
description: When returning SkCodec::Result errors in codec factory methods or decoders, return kInvalidParameters for caller errors and kIncompleteInput for truncated streams rather than kInvalidInput
rubric: correctness
severity: should_fix
---

**Rule.** When reporting errors in SkCodec implementations or factory methods
(such as MakeFromStream), return SkCodec::kInvalidParameters (or set *result =
kInvalidParameters) for invalid arguments, null stream/buffer pointers, invalid
row bytes, or out-of-bounds parameters passed by the caller. When the input
stream ends prematurely or image headers are truncated, return
SkCodec::kIncompleteInput so streaming and progressive decoders can resume when
more data arrives. Reserve SkCodec::kInvalidInput strictly for unrecoverable
malformed, corrupted, or unrecognized stream data.

**Why.** kInvalidParameters signals a caller programming error rather than an
asset issue. kIncompleteInput is a recoverable status indicating that more data
may arrive, which is essential for streaming and progressive decoding in callers
like Chromium. Using kInvalidInput for caller errors or truncated headers
incorrectly treats recoverable or caller bugs as fatal stream corruption.

## Examples

Bad:

```cpp
// Bad: Returning kInvalidInput for caller argument errors or truncated headers
std::unique_ptr<SkCodec> SkFooCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                    Result* result) {
    if (!stream) {
        *result = kInvalidInput;
        return nullptr;
    }
    auto headerResult = ParseHeader(stream.get());
    if (headerResult == HeaderResult::kTruncated) {
        *result = kInvalidInput;
        return nullptr;
    }
    // ...
}
```

Good:

```cpp
// Good: Return kInvalidParameters for caller errors and kIncompleteInput for truncated data
std::unique_ptr<SkCodec> SkFooCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                    Result* result) {
    if (!stream) {
        *result = kInvalidParameters;
        return nullptr;
    }
    auto headerResult = ParseHeader(stream.get());
    if (headerResult == HeaderResult::kTruncated) {
        *result = kIncompleteInput;
        return nullptr;
    }
    // ...
}
```
