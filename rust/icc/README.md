# Rust ICC profile parser

This directory contains Skia's checked Rust parser for the ICC profile data consumed by image
codecs. It parses untrusted ICC bytes into an owned `IccProfile`, then the C++ bridge exposes the
supported fields as an `skcms_ICCProfile` for Skia's existing color transformations.

## Architecture

```text
Image codec -> ICC bytes -> rust_icc::parse_icc_profile()
                                  |
                         owned IccProfile
                                  |
                        ToSkcmsIccProfile()
                                  |
                         skcms_ICCProfile
                                  |
                    Skia color transformations
```

`FFI.rs` defines the stable CXX data boundary and delegates parsing to the following modules:

- `icc_reader.rs`: checked big-endian reads, profile envelope, and tag table
- `icc_curve.rs`: `XYZ `, `curv`, and `para` data
- `icc_lut.rs`: `mft1`, `mft2`, `mAB `, and `mBA ` transforms
- `icc_parser.rs`: ICC header validation, tag selection, and profile assembly

The reader uses checked ranges and arithmetic and contains no `unsafe` code. Curve tables and CLUT
bytes are copied into Rust-owned vectors. The codec retains the Rust `IccProfile` for as long as
the converted `skcms_ICCProfile` can reference those vectors.

## Profile Support

The parser supports:

- ICC v2.0-v2.4 and v4.0-v4.4 headers with XYZ or Lab profile connection spaces
- RGB colorant matrices and RGB or Gray transfer curves
- CICP metadata on permitted input and display profiles
- A2B0-A2B2 and B2A0-B2A2 priority selection
- Legacy `mft1`/`mft2` and modern `mAB `/`mBA ` LUT elements
- One to four device channels and three-channel profile connection space data

Unknown tags are ignored. The parser does not implement profile serialization, color transforms,
text metadata, or HDR gain-map metadata. Final source-profile usability validation and the skcms
fallback remain in the C++ integration.

As a compatibility exception for deployed malformed profiles, an exactly all-zero version field is
accepted. Other v0.x encodings and versions newer than ICC v4 remain rejected.

Resource limits match the parser contract: input profiles must be smaller than 10 MiB, table TRCs
may contain at most 40,000 entries, and CLUTs may contain at most 10,000,000 elements.

## Building

### Bazel

```bash
bazelisk build //rust/icc:ffi_rs
bazelisk test //rust/icc:ffi_rs_test
```

### gn / ninja

Enabled automatically when Rust codecs are enabled:

```bash
gn gen out/RustEnabled --args='skia_use_rust_icc=true'
autoninja -C out/RustEnabled dm
```

## Testing

### Unit Tests

The Rust tests cover checked parsing, malformed inputs, synthetic LUT variants, and every profile
under `resources/icc_profiles`:

```bash
bazelisk test //rust/icc:ffi_rs_test --test_output=all
```

The C++ tests cover the complete bridge, lifetime-sensitive table and CLUT pointers, parser
equivalence on resources, and fallback behavior:

```bash
gn gen out/Debug --args='skia_use_rust_icc=true'
ninja -C out/Debug dm
out/Debug/dm --src tests --match RustIcc --resourcePath resources
```

## Dependencies

- **cxx**: FFI bridge between Rust and C++
- **skcms**: destination profile representation and color transformations

## References

- ICC Specification: https://www.color.org/specification/ICC.1-2022-05.pdf
- skcms: https://skia.googlesource.com/skcms/
