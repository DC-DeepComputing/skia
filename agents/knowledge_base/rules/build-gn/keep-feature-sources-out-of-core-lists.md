---
name: skia/build-gn/keep-feature-sources-out-of-core-lists
description: When defining build file lists in GN or Bazel, isolate conditional, legacy, or feature-specific headers and sources in dedicated lists rather than core or public filegroups
rubric: design
severity: should_fix
---

**Rule.** In GN (*.gni) and Bazel (BUILD.bazel) build files, isolate optional,
experimental, legacy, or feature-specific source and header files in dedicated
feature lists (e.g., skia_<feature>_sources in GN or <FEATURE>_FILES using
split_srcs_and_hdrs in Bazel) rather than unconditional core or public lists
(such as skia_core_sources, CORE_FILES, public_hdrs, or core_hdrs).
Specifically, do not include headers in public or core filegroups if their
dependencies (such as legacy or deprecated headers) are excluded from the target
or module. In GN, gate feature source sets behind an explicit build argument in
gn/skia.gni (declare_args() { skia_enable_<feature> = false }) and conditionally
append them with preprocessor defines (SK_ENABLE_<FEATURE>) in BUILD.gn.

**Why.** Placing feature-specific or legacy headers in unconditional core lists
or public filegroups violates build modularity and exposes declarations whose
dependencies or definitions are excluded, causing link failures or broken
standalone header compilation in downstream embedders. In GN, compiling
experimental code unconditionally increases binary size and compile times for
embedders; gating behind dedicated variables and flags keeps core builds lean.

## Examples

Bad:

```cpp
// Bazel (BUILD.bazel): Placing feature or legacy headers in core/public lists
CORE_FILES = [
    ...
    "SkFilterColorProgram.h",
]
skia_filegroup(
    name = "public_hdrs",
    srcs = [
        "SkBlurDrawLooper.h",  # Depends on legacy SkDrawLooper.h, excluded from core
    ],
)

// GN (gn/graphite.gni): Feature header in unconditional core list
skia_graphite_sources = [
    ...
    "$_src/gpu/graphite/sparse_strips/MSAA_LUT.h",
]
```

Good:

```cpp
// Bazel (BUILD.bazel): Headers isolated in feature lists; excluded from core hdrs
CORE_FILES = [
    ...
]
SKSL_FILES = [
    "SkFilterColorProgram.cpp",
    "SkFilterColorProgram.h",
]
skia_filegroup(
    name = "public_hdrs",
    srcs = [
        # SkBlurDrawLooper.h omitted because its legacy dependency is not in core
    ],
)

// GN (gn/graphite.gni and BUILD.gn): Gated in dedicated feature list
skia_graphite_sparse_strips_sources = [
    "$_src/gpu/graphite/sparse_strips/MSAA_LUT.h",
]
if (skia_enable_sparse_strips) {
    sources += skia_graphite_sparse_strips_sources
    public_defines += [ "SK_ENABLE_SPARSE_STRIPS" ]
}
```
