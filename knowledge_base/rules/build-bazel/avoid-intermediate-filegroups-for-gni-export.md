---
name: skia/build-bazel/avoid-intermediate-filegroups-for-gni-export
description: When defining, splitting, or reviewing skia_filegroup targets in BUILD.bazel, use literal file paths, register exported filegroups in bazel/exporter_tool, and verify mappings before deleting unreferenced targets
rubric: correctness
severity: should_fix
---

**Rule.** When defining or modifying skia_filegroup (or filegroup) targets in
BUILD.bazel, populate srcs with literal file paths only and avoid creating
duplicate or composite filegroups solely for GN export; configure
bazel/exporter_tool/main.go to consume private leaf filegroups directly. When
creating or splitting filegroups containing sources or headers needed in GN
builds, register the filegroup in bazel/exporter_tool/main.go and regenerate GN
file lists via make -C bazel generate_gni. Additionally, do not assume or flag
unreferenced filegroups in BUILD.bazel as dead code without verifying whether
bazel/exporter_tool/main.go exports them to GN (.gni) files.

**Why.** The Skia GNI exporter tool (bazel/exporter_tool) ignores Bazel
visibility rules and maps leaf filegroups directly into GN variables, making
duplicate intermediate filegroups redundant and nested target references in srcs
unresolvable. Omitting new or split filegroups from exporter_tool drops files
from GN builds, while removing filegroups that appear unused in Bazel breaks GN
file list generation for downstream embedders.

## Examples

Bad:

```cpp
# Bad: nested target labels in srcs, or duplicate filegroups for export
skia_filegroup(
    name = "srcs",
    srcs = [
        "SkFoo.cpp",
        "//include:hdrs",  # Nested targets unresolvable by exporter
    ],
)

# Bad: creating or splitting an exported filegroup without registering it
# in bazel/exporter_tool/main.go or regenerating GNI files:
skia_filegroup(
    name = "skopts_hdrs",
    srcs = ["SkSLTraceHook.h"],
)
# (omitted from bazel/exporter_tool/main.go and gn/*.gni)
```

Good:

```cpp
# Good: canonical leaf filegroup with literal file paths only
skia_filegroup(
    name = "skopts_hdrs",
    srcs = ["SkSLTraceHook.h"],
    visibility = ["//src/opts:__pkg__"],
)

# In bazel/exporter_tool/main.go, register new or leaf filegroups directly:
{Var: "skia_sksl_core_sources",
    Rules: []string{
        "//src/sksl/tracing:skopts_hdrs",
        "//src/sksl/tracing:srcs",
    },
}
# Then regenerate GN file lists: `make -C bazel generate_gni`
```
