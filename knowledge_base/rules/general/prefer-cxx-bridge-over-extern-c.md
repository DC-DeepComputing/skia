---
name: skia/prefer-cxx-bridge-over-extern-c
description: 'When interfacing Rust with C/C++, use #[cxx::bridge] instead of manual extern "C" blocks, and reuse bridge structs rather than duplicate #[repr(C)] types'
rubric: design
severity: should_fix
---

**Rule.** When binding C or C++ APIs in Rust, expose helper functions through
`#[cxx::bridge]` (using `unsafe extern "C++"` and an accompanying C++ header)
rather than declaring manual `extern "C"` blocks and handwritten `#[repr(C)]`
structs. When structured types are already defined in a `#[cxx::bridge]`, reuse
those bridge struct definitions directly rather than declaring duplicate
`#[repr(C)]` mirror structs. If build-time circular dependencies arise between
the C++ FFI implementation and the generated bridge header, split the C++
function declarations into a separate header-only target.

**Why.** Manual `extern "C"` declarations and duplicated `#[repr(C)]` structs
rely on unchecked developer assumptions, bypass compiler verification against
C/C++ headers, and add redundant type definitions. Using `#[cxx::bridge]`
generates bidirectional compile-time type and ABI validation across the FFI
boundary, ensuring shared structs have matching size, alignment, and standard
layout.

## Examples

Bad:

```cpp
// Duplicate #[repr(C)] mirror struct and manual extern "C" block:
#[repr(C)]
struct SkcmsTransferFunction {
    g: f32, a: f32, b: f32, c: f32, d: f32, e: f32, f: f32,
}

extern "C" {
    fn skcms_ApproximateCurve(
        curve: *const SkcmsCurve,
        approx: *mut SkcmsTransferFunction,
        max_error: *mut f32,
    ) -> bool;
}

#[cxx::bridge(namespace = "rust_icc")]
mod ffi {
    struct TransferFunction {
        g: f32, a: f32, b: f32, c: f32, d: f32, e: f32, f: f32,
    }
}
```

Good:

```cpp
#[cxx::bridge(namespace = "rust_icc")]
mod ffi {
    // Reuse single bridge struct with compile-time layout checks:
    struct TransferFunction {
        g: f32, a: f32, b: f32, c: f32, d: f32, e: f32, f: f32,
    }

    unsafe extern "C++" {
        include!("rust/icc/FFI.h");

        #[cxx_name = "ApproximateCurveWrapper"]
        fn approximate_curve_wrapper(
            table: &[u16],
            out_approx: &mut TransferFunction,
            out_max_error: &mut f32,
        ) -> bool;
    }
}
```
