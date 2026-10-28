---
name: skia/rust/panic-on-unexpected-enum-variants
description: When matching crate enums in Rust FFI or decoders, explicitly match variants without wildcard fallbacks, and panic on contract errors or unexpected variants
rubric: readability
severity: should_fix
---

**Rule.** When matching on third-party or crate enums (such as error, format, or
color type enums) in Rust FFI and decoder wrappers, do not use silent wildcard
(`_ =>`) fallback arms that return default metadata or convert errors to generic
input failures. Explicitly enumerate all variants to preserve compiler
exhaustiveness diagnostics when dependencies update, and fail fast with `panic!`
(or `unreachable!`) on unexpected variants or internal contract/parameter errors
(such as `ImageError::Parameter`).

**Why.** Silently defaulting formats or error codes masks decoder
misconfigurations, suppresses compiler exhaustiveness checks when crate enums
change, and misreports internal crate usage bugs as malformed external input
rather than failing fast.

## Examples

Bad:

```cpp
// Silently returning default format on unhandled variant:
let (color, alpha, bpp) = match image_color_type {
    image::ColorType::Rgb8 => (BmpColor::RGB, BmpAlpha::Opaque, 3),
    image::ColorType::Rgba8 => (BmpColor::RGBA, BmpAlpha::Unpremul, 4),
    _ => (BmpColor::RGB, BmpAlpha::Opaque, 3),
};

// Wildcard fallback masking compiler checks and contract errors:
impl From<image::ImageError> for BmpError {
    fn from(err: image::ImageError) -> Self {
        match err {
            image::ImageError::Decoding(_) => BmpError::FormatError,
            image::ImageError::Limits(_) => BmpError::UnsupportedFeature,
            _ => BmpError::InvalidData,
        }
    }
}
```

Good:

```cpp
// Explicitly panic on invariant-violating format variants:
let (color, alpha, bpp) = match image_color_type {
    image::ColorType::Rgb8 => (BmpColor::RGB, BmpAlpha::Opaque, 3),
    image::ColorType::Rgba8 => (BmpColor::RGBA, BmpAlpha::Unpremul, 4),
    _ => panic!("Unexpected color type: {:?}", image_color_type),
};

// Exhaustively match variants and panic on internal contract bugs:
impl From<image::ImageError> for BmpError {
    fn from(err: image::ImageError) -> Self {
        match err {
            image::ImageError::Decoding(_) => BmpError::FormatError,
            image::ImageError::Limits(_) => BmpError::UnsupportedFeature,
            image::ImageError::Parameter(_) => {
                panic!("Internal caller contract bug in crate usage: {:?}", err);
            }
            image::ImageError::IoError(e) => BmpError::from(e),
            image::ImageError::Unsupported(_) => BmpError::UnsupportedFeature,
        }
    }
}
```
