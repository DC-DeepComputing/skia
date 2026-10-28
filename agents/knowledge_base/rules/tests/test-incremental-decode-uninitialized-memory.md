---
name: skia/tests/test-incremental-decode-uninitialized-memory
description: When testing SkCodec incremental or streaming decoding in unit tests, pre-fill destination bitmaps with non-zero sentinel bytes and set Options::fZeroInitialized to kNo_ZeroInitialized
rubric: testing
severity: should_fix
---

**Rule.** When writing or modifying unit tests for SkCodec incremental or
streaming decoding (using startIncrementalDecode and incrementalDecode), test
with non-zero initialized destination buffers by pre-filling destination bitmaps
with a sentinel byte pattern (e.g. via memset or eraseColor) and explicitly
setting SkCodec::Options::fZeroInitialized to SkCodec::kNo_ZeroInitialized.
Verify that the decoder properly writes all expected pixels and overwrites the
sentinel data without relying on zero-initialized memory.

**Why.** Callers of incremental or progressive decoding often supply
uninitialized memory buffers (kNo_ZeroInitialized). Testing solely with
zero-initialized or transparently cleared buffers masks decoder bugs where
pixels or transparent regions are not written, or where scanline decoders
erroneously assume destination memory has been pre-cleared.

## Examples

Bad:

```cpp
SkBitmap bitmap;
bitmap.allocPixels(codec->getInfo());
bitmap.eraseColor(SK_ColorTRANSPARENT);

// Bad: Relies on zero-cleared memory and omits uninitialized memory testing.
SkCodec::Result result = codec->startIncrementalDecode(
    bitmap.info(), bitmap.getPixels(), bitmap.rowBytes());
REPORTER_ASSERT(r, result == SkCodec::kSuccess);
```

Good:

```cpp
SkBitmap bitmap;
bitmap.allocPixels(codec->getInfo());
// Pre-fill destination with sentinel bytes to verify uninitialized memory handling.
memset(bitmap.getPixels(), 0xAB, bitmap.computeByteSize());

SkCodec::Options options;
options.fZeroInitialized = SkCodec::kNo_ZeroInitialized;
SkCodec::Result result = codec->startIncrementalDecode(
    bitmap.info(), bitmap.getPixels(), bitmap.rowBytes(), &options);
REPORTER_ASSERT(r, result == SkCodec::kSuccess);
```
