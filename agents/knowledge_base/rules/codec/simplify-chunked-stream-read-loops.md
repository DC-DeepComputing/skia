---
name: skia/codec/simplify-chunked-stream-read-loops
description: When reading a fixed payload from a stream in buffer-sized chunks, pass std::min(remaining, buffer_size) directly and decrement remaining bytes in-place
rubric: readability
severity: should_fix
---

**Rule.** When reading a fixed payload from a stream in buffer-sized chunks,
pass std::min(remaining, buffer_size) directly to the read call and decrement
remaining -= bytes_read in-place. Do not introduce intermediate counter
variables, separate clamp variables, or manual branching to compute chunk sizes.

**Why.** Using std::min directly in the read call and decrementing the remaining
byte count in-place eliminates redundant tracking variables and manual
branching. It also leaves remaining as a clean loop exit check, allowing callers
to verify remaining == 0 after the loop to detect premature stream truncation.

## Examples

Bad:

```cpp
size_t payloadCounter = payloadSize;
while (payloadCounter) {
    size_t readSize = payloadCounter;
    if (payloadCounter > bufferSize) {
        readSize = bufferSize;
    }
    bytes_read = stream->read(buffer, readSize);
    if (bytes_read < readSize) {
        return {false, nullptr};
    }
    payloadCounter -= readSize;
    tempStream.write(buffer, bytes_read);
}
```

Good:

```cpp
while (payloadSize) {
    bytes_read = stream->read(buffer, std::min((size_t)payloadSize, kBufferSize));
    if (!bytes_read) {
        break;
    }
    tempStream.write(buffer, bytes_read);
    payloadSize -= bytes_read;
}
if (payloadSize) {
    SkASSERT(stream->isAtEnd());
    return {false, nullptr};
}
```
