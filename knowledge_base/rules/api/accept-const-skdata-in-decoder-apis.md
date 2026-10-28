---
name: skia/api/accept-const-skdata-in-decoder-apis
description: When accepting, returning, or storing read-only, serialized, or decoded buffer data, use sk_sp<const SkData> instead of mutable sk_sp<SkData>
rubric: readability
severity: should_fix
---

**Rule.** When accepting, returning, or storing read-only, serialized, or
immutable data buffers (such as in decoder `Decode` functions, image/generator
encoded data accessors, or serialized metadata members and parameters), use
`sk_sp<const SkData>` rather than mutable `sk_sp<SkData>`. In image decoder
`Decode` functions (such as `Sk<Format>Decoder::Decode`), avoid introducing or
maintaining signatures that take non-const `sk_sp<SkData>`, except for
deprecated inline backward-compatibility overloads where required for legacy
callers.

**Why.** `SkData` provides a mutable `writable_data()` accessor unless qualified
as `const`. Using `sk_sp<const SkData>` explicitly signals that the underlying
buffer is read-only, prevents accidental mutation across shared references,
avoids unsafe `const_cast`s by callers holding const data, and enforces
const-correctness across Skia's public and internal APIs.

## Examples

Bad:

```cpp
// Bad: accepts or stores mutable sk_sp<SkData> for read-only data
namespace SkBmpRustDecoder {
SK_API std::unique_ptr<SkCodec> Decode(sk_sp<SkData> data,
                                       SkCodec::Result* result = nullptr,
                                       SkCodecs::DecodeContext ctx = nullptr);
}

class SkHdrMetadata {
    sk_sp<SkData> fAgtm;
    sk_sp<SkData> getSerializedAgtm() const;
    void setSerializedAgtm(sk_sp<SkData> agtm);
};
```

Good:

```cpp
// Good: enforces immutability with sk_sp<const SkData>
namespace SkBmpRustDecoder {
SK_API std::unique_ptr<SkCodec> Decode(sk_sp<const SkData> data,
                                       SkCodec::Result* result = nullptr,
                                       SkCodecs::DecodeContext ctx = nullptr);
}

class SkHdrMetadata {
    sk_sp<const SkData> fAgtm;
    sk_sp<const SkData> getSerializedAgtm() const;
    void setSerializedAgtm(sk_sp<const SkData> agtm);
};
```
