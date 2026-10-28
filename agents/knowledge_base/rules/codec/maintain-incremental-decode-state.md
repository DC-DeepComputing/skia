---
name: skia/codec/maintain-incremental-decode-state
description: When implementing SkCodec incremental decoding, maintain persistent state across chunks, reset state when result != kIncompleteInput, and document the lifecycle invariant
rubric: readability
severity: should_fix
---

**Rule.** When implementing SkCodec::onIncrementalDecode (or internal
incremental decode helpers), maintain persistent decoder state across iterations
to resume decoding from newly received stream bytes rather than restarting from
byte 0. Additionally, reset the incremental decoding state whenever decoding
completes or encounters an error (result != kIncompleteInput) using a direct
inequality check rather than redundant conditions (like result == kSuccess ||
result != kIncompleteInput), and document the lifecycle invariant in an
explanatory comment noting that kIncompleteInput is the sole recoverable result
while kSuccess and fatal errors terminate the session.

**Why.** Incremental decoding is designed for push-model streaming where chunks
arrive progressively; re-decoding from byte 0 causes O(N^2) CPU overhead.
Furthermore, kIncompleteInput is the sole result where decoding state is
retained across calls while awaiting more data, so all other results terminate
the session and reset the state; documenting this invariant prevents maintainers
from misunderstanding the reset condition or attempting invalid re-entry.

## Examples

Bad:

```cpp
SkCodec::Result SkCodecSubclass::onIncrementalDecode(int* rowsDecoded) {
    // Bad: Re-decodes entire image from byte 0 on every incremental chunk
    rust_decoder::DecodingResult readResult = fReader->read_image_data();
    Result result = (readResult == rust_decoder::DecodingResult::IncompleteInput)
            ? kIncompleteInput : (fReader->get_next_rows(dst) ? kSuccess : kErrorInInput);
    // Bad: Redundant compound check and missing comment on state reset lifecycle
    if (result == kSuccess || result != kIncompleteInput) {
        fIncrementalDecodingState.reset();
    }
    return result;
}
```

Good:

```cpp
SkCodec::Result SkCodecSubclass::onIncrementalDecode(int* rowsDecoded) {
    // Good: Resumes decoding using persistent incremental decoder state
    rust_decoder::DecodingResult readResult = fReader->read_incremental_image_data();
    Result result = (readResult == rust_decoder::DecodingResult::IncompleteInput)
            ? kIncompleteInput : (fReader->get_next_rows(dst) ? kSuccess : kErrorInInput);
    // Good: Resets state directly when result != kIncompleteInput and documents invariant
    if (result != kIncompleteInput) {
        // kIncompleteInput is the only recoverable result allowing continued decoding;
        // kSuccess and fatal errors terminate the session and allow startIncrementalDecode.
        fIncrementalDecodingState.reset();
    }
    return result;
}
```
