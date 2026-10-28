---
name: skia/build-bazel/isolate-headers-in-dedicated-opts-filegroups
description: When exposing headers or sources to sensitive targets like //src/opts, isolate them in dedicated filegroups rather than expanding visibility on broad directory lists
rubric: design
severity: should_fix
---

**Rule.** When exposing header or source files to packages with strict
compilation constraints or multiple micro-architecture builds (such as
//src/opts), isolate the required files in a dedicated skia_filegroup (e.g.,
:opts_srcs or :skopts_hdrs) with visibility restricted to that package. Do not
grant visibility to broad directory filegroups (such as :private_hdrs or :srcs)
that bundle unrelated files.

**Why.** Targets like //src/opts compile repeatedly across multiple CPU
micro-architectures (such as AVX, Haswell, and NEON). Granting visibility to
broad directory filegroups leaks unneeded headers and transitive dependencies
into these targets, bloating include graphs and substantially increasing
compilation overhead across every architectural variant.

## Examples

Bad:

```cpp
# In src/sksl/tracing/BUILD.bazel:
# Bad: Exposing entire private_hdrs filegroup to //src/opts
skia_filegroup(
    name = "private_hdrs",
    srcs = [
        "SkRPDebugTrace.h",
        "SkSLTraceHook.h",
        "SkVMDebugTrace.h",
    ],
    visibility = [
        "//src/sksl:__pkg__",
        "//src/opts:__pkg__",
    ],
)
```

Good:

```cpp
# In src/sksl/tracing/BUILD.bazel:
# Good: Isolate only the required header in a dedicated opts filegroup
skia_filegroup(
    name = "opts_srcs",
    srcs = [
        "SkSLTraceHook.h",
    ],
    visibility = ["//src/opts:__pkg__"],
)

skia_filegroup(
    name = "private_hdrs",
    srcs = [
        "SkRPDebugTrace.h",
        "SkVMDebugTrace.h",
        ":opts_srcs",
    ],
    visibility = ["//src/sksl:__pkg__"],
)
```
