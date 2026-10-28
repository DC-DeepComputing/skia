---
name: skia/distinguish-skslc-from-sksl-targets
description: When naming build targets, file lists, or variables in build files, reserve skslc for standalone compiler dependencies rather than general sksl
rubric: readability
severity: should_fix
---

**Rule.** Distinguish between the standalone SkSL compiler (`skslc`) and general
SkSL features (`sksl`) when naming build targets, file lists, and variables in
build configurations (`BUILD.bazel`, `BUILD.gn`, `*.gni`). Do not label source
lists, filegroups, or targets with `skslc` (or `_skslc`) unless they are direct
dependencies of the standalone `skslc` tool (exported to `skslc_deps`); use
`sksl` for sources only required by Skia's internal SkSL integration.

**Why.** `skslc` refers specifically to the standalone SkSL command-line
compiler executable and its dedicated minimal dependency set (`skslc_deps`).
Naming internal SkSL files or conditional targets `skslc` misleadingly implies
they belong to or are exported to the standalone compiler tool, creating
confusion during build configuration audits and risking unintended dependencies
in the standalone compiler binary.

## Examples

Bad:

```cpp
# In separate group for exporting to sksl.gni:skslc_deps.
SRCS_SKSLC_FILES = [
    "SkShaderUtils.cpp",
    "SkShaderUtils.h",
]

split_srcs_and_hdrs(
    name = "srcs_skslc",
    files = SRCS_SKSLC_FILES,
)
```

Good:

```cpp
# In separate group to avoid exporting to a *.gni file.
SKSL_FILES = [
    "SkShaderUtils.cpp",
    "SkShaderUtils.h",
]

split_srcs_and_hdrs(
    name = "sksl",
    files = SKSL_FILES,
)
```
