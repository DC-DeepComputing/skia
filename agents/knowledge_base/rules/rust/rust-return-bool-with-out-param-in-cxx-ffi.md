---
name: skia/rust/rust-return-bool-with-out-param-in-cxx-ffi
description: In Rust CXX FFI bridges, keep shared structs and signatures minimal by omitting unconsumed fields or enums and returning bool with an out-parameter
rubric: design
severity: should_fix
---

**Rule.** In Rust CXX FFI bridge declarations (`#[cxx::bridge]`), keep shared
structs and function signatures minimal: do not define or populate unused
fields, enums, or mapping logic (such as unconsumed metadata, speculative
options, or error strings) that C++ callers ignore. For fallible operations,
avoid returning composite result structs that pair a `success: bool` flag with
payload fields that must be dummy-populated on failure; return `bool` indicating
success and accept a mutable reference out-parameter (`out: &mut T`) to populate
only on success.

**Why.** CXX bridges do not support returning Rust `Result` types without C++
exception handling, which Skia disables. Exposing speculative enums,
unpropagated fields, or composite failure-structs forces dummy allocations (like
`Vec` or `String`), adds unnecessary translation overhead across the FFI
boundary, and increases maintenance cost for code that downstream C++ consumers
do not use.

## Examples

Bad:

```cpp
#[cxx::bridge(namespace = "rust_icc")]
mod ffi {
    enum RenderingIntent { Perceptual, RelativeColorimetric }

    struct IccProfileResult {
        success: bool,
        error_message: String,
        rendering_intent: RenderingIntent, // Unused by C++
        profile_bytes: Vec<u8>,
    }

    extern "Rust" {
        // Forces dummy String/Vec allocations on errors and exposes unconsumed fields/enums
        fn parse_icc_profile(data: &[u8]) -> IccProfileResult;
    }
}
```

Good:

```cpp
#[cxx::bridge(namespace = "rust_icc")]
mod ffi {
    struct IccProfile {
        data_color_space: ColorSpace,
        connection_space: ColorSpace,
        to_xyzd50: Matrix3x3,
        // Omitted unused rendering_intent field and RenderingIntent enum
    }

    extern "Rust" {
        // Returns bool and populates destination struct on success
        fn parse_icc_profile(data: &[u8], out: &mut IccProfile) -> bool;
    }
}
```
