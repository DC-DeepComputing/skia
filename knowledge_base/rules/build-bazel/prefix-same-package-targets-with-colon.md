---
name: skia/build-bazel/prefix-same-package-targets-with-colon
description: When referencing rule targets in the same Bazel package, prefix the target name with a leading colon to distinguish it from source files
rubric: readability
severity: should_fix
---

**Rule.** When referencing rule targets (such as filegroups, libraries, or
macros) defined within the same Bazel package, prefix the target label with a
leading colon (e.g., ":target_name"). Do not omit the colon on package-local
target references, and reserve unadorned relative paths for direct source files.

**Why.** In Bazel, labels without a leading colon or slash denote source files
relative to the current package. Referencing rule targets without a leading
colon violates Bazel and Buildifier conventions, obscures whether an attribute
refers to a source file or a rule target, and can lead to build errors if Bazel
interprets the label as a file input.

## Examples

Bad:

```cpp
rust_cxx_bridge(
    name = "cxx_bridge",
    src = "rs_srcs",
)

skia_cc_library(
    name = "utils",
    deps = ["base_helpers"],
)
```

Good:

```cpp
rust_cxx_bridge(
    name = "cxx_bridge",
    src = ":rs_srcs",
)

skia_cc_library(
    name = "utils",
    deps = [":base_helpers"],
)
```
