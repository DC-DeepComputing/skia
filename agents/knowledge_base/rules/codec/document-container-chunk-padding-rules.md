---
name: skia/codec/document-container-chunk-padding-rules
description: When parsing container formats and adjusting chunk payload sizes or stream offsets for specification-mandated padding, document the format requirement in comments
rubric: readability
severity: should_fix
---

**Rule.** When parsing container image or media formats and adjusting chunk
payload sizes or stream read offsets to account for specification-mandated
padding (such as RIFF 2-byte alignment for odd chunk sizes), document the
governing format specification rule in an inline comment. Explain why payload
sizes are incremented or trailing padding bytes are skipped rather than leaving
unannotated offset arithmetic.

**Why.** In container formats like RIFF or WebP, odd-sized chunk payloads are
followed by a padding byte to maintain word alignment. Incrementing chunk
payload sizes or advancing stream offsets without an explanatory comment looks
like an off-by-one bug or unhandled read failure to maintainers.

## Examples

Bad:

```cpp
uint32_t payloadSize;
memcpy(&payloadSize, &buffer[4], 4);
if (payloadSize % 2 != 0) {
    payloadSize += 1;
}
```

Good:

```cpp
// According to the RIFF document format, if 'payloadSize' is odd a single
// padding byte -- which must be 0 to conform with RIFF -- is added.
uint32_t payloadSize;
memcpy(&payloadSize, &buffer[4], 4);
if (payloadSize % 2 != 0) {
    payloadSize += 1;
}
```
