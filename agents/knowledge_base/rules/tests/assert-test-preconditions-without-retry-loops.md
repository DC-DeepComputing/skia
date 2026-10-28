---
name: skia/tests/assert-test-preconditions-without-retry-loops
description: When writing unit tests with deterministic inputs or bounded streams, assert preconditions directly rather than writing defensive retry loops or chunk-pumping fallbacks
rubric: readability
severity: should_fix
---

**Rule.** Do not add defensive retry loops, polling loops, or chunk-pumping
fallback logic in unit tests to satisfy preconditions that deterministic test
setup should already guarantee (such as sizing partial streams with sufficient
initial bytes for header decoding). Assert successful creation or completion
directly with REPORTER_ASSERT to fail fast if setup invariants are violated.

**Why.** In unit tests, test inputs and buffer boundaries are deterministic and
fully controlled. Defensive retry loops introduce dead complexity and can
silently mask regressions where an operation unexpectedly consumes more data
than intended to satisfy its precondition.

## Examples

Bad:

```cpp
auto haltingStream = std::make_unique<HaltingStream>(data, initialBytes);
std::unique_ptr<SkCodec> codec = SkBmpRustDecoder::Decode(std::move(haltingStream), &result);
while (!codec && result == SkCodec::kIncompleteInput && !streamPtr->isAllDataReceived()) {
    streamPtr->addNewData(chunkSize);
    haltingStream = std::make_unique<HaltingStream>(data, streamPtr->getLength());
    streamPtr = haltingStream.get();
    codec = SkBmpRustDecoder::Decode(std::move(haltingStream), &result);
}
REPORTER_ASSERT(r, codec, "Failed to create codec");
```

Good:

```cpp
auto haltingStream = std::make_unique<HaltingStream>(data, initialBytes);
std::unique_ptr<SkCodec> codec = SkBmpRustDecoder::Decode(std::move(haltingStream), &result);
REPORTER_ASSERT(r, codec, "Failed to create codec with %zu bytes, result: %s",
                initialBytes, SkCodec::ResultToString(result));
```
