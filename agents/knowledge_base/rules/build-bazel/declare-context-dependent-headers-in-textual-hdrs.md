---
name: skia/build-bazel/declare-context-dependent-headers-in-textual-hdrs
description: When declaring context-dependent or macro-configured headers in Bazel library targets, list them in textual_hdrs rather than hdrs or srcs
rubric: design
severity: should_fix
---

**Rule.** In Bazel C++ targets (such as cc_library or skia_cc_library), declare
headers that vary based on enclosing preprocessor definitions (such as
architecture-specific opts or SIMD headers) in textual_hdrs rather than hdrs or
srcs. When exposing such headers from a package, encapsulate them in a filegroup
dedicated to textual headers.

**Why.** Bazel's layering_check and C++ modules require files in hdrs and srcs
to be self-contained and modular, compiling identically regardless of include
order. Headers that change behavior or definitions based on surrounding
preprocessor macros (like SK_OPTS_TARGET) violate modularity unless declared in
textual_hdrs, which informs the build tool to treat them as raw text inclusions.

## Examples

Bad:

```cpp
# src/opts/BUILD.bazel
skia_filegroup(
    name = "core_priv_hdrs",
    srcs = ["SkBitmapProcState_opts.h"],
)

# src/core/BUILD.bazel
skia_cc_library(
    name = "core",
    srcs = [
        "SkBitmapProcState_opts_ssse3.cpp",  # Defines SK_OPTS_TARGET
        "//src/opts:core_priv_hdrs",
    ],
    features = ["layering_check"],
)
```

Good:

```cpp
# src/opts/BUILD.bazel
skia_filegroup(
    name = "textual_hdrs",
    srcs = ["SkBitmapProcState_opts.h"],
)

# src/core/BUILD.bazel
skia_cc_library(
    name = "core",
    srcs = [
        "SkBitmapProcState_opts_ssse3.cpp",
    ],
    textual_hdrs = [
        "//src/opts:textual_hdrs",
    ],
    features = ["layering_check"],
)
```
