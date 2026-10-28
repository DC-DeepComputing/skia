---
name: skia/rust/reuse-upstream-crates-for-format-decoders
description: When introducing Rust support for image formats or codecs, wrap established upstream crates via FFI instead of writing bespoke decoders in-tree
rubric: design
severity: should_fix
---

**Rule.** When introducing Rust support for image formats or media codecs, reuse
established third-party crates (such as @crates//:image, @crates//:png, or
@crates//:zune-jpeg) via thin FFI wrappers and contribute missing functionality
upstream, rather than authoring and maintaining bespoke in-tree decoder
implementations from scratch.

**Why.** Writing and maintaining custom image format decoders and parsers
in-tree incurs significant long-term maintenance, security review, and fuzzing
overhead. Reusing community crates amortizes maintenance across the open-source
ecosystem, keeps Skia's codebase focused on thin FFI bridging, and allows
feature gaps (such as streaming decode or color profile parsing) to be
contributed upstream or bridged incrementally.

## Examples

Bad:

```cpp
// Implementing a bespoke format decoder and parser from scratch in-tree:
// experimental/rust_bmp/ffi/bmp_decoder.rs
// experimental/rust_bmp/ffi/bmp_header.rs
struct BmpHeader {
    width: u32,
    height: u32,
    bpp: u16,
}
impl BmpHeader {
    fn parse(stream: &mut [u8]) -> Result<Self, DecodeError> { /* ... */ }
}
```

Good:

```cpp
// Wrapping an established upstream crate (@crates//:image) via an FFI bridge:
// rust/bmp/FFI.rs
use image::codecs::bmp::BmpDecoder;

pub struct RustBmpDecoder<'a> {
    decoder: BmpDecoder<SkStreamReader<'a>>,
}
```
