---
name: skia/codec/avoid-synthetic-stream-markers
description: When implementing decode termination or stream truncation in codec source managers, control decoders via library APIs instead of injecting synthetic marker bytes
rubric: correctness
severity: should_fix
---

**Rule.** Do not implement stream truncation or decode termination by injecting
synthetic marker bytes (such as JPEG End-of-Image markers) into specific stream
source manager subclasses. Ensure decode logic is stream-agnostic and uniform
across all SkStream backings, controlling decoder termination and scan handling
via the underlying library's decode control APIs instead of forging stream
payloads.

**Why.** Skia decoders adapt multiple SkStream backings (memory, buffered,
unseekable) through source managers. Injecting synthetic markers into a specific
subclass leaves other stream types broken or inconsistent, and makes fragile
assumptions about the third-party decoding library's internal state machine.

## Examples

Bad:

```cpp
// In SkJpegMemorySourceMgr::fillInputBuffer:
if (fShouldTruncate) {
    uint8_t markerCode[kJpegMarkerCodeSize] = {0xFF, kJpegMarkerEndOfImage};
    nextInputByte = markerCode;
    bytesInBuffer = 2;
    return true;
}
```

Good:

```cpp
// Control decode termination and scan handling via decoder library APIs
// (e.g., jpeg_consume_input or checking output pass state) rather than
// forging synthetic marker bytes in specific stream manager subclasses.
```
