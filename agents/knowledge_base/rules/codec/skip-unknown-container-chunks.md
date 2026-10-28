---
name: skia/codec/skip-unknown-container-chunks
description: When parsing chunk-based container formats (like RIFF or WebP), do not reject unrecognized chunk types; skip or pass through unknown chunks by payload size
rubric: correctness
severity: should_fix
---

**Rule.** When parsing chunk-based container formats (such as RIFF, WebP, or
PNG), do not reject or fail decoding upon encountering unrecognized chunk types
against a hardcoded whitelist. Instead, safely skip or pass through unknown
chunks using their declared payload length to maintain forward compatibility
with newer or optional format extensions.

**Why.** Chunk-based container formats are designed to be extensible, allowing
writers to include optional or newly standardized metadata and feature chunks
that older decoders can safely ignore. Enforcing a strict whitelist of
recognized chunk types causes decoders to reject valid files containing optional
chunks, breaking forward compatibility.

## Examples

Bad:

```cpp
bool is_valid_chunk_header(const void* buf, size_t bytesRead) {
    // Bad: Hardcoded whitelist rejects valid files containing extension chunks.
    return bytesRead >= 8 &&
           (!memcmp(buf, "ICCP", 4) ||
            !memcmp(buf, "ANIM", 4) ||
            !memcmp(buf, "ANMF", 4) ||
            !memcmp(buf, "ALPH", 4));
}
...
if (!is_valid_chunk_header(buffer, bytes_read)) {
    return {false, nullptr};
}
```

Good:

```cpp
// Good: Read chunk header, process known chunks, and skip/pass through unknown chunks.
if (!memcmp(buffer, "ICCP", 4)) {
    handle_iccp_chunk(stream, payloadSize);
} else {
    // Skip or pass through unrecognized chunks by their payload length.
    while (payloadSize > 0) {
        size_t to_read = std::min((size_t)payloadSize, kBufferSize);
        size_t bytes_read = stream->read(buffer, to_read);
        if (!bytes_read) {
            break;
        }
        tempStream.write(buffer, bytes_read);
        payloadSize -= bytes_read;
    }
}
```
