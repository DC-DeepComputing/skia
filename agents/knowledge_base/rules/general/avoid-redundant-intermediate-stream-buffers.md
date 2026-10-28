---
name: skia/avoid-redundant-intermediate-stream-buffers
description: When serializing to an SkWStream, write sequential data directly to the destination stream, and pipe buffered SkDynamicMemoryWStream using writeToStream() rather than detachAsData()
rubric: safety
severity: should_fix
---

**Rule.** Do not buffer serialized data in a temporary intermediate stream (such
as SkDynamicMemoryWStream) when the data order matches the output layout and can
be written directly to the destination SkWStream. When intermediate buffering is
necessary (such as staging out-of-order blocks or deferred payloads), write the
dynamic stream into the destination stream using writeToStream(&stream) (or
writeToAndReset(&stream)) rather than allocating an intermediate SkData via
detachAsData().

**Why.** Buffering sequential data into a temporary stream incurs redundant
memory allocations and unnecessary copies. When an intermediate
SkDynamicMemoryWStream is required, calling detachAsData() allocates a new
contiguous heap buffer and copies all chunk blocks into an SkData, whereas
writeToStream() streams the internal chunk blocks directly into the destination
stream without redundant allocation or copying.

## Examples

Bad:

```cpp
// Bad: Buffering sequential output in an intermediate stream
SkDynamicMemoryWStream subIFD;
if (subIFDExists) {
    SkStreamPriv::WriteU16BE(&subIFD, numSubIFDTags);
}
if (subIFDExists && !subIFD.writeToStream(&stream)) {
    return false;
}

// Bad: Materializing intermediate SkData to write into destination stream
sk_sp<SkData> bufferData = buffer.detachAsData();
if (!stream.write(bufferData->data(), bufferData->size())) {
    return false;
}
```

Good:

```cpp
// Good: Writing sequential data directly into the destination stream
if (subIFDExists) {
    if (!SkStreamPriv::WriteU16BE(&stream, numSubIFDTags)) {
        return false;
    }
}

// Good: Streaming buffered dynamic memory blocks directly
if (!buffer.writeToStream(&stream)) {
    return false;
}
```
