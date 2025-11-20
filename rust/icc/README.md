# ICC Profile Parsing using Rust/moxcms

This directory contains a Rust-based ICC profile parser for Skia codecs.
It uses the `moxcms` Rust crate to validate and parse ICC (International Color
Consortium) color profiles embedded in image files.

The parsed profile data is returned to C++ where Skia's skcms performs color
transformations.

## Architecture

```
Image Codec (PNG/BMP) → ICC bytes → rust_icc::parse_icc_profile()
                                          ↓
                                   IccProfileResult
                                          ↓
                           C++ (SkEncodedInfo + skcms transforms)
```

### Separation of Concerns

- **Parsing (This module)**: Validates and parses ICC profile structure using Rust/moxcms
- **Transformation (skcms)**: Performs actual pixel color space conversions in C++

This separation allows:
- Type-safe ICC parsing in Rust
- Reusing Skia's optimized color transformation pipeline
- Supporting non-sRGB target displays (calibrated monitors, HDR, etc.)

## Building

### Bazel

To build the code from this directory from Skia:

```
$ cd skia-repo-root
$ bazelisk build //rust/icc:ffi_rs
$ bazelisk test //rust/icc:ffi_rs_test
```

### gn / ninja

The module is conditionally enabled when rust_png or rust_bmp codecs are enabled:

```
$ gn args out/RustEnabled
# Set: skia_use_rust_png_decode = true
$ gn gen out/RustEnabled
$ autoninja -C out/RustEnabled
```

## Testing

### Unit Tests (Rust)

```bash
$ bazelisk test //rust/icc:ffi_rs_test
```

### Integration Tests (C++)

The module is tested indirectly through codec tests when ICC profiles are present
in PNG or BMP images.

## Usage Example

From a Skia codec (e.g., SkPngRustCodec):

```cpp
#include "rust/icc/FFI.rs.h"

// Get ICC profile bytes from image decoder
rust::Slice<const uint8_t> iccBytes = reader->icc_profile();
if (!iccBytes.empty()) {
    rust_icc::IccProfileResult result = rust_icc::parse_icc_profile(iccBytes);
    if (result.success) {
        // Use result.profile_bytes with skcms
        sk_sp<SkData> iccData = SkData::MakeWithCopy(
            result.profile_bytes.data(),
            result.profile_bytes.size()
        );
        // Pass to SkEncodedInfo::Make(...)
    }
}
```

## Dependencies

- **cxx**: FFI bridge between Rust and C++
- **moxcms**: ICC profile parser (https://github.com/aDotInTheVoid/moxcms)
- **rust/common**: Shared FFI utilities

## References

- ICC Specification: https://www.color.org/specification/ICC.1-2022-05.pdf
- skcms: https://skia.googlesource.com/skcms/
- CXX: https://cxx.rs/

