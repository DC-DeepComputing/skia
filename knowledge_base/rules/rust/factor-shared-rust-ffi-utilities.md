---
name: skia/rust/factor-shared-rust-ffi-utilities
description: When defining shared Rust FFI utilities, adapters, or build targets, place them in rust/common instead of root rust/ or format-specific directories
rubric: design
severity: should_fix
---

**Rule.** Do not place generic Rust FFI utilities (such as slice-to-span
converters, stream adapters, or helper headers) directly in the root rust/
directory or in format-specific directories (such as rust/png/), and do not
grant cross-codec visibility into them. Factor shared FFI helpers into
rust/common/ (e.g., //rust/common:ffi_utils), keeping format-specific crates
decoupled and the root directory clean.

**Why.** Generic Rust FFI utilities are needed across multiple Rust codecs (such
as PNG, BMP, and JPEG). The rust/ root directory is organized into crate
subdirectories, while leaving shared utilities in format-specific folders or
exposing them via cross-codec visibility couples unrelated codecs together,
violates subsystem layering, and complicates modular builds.

## Examples

Bad:

```cpp
// Bad: Placing shared helpers in root rust/ or format-specific directories
// rust/SpanUtils.h
#include <span>

// Or in rust/png/BUILD.bazel:
skia_cc_library(
    name = "utils",
    hdrs = ["UtilsForFFI.h"],
    visibility = ["//experimental/rust_bmp/ffi:__pkg__"],
)
```

Good:

```cpp
// Good: Factoring shared FFI helpers and build targets into rust/common/
// rust/common/SpanUtils.h
#include <span>

// In rust/common/BUILD.bazel:
skia_cc_library(
    name = "ffi_utils",
    hdrs = ["SpanUtils.h", "SkStreamAdapter.h"],
    visibility = ["//:__subpackages__"],
)
```
