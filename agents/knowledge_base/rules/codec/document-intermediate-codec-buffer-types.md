---
name: skia/codec/document-intermediate-codec-buffer-types
description: When defining intermediate color types or declaring primitive row buffers for codec swizzling and color transformation, document format invariants and element widths
rubric: readability
severity: nit
---

**Rule.** When defining intermediate color types (such as kXformSrcColorType =
kRGBA_8888_SkColorType) for codec swizzling and color transformation, document
the format invariants (e.g., fixed 8-bit depth) justifying that choice without
precision loss. Additionally, when declaring intermediate row buffers typed as
primitive arrays (such as std::unique_ptr<uint32_t[]> fXformBuffer), document
the primitive element width and explicitly link it to the intermediate color
format.

**Why.** Hardcoding intermediate color types like kRGBA_8888_SkColorType assumes
the encoded format does not exceed 8 bits per component; documenting this
invariant clarifies why precision is preserved. Similarly, primitive array types
like uint32_t[] obscure byte width assumptions, risking sizing confusion or
subtle buffer overrun bugs if intermediate color types are refactored.

## Examples

Bad:

```cpp
// Color type to use when creating the swizzler for xform
static constexpr SkColorType kXformSrcColorType = kRGBA_8888_SkColorType;

// Intermediate buffer between swizzler and color transformation.
std::unique_ptr<uint32_t[]> fXformBuffer;
```

Good:

```cpp
// Color type to use when creating the swizzler for xform.
// Note that `bitsPerComponent` is always 8 for all supported BMP images.
static constexpr SkColorType kXformSrcColorType = kRGBA_8888_SkColorType;

// Using 4-bytes-wide `uint32_t` for each pixel, because
// `kXformSrcColorType = kRGBA_8888_SkColorType`.
std::unique_ptr<uint32_t[]> fXformBuffer;
```
