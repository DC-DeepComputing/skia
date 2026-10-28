---
name: skia/validate-stream-data-before-buffering
description: When buffering input from an SkStream into an accumulation buffer or SkDynamicMemoryWStream, validate the read byte count and magic header bytes before writing
rubric: safety
severity: should_fix
---

**Rule.** Validate stream read counts and header magic bytes before buffering or
writing data into an output or accumulation stream (such as
SkDynamicMemoryWStream). Do not write unverified bytes into in-memory streams or
allocate dynamic buffers ahead of early validation bailouts.

**Why.** Writing unvalidated bytes to an accumulation stream or dynamic memory
buffer causes unnecessary heap allocations, block expansions, and memory copies
when the input stream is truncated, corrupted, or of an unexpected format.
Validating read lengths and magic bytes upfront ensures malformed inputs fail
fast with zero allocation overhead.

## Examples

Bad:

```cpp
SkDynamicMemoryWStream tempStream;
size_t bytesRead = stream->read(buffer, kHeaderSize);
tempStream.write(buffer, bytesRead);
if (bytesRead < kHeaderSize ||
    memcmp(buffer, "RIFF", 4) != 0 ||
    memcmp(&buffer[8], "WEBP", 4) != 0) {
    return {false, nullptr};
}
```

Good:

```cpp
SkDynamicMemoryWStream tempStream;
size_t bytesRead = stream->read(buffer, kHeaderSize);
if (bytesRead < kHeaderSize ||
    memcmp(buffer, "RIFF", 4) != 0 ||
    memcmp(&buffer[8], "WEBP", 4) != 0) {
    return {false, nullptr};
}
tempStream.write(buffer, bytesRead);
```
