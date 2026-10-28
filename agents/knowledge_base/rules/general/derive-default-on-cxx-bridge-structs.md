---
name: skia/derive-default-on-cxx-bridge-structs
description: When defining shared structs in CXX bridges or Rust FFI, derive Default and use Type::default() instead of manually zero-initializing fields
rubric: readability
severity: nit
---

**Rule.** Derive `Default` on shared structs (including those within
`#[cxx::bridge]`) and use `Type::default()` instead of manually spelling out
zero or default values for every field at call sites.

**Why.** CXX bridge shared structs support `#[derive(Default)]`. Deriving
`Default` and calling `Type::default()` eliminates repetitive
zero-initialization boilerplate, avoids manual field-by-field typing mistakes,
and ensures newly added fields with default values are automatically initialized
correctly.

## Examples

Bad:

```cpp
Some(ffi::Curve {
    table_entries: table.len() as u32,
    parametric: ffi::TransferFunction {
        g: 0.0,
        a: 0.0,
        b: 0.0,
        c: 0.0,
        d: 0.0,
        e: 0.0,
        f: 0.0,
    },
    table_data: data,
})
```

Good:

```cpp
// Inside #[cxx::bridge]:
#[derive(Clone, Copy, Default)]
struct TransferFunction {
    g: f32,
    a: f32,
    b: f32,
    c: f32,
    d: f32,
    e: f32,
    f: f32,
}

// At call site:
Some(ffi::Curve {
    table_entries: table.len() as u32,
    parametric: ffi::TransferFunction::default(),
    table_data: data,
})
```
