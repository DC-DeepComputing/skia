---
name: skia/gpu/pass-buffer-byte-size-to-base64
description: When passing multi-byte typed buffers or spans to byte-oriented utilities like SkBase64::Encode and SkBase64::EncodedSize, pass buffer size in bytes rather than element count
rubric: correctness
severity: should_fix
---

**Rule.** When passing multi-byte typed buffers or spans (such as uint32_t[],
float[], or SkSpan<const T>) to byte-oriented APIs like SkBase64::Encode and
SkBase64::EncodedSize, always pass the buffer size in bytes (e.g. dataLength *
sizeof(T) or span.size_bytes()) rather than the raw element count.

**Why.** Byte-oriented utilities like SkBase64::Encode accept void* data and
interpret length arguments as byte counts. Passing raw element counts for
multi-byte types silently truncates the payload (e.g., encoding only the first
quarter of a 32-bit integer array).

## Examples

Bad:

```cpp
// Bug: dataLength is the uint32_t element count; SkBase64 truncates to dataLength bytes.
SkAutoMalloc encodedData{SkBase64::EncodedSize(dataLength)};
char* dst = static_cast<char*>(encodedData.get());
size_t encodedLen = SkBase64::Encode(keyData.data(), dataLength, dst);
```

Good:

```cpp
const size_t srcDataSize = dataLength * sizeof(uint32_t);
SkAutoMalloc encodedData{SkBase64::EncodedSize(srcDataSize)};
char* dst = static_cast<char*>(encodedData.get());
size_t encodedLen = SkBase64::Encode(keyData.data(), srcDataSize, dst);
```
