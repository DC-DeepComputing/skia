---
name: skia/rust/validate-cxx-enum-discriminants
description: When mirroring an existing C++ enum in a Rust cxx::bridge, declare it as an extern enum with explicit discriminants to enforce compile-time validation
rubric: correctness
severity: should_fix
---

**Rule.** When bridging an enum between Rust and C++ using cxx::bridge to mirror
an existing C++ enum, declare it as an extern enum by importing the type under
extern "C++" and assign explicit matching discriminant values in the Rust enum
declaration. Do not declare a freestanding Rust bridge enum with default or
sequential integer discriminants when binding to existing C++ enum constants.

**Why.** Declaring an independent Rust enum in cxx::bridge generates a separate
C++ enum and omits compile-time cross-language validation. Binding the Rust enum
to the C++ type via an extern "C++" type declaration prompts CXX to generate
static_assert statements verifying that every variant discriminant matches the
C++ value, preventing silent data corruption from mismatched values or layout
drift.

## Examples

Bad:

```cpp
#[cxx::bridge(namespace = "rust_icc")]
mod ffi {
    #[derive(Debug, Clone, Copy, PartialEq, Eq)]
    enum ColorSpace {
        XYZ = 1,
        YCbCr = 4,
        RGB = 6,
    }
}
```

Good:

```cpp
#[cxx::bridge(namespace = "rust_icc")]
mod ffi {
    #[repr(u32)]
    #[derive(Debug, Clone, Copy, PartialEq, Eq)]
    enum skcms_Signature {
        skcms_Signature_RGB   = 0x52474220,
        skcms_Signature_YCbCr = 0x59436272,
        skcms_Signature_XYZ   = 0x58595A20,
    }

    unsafe extern "C++" {
        include!("modules/skcms/skcms.h");

        #[namespace = ""]
        type skcms_Signature;
    }
}
```
