---
name: skia/api/use-primary-extension-for-decoder-id
description: When defining SkCodecs::Decoder registration descriptors, set the format ID to the primary lowercase filename extension rather than implementation-specific prefixes
rubric: readability
severity: should_fix
---

**Rule.** In `SkCodecs::Decoder` definitions, set the decoder `id` field to the
format's primary lowercase filename extension (such as `"ico"`, `"png"`,
`"bmp"`, or `"jpeg"`) rather than prefixing it with implementation details like
`"rust_"`.

**Why.** Per the convention documented in `include/codec/SkCodec.h`, decoder IDs
should be all lowercase letters matching the primary filename extension.
`SkCodecs::Register()` uses this ID to replace existing decoders for that format
in the registration list, and `SkCodecs::HasDecoder()` queries decoders by
format ID; adding backend or language prefixes breaks this replacement mechanism
and consistency across decoders.

## Examples

Bad:

```cpp
inline constexpr SkCodecs::Decoder Decoder() {
    return { "rust_ico", IsIco, Decode };
}
```

Good:

```cpp
inline constexpr SkCodecs::Decoder Decoder() {
    return { "ico", IsIco, Decode };
}
```
