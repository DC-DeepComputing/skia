---
name: skia/prefer-library-targets-over-exports-files
description: When sharing code across Bazel packages, define and depend on structured library targets instead of exporting raw files with exports_files
rubric: design
severity: should_fix
---

**Rule.** Do not use `exports_files` to share source or header files across
Bazel packages. Encapsulate source files within their owning package by defining
structured library targets (such as `skia_cc_library` or `rust_library`), and
depend on those targets directly via `deps`.

**Why.** Using `exports_files` to expose raw source or header files across
package boundaries bypasses target compilation settings, breaks package
encapsulation, and degrades build graph clarity. Exposing functionality through
library targets preserves compiler flags, enables granular caching, and enforces
clean module layering.

## Examples

Bad:

```cpp
# In rust/common/BUILD.bazel
exports_files(
    [
        "SpanUtils.h",
        "io_traits_ffi.rs",
    ],
    visibility = ["//rust/png:__pkg__"],
)
```

Good:

```cpp
# In rust/common/BUILD.bazel, package files into libraries:
skia_cc_library(
    name = "ffi_utils",
    hdrs = ["SpanUtils.h"],
    visibility = ["//rust/png:__pkg__"],
    deps = ["//:core"],
)

# In rust/png/BUILD.bazel, depend directly on the library target:
skia_cc_library(
    name = "ffi_cpp",
    hdrs = ["FFI.h"],
    deps = ["//rust/common:ffi_utils"],
)
```
