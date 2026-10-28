---
name: skia/tests/avoid-tautological-constant-tests
description: When writing unit tests for constants or format definitions, avoid tautological assertions that only check constants against their defined literal values
rubric: readability
severity: should_fix
---

**Rule.** Do not author unit tests that merely assert a named constant equals
the exact literal value it was initialized with. Reserve constant-related test
assertions for verifying non-trivial relationships across multiple constants,
specification-mandated format signatures, multi-byte layouts, or calculated
derivations.

**Why.** Tests asserting that a constant equals its own literal definition pass
tautologically by construction and provide no regression protection or
functional coverage. They add maintenance overhead, clutter test suites, and
create a false impression of meaningful verification.

## Examples

Bad:

```cpp
// Rust:
#[test]
fn test_icc_profile_constants() {
    assert_eq!(BITMAPV5HEADER_SIZE, 124);
}
#[test]
fn test_pixel_format_constants() {
    assert_eq!(BYTES_PER_PIXEL_RGB, 3);
}

// C++:
DEF_TEST(PixelFormatConstants, reporter) {
    REPORTER_ASSERT(reporter, kBytesPerPixelRGB == 3);
}
```

Good:

```cpp
// Rust: verify multi-byte wire format signature or endian layout
#[test]
fn test_bmp_signature() {
    assert_eq!(BMP_SIGNATURE[0], b'B');
    assert_eq!(BMP_SIGNATURE[1], b'M');
    assert_eq!(BMP_SIGNATURE_U16, 0x4D42);
}

// C++: verify cross-constant invariant or layout relationship
DEF_TEST(HeaderSizeValid, reporter) {
    REPORTER_ASSERT(reporter, kHeaderSize >= sizeof(HeaderPrefix));
}
```
