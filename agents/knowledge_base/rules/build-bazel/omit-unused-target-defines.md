---
name: skia/build-bazel/omit-unused-target-defines
description: When configuring Bazel library targets (like skia_cc_library), omit unused preprocessor macro definitions from defines unless required by source files or downstream dependents
rubric: readability
severity: should_fix
---

**Rule.** Do not specify preprocessor macro definitions in `defines` on Bazel
C++ library targets (`skia_cc_library` or `cc_library`) if the library's source
files do not inspect the macro and downstream dependents do not require it. If a
macro is needed exclusively by the library's own implementation, use
`local_defines` instead to avoid transitively propagating flags to consumers.

**Why.** In Bazel, `defines` attributes add compiler preprocessor flags
(`-D...`) to the target and transitively propagate them to all downstream
dependents. Adding uninspected macros bloats compile command lines, risks
masking configuration differences, and can unintentionally leak conditional
compilation flags into consuming compilation units.

## Examples

Bad:

```cpp
skia_cc_library(
    name = "decoder",
    srcs = [":srcs"],
    hdrs = [":hdrs"],
    defines = ["SK_CODEC_DECODES_JPEG_WITH_RUST"],
    deps = [
        "//src/codec:codec_support_priv",
    ],
)
```

Good:

```cpp
skia_cc_library(
    name = "decoder",
    srcs = [":srcs"],
    hdrs = [":hdrs"],
    deps = [
        "//src/codec:codec_support_priv",
    ],
)
```
