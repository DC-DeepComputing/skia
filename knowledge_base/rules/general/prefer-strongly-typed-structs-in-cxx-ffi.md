---
name: skia/prefer-strongly-typed-structs-in-cxx-ffi
description: In Rust/C++ CXX FFI bridges, define dedicated shared structs or fixed-size arrays for multi-field data instead of untyped slices or loose primitive out-parameters
rubric: readability
severity: should_fix
---

**Rule.** In Rust/C++ FFI bindings (such as #[cxx::bridge]), define dedicated
shared structs or fixed-size arrays for structured multi-value data and metadata
instead of passing untyped primitive slices (such as &[f32]) or long lists of
loose primitive out-parameters. Do not decompose composite metadata into
unconstrained float/byte slices or loose parameters that require manual offset
indexing and runtime length validation.

**Why.** Passing structured data as untyped primitive slices or loose primitive
parameters discards semantic field names, forces runtime length checks or manual
indexing, and increases the risk of parameter-ordering bugs. Defining shared
structs in the CXX bridge provides compile-time type safety, clear field names,
and consistent memory layout across both Rust and C++.

## Examples

Bad:

```cpp
#[cxx::bridge(namespace = "rust_png")]
mod ffi {
    extern "Rust" {
        // Untyped &[f32] slice requiring runtime length checks and magic indexing:
        fn new_writer(
            output: UniquePtr<WriteTrait>,
            mdcv: &[f32],
            clli: &[f32],
        ) -> Box<ResultOfWriter>;

        // Loose primitive out-parameters across FFI boundary:
        fn try_get_mdcv_chunk(
            self: &Reader,
            rx: &mut f32, ry: &mut f32, gx: &mut f32, gy: &mut f32,
            bx: &mut f32, by: &mut f32, wx: &mut f32, wy: &mut f32,
            max_luminance: &mut f32, min_luminance: &mut f32,
        ) -> bool;
    }
}
```

Good:

```cpp
#[cxx::bridge(namespace = "rust_png")]
mod ffi {
    struct ContentLightLevelInfo {
        fMaxCLL: f32,
        fMaxFALL: f32,
    }

    struct MasteringDisplayColorVolume {
        fRX: f32, fRY: f32, fGX: f32, fGY: f32,
        fBX: f32, fBY: f32, fWX: f32, fWY: f32,
        fMaxLuminance: f32,
        fMinLuminance: f32,
    }

    extern "Rust" {
        fn try_get_clli_chunk(self: &Reader, clli: &mut ContentLightLevelInfo) -> bool;
        fn try_get_mdcv_chunk(self: &Reader, mdcv: &mut MasteringDisplayColorVolume) -> bool;
    }
}
```
