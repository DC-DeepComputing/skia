---
name: skia/codec/prefer-stream-getdata-for-readonly-skdata
description: When obtaining read-only SkData from an SkStream, call stream->getData() directly rather than using intermediate const-casting helpers or casting away constness
rubric: readability
severity: should_fix
---

**Rule.** When obtaining read-only `SkData` from an `SkStream`, call
`stream->getData()` directly rather than wrapping the call in intermediate
helpers or casting away constness with `const_cast`. Preserve the native
`sk_sp<const SkData>` return type instead of converting to mutable
`sk_sp<SkData>` when caller code only reads the buffer.

**Why.** `SkStream::getData()` already returns `sk_sp<const SkData>` directly.
Introducing intermediate helper functions or using `const_cast` to obtain
non-const `SkData` is unnecessary and undermines const-correctness when callers
only require read-only access to the stream's underlying data.

## Examples

Bad:

```cpp
// Calling intermediate const-casting helper when read-only data is sufficient
fCachedData = SkStreamPriv::GetNonConstData(stream.get());
```

Good:

```cpp
// Obtaining read-only sk_sp<const SkData> directly from the stream
fCachedData = stream->getData();
```
