---
name: skia/rust/name-rust-library-target-after-crate
description: When defining internal Rust crates and Bazel rust_library targets, prefix crate names with a project identifier like skia_rust_ and match target names
rubric: readability
severity: nit
---

**Rule.** Prefix internal Rust crate names with a project identifier (such as
skia_rust_ or skia_, e.g., skia_rust_common) rather than generic names like
rust_common to prevent crate collisions in downstream embedders. In Bazel
rust_library declarations, set the target name to match the prefixed crate name
rather than using generic labels like ffi or ffi_utils, omitting redundant
crate_name attributes when the target name already matches.

**Why.** Rust crate names reside in a shared namespace across crates and
downstream embedders (such as Chromium), where generic crate names risk
colliding with external dependencies. Naming the Bazel target after the prefixed
crate name avoids ambiguous target labels in mixed C++/Rust packages, aligns
with rules_rust conventions, and makes build dependencies self-documenting.

## Examples

Bad:

```cpp
# In rust/common/BUILD.bazel:
rust_library(
    name = "ffi",
    srcs = ["io_traits_ffi.rs"],
    crate_name = "rust_common",
    crate_root = "io_traits_ffi.rs",
)

// In rust/png/FFI.rs:
type SkStreamAdapter = rust_common::SkStreamAdapter;
```

Good:

```cpp
# In rust/common/BUILD.bazel:
rust_library(
    name = "skia_rust_common",
    srcs = ["io_traits_ffi.rs"],
    crate_root = "io_traits_ffi.rs",
)

// In rust/png/FFI.rs:
type SkStreamAdapter = skia_rust_common::SkStreamAdapter;
```
