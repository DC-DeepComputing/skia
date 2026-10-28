---
name: skia/codec/test-progressive-decode-inverted-row-formats
description: When testing SkCodec incremental decoding with chunked streams, assert exact decoded row and iteration counts, verify partial pixels, and test all row orientations
rubric: testing
severity: should_fix
---

**Rule.** When adding or modifying SkCodec incremental decoding tests with
chunked inputs (e.g., using HaltingStream from tests/FakeStreams.h), verify that
the codec returns kIncompleteInput on partial buffers and resumes cleanly when
new data arrives. In the incremental decode loop, pass &rowsDecoded to
incrementalDecode and assert exact expected row counts and iteration counts
rather than merely checking non-zero progress (rowsDecoded > 0), and verify that
intermediate partially decoded rows populate valid pixel data matching reference
images. Additionally, for formats supporting multiple or inverted row orderings
(such as BMP's top-down and bottom-up modes), test partial streaming across all
supported orientations and verify opposing boundary pixels against a reference
decoder.

**Why.** Image decoders often receive data progressively over streams.
Simulating chunked input with HaltingStream verifies graceful resumption without
state corruption. Asserting exact row counts and iteration counts—rather than
merely checking rowsDecoded > 0—prevents masking off-by-one errors in row
padding, buffer offset bugs, or decoders that buffer all data until completion,
while testing all row orientations catches coordinate inversion regressions in
bottom-up formats.

## Examples

Bad:

```cpp
// Bad: Only asserting final kSuccess, checking rowsDecoded > 0,
// or only testing a single default row orientation.
while (result == SkCodec::kIncompleteInput) {
    if (rowsDecoded > 0 && rowsDecoded < codec->getInfo().height()) {
        sawPartialRows = true; // Weak assertion: hides partial decode or row padding bugs
    }
    haltingStream->addNewData(chunkSize);
    result = codec->incrementalDecode(&rowsDecoded);
}
REPORTER_ASSERT(r, sawPartialRows);
REPORTER_ASSERT(r, result == SkCodec::kSuccess);
```

Good:

```cpp
// Good: Assert exact expected rows and iterations on deterministic chunks:
int iterations = 0;
while (result == SkCodec::kIncompleteInput) {
    if (iterations == 0) {
        REPORTER_ASSERT(r, rowsDecoded == testCase.expectedFirstRows);
        REPORTER_ASSERT(r, rowsMatchReference(bitmap, reference, rowsDecoded));
    }
    haltingStream->addNewData(chunkSize);
    result = codec->incrementalDecode(&rowsDecoded);
    iterations++;
}
REPORTER_ASSERT(r, iterations == testCase.expectedIterations);
REPORTER_ASSERT(r, result == SkCodec::kSuccess);

// Test all supported orientations (top-down and bottom-up) against reference:
for (const auto& tc : {topDownCase, bottomUpCase}) {
    REPORTER_ASSERT(r, actual.getColor(0, 0) == ref.getColor(0, 0));
    REPORTER_ASSERT(r, actual.getColor(0, h - 1) == ref.getColor(0, h - 1));
}
```
