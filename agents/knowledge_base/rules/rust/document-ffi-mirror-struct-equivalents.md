---
name: skia/rust/document-ffi-mirror-struct-equivalents
description: When defining or mirroring C/C++ structs in Rust cxx::bridge modules, document their equivalent types, canonical header, drift guardrails, and binding TODOs
rubric: readability
severity: should_fix
---

**Rule.** When defining mirror or bridging structs in Rust `cxx::bridge`
modules, document their equivalent C/C++ types (e.g., `/// FFI/layering-friendly
equivalent of \`SkFoo\` from C/C++.`) and whether the struct adapts or
duplicates a type for layering or ABI isolation. Additionally, when manually
redefining foreign C/C++ structs that must be kept in sync, document the
canonical C/C++ header, explain the guardrails used to detect drift (such as
`static_assert` layout checks or round-trip tests) and their limits, and add a
`TODO` tracking automated binding generation (e.g., `bindgen`).

**Why.** Foreign C/C++ struct redefinitions across FFI boundaries are
susceptible to silent layout or field-order desynchronization when upstream
headers change. Documenting counterpart types, canonical headers, and the
limitations of drift guardrails (layout compatibility vs. field ordering)
clarifies design intent, aids traceability, and tracks future migration to
automated bindings.

## Examples

Bad:

```cpp
#[cxx::bridge(namespace = "rust_ffi")]
mod ffi {
    // Missing doc comment specifying equivalent C++ type:
    struct ColorSpacePrimaries {
        fRX: f32,
        fRY: f32,
    }

    // Redefines foreign C struct without canonical header, drift guardrails, or TODO:
    /// 3x3 matrix for color space transforms (matches skcms_Matrix3x3).
    struct Matrix3x3 {
        vals: [[f32; 3]; 3],
    }
}
```

Good:

```cpp
#[cxx::bridge(namespace = "rust_ffi")]
mod ffi {
    /// FFI/layering-friendly equivalent of `SkColorSpacePrimaries` from C/C++.
    struct ColorSpacePrimaries {
        fRX: f32,
        fRY: f32,
    }

    // Re-defines C structs from `skcms_public.h` kept in sync manually. Guardrails:
    // * static_asserts in C++ verify std::is_layout_compatible (doesn't catch swapped fields of same type)
    // * Round-trip tests verify order of known fields
    // TODO(crbug.com/...): Use bindgen when supported by all clients to avoid manual redefinition.
    /// 3x3 matrix for color space transforms (matches skcms_Matrix3x3).
    struct Matrix3x3 {
        vals: [[f32; 3]; 3],
    }
}
```
