---
name: skia/rust/declare-cxx-unique-ptr-shim
description: 'When wrapping extern C++ types in cxx::UniquePtr across crates or without bridge function signatures, declare impl UniquePtr<T> {} in #[cxx::bridge] instead of dummy functions'
rubric: readability
severity: should_fix
---

**Rule.** When exposing an extern C++ type in a #[cxx::bridge] module that will
be held in cxx::UniquePtr<T> across crate boundaries or without bridge functions
directly referencing UniquePtr<T>, declare impl UniquePtr<T> {} inside the
bridge module. Do not introduce dummy bridge functions or duplicate extern type
definitions to force CXX to synthesize the UniquePtrTarget trait.

**Why.** CXX only synthesizes the UniquePtrTarget trait implementation for an
extern C++ type if UniquePtr<T> appears in that bridge's function signatures or
an explicit impl UniquePtr<T> {} block is declared. Without the explicit impl
block, downstream crates or Rust code attempting to construct or store
cxx::UniquePtr<T> fail compilation with missing trait errors, and adding dummy
workaround functions adds clutter and unneeded symbol exports.

## Examples

Bad:

```cpp
#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        type SkStreamAdapter;
    }

    // Workaround: dummy function to force CXX to synthesize UniquePtrTarget
    extern "Rust" {
        fn force_unique_ptr_bridge(_: UniquePtr<SkStreamAdapter>);
    }
}
```

Good:

```cpp
#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        type SkStreamAdapter;
    }

    // Explicit shim impl to materialize UniquePtrTarget trait implementation
    impl UniquePtr<SkStreamAdapter> {}
}
```
