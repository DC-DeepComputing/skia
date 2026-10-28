---
name: skia/rust/unify-shared-cxx-extern-types
description: When sharing extern C++ types across Rust CXX bridges, declare them in common bridges and import via type aliases instead of duplicating declarations or using include!()
rubric: design
severity: should_fix
---

**Rule.** When sharing an extern C++ type across multiple Rust crates via CXX
bridges, declare the type, its methods, and UniquePtr implementations once in a
common bridge module (such as rust/common/io_traits_ffi.rs). In dependent
bridges, import the type using a CXX type alias (e.g., type SkStreamAdapter =
skia_rust_common::SkStreamAdapter;) instead of repeating method signatures,
duplicating trait implementations, or using include!() macros.

**Why.** CXX safely unifies extern C++ types across separate crate bridges when
imported via type aliases. Re-declaring extern C++ methods across multiple
crates or including bridge definitions via include!() duplicates boilerplate,
risks signature drift, and causes conflicting or redundant trait
implementations.

## Examples

Bad:

```cpp
// In rust/png/FFI.rs
include!("../common/io_traits_ffi.rs");

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        #[namespace = "rust::stream"]
        type StreamAdapter;
        fn read(self: Pin<&mut StreamAdapter>, buffer: &mut [u8]) -> usize;
        fn seek_from_start(self: Pin<&mut StreamAdapter>, requested_pos: u64, final_pos: &mut u64) -> bool;
    }
}
impl_stream_adapter!(ffi::StreamAdapter);
```

Good:

```cpp
// In shared crate (rust/common/io_traits_ffi.rs):
#[cxx::bridge(namespace = "rust::stream")]
mod ffi {
    unsafe extern "C++" {
        include!("rust/common/SkStreamAdapter.h");
        #[namespace = "rust::stream"]
        type SkStreamAdapter;
        fn read(self: Pin<&mut SkStreamAdapter>, buffer: &mut [u8]) -> usize;
    }
    impl UniquePtr<SkStreamAdapter> {}
}
pub use ffi::SkStreamAdapter;

// In consuming crate (rust/png/FFI.rs):
#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        type SkStreamAdapter = skia_rust_common::SkStreamAdapter;
    }
}
```
