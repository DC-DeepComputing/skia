---
name: skia/general-prefer-skmemorystream-make-over-make-unique
description: When constructing heap-allocated SkMemoryStream instances from SkData or raw buffers, use static SkMemoryStream::Make factory methods rather than std::make_unique<SkMemoryStream>
rubric: readability
severity: nit
---

**Rule.** When constructing a heap-allocated SkMemoryStream (or SkFILEStream)
from SkData, raw memory, or file paths, use the static factory
SkMemoryStream::Make(...) rather than std::make_unique<SkMemoryStream>(...).
When only a temporary stream is needed for a local call and ownership is not
transferred, allocate it on the stack instead.

**Why.** SkMemoryStream::Make is Skia's dedicated static factory that returns
std::unique_ptr<SkMemoryStream>, providing more idiomatic and concise stream
construction than invoking std::make_unique directly.

## Examples

Bad:

```cpp
// Bad: Calling std::make_unique directly for SkMemoryStream
std::unique_ptr<SkCodec> codec = SkBmpRustDecoder::Decode(
        std::make_unique<SkMemoryStream>(std::move(data)), &result);
```

Good:

```cpp
// Good: Calling SkMemoryStream::Make factory method
std::unique_ptr<SkCodec> codec = SkBmpRustDecoder::Decode(
        SkMemoryStream::Make(std::move(data)), &result);
```
