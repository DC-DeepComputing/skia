---
name: skia/codec/assert-codec-private-stream-validity
description: In SkCodec subclasses that retain a private stream, assert that the stream member is non-null before dereferencing or duplicating it in getEncodedData
rubric: readability
severity: should_fix
---

**Rule.** In SkCodec subclasses that retain private stream ownership (such as
fPrivStream) instead of passing the stream to the SkCodec base class, assert
that the stream member is non-null using SkASSERT_RELEASE (or SkASSERT) before
accessing or duplicating it in getEncodedData(). Ensure the invariant is also
asserted in constructors when initializing private stream members.

**Why.** SkCodec subclasses such as SkWuffsCodec and Rust-based decoders retain
private stream members to prevent unwanted automatic base-class stream
rewinding. Because getEncodedData() overrides the base class implementation,
asserting the stream invariant prevents null dereferences and guarantees
consistency across all codec backends.

## Examples

Bad:

```cpp
sk_sp<const SkData> SkPngRustCodec::getEncodedData() const {
    // Missing stream invariant assertion before dereferencing or duplicating
    auto dStream = fPrivStream->duplicate();
    return dStream ? dStream->detachAsData() : nullptr;
}
```

Good:

```cpp
sk_sp<const SkData> SkPngRustCodec::getEncodedData() const {
    SkASSERT_RELEASE(fPrivStream);
    sk_sp<const SkData> data = fPrivStream->getData();
    if (data) {
        return data;
    }
    auto dStream = fPrivStream->duplicate();
    return dStream ? dStream->detachAsData() : nullptr;
}
```
