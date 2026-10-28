---
name: skia/build-bazel/keep-package-scoped-filegroups-minimal
description: When defining package-scoped filegroups in BUILD.bazel, include only the specific headers or sources required by the consuming package rather than bundling extraneous files
rubric: design
severity: should_fix
---

**Rule.** When defining package-scoped filegroups (skia_filegroup or filegroup)
in BUILD.bazel to share files across packages, keep srcs strictly minimal.
Include only the specific headers or source files required by the consuming
package, rather than bundling extraneous headers or sub-filegroups.

**Why.** Exporting unnecessary headers or composite filegroups across package
boundaries leaks internal implementation details, creates spurious build
dependencies, and triggers unnecessary rebuilds when unrelated internal files
change.

## Examples

Bad:

```cpp
# Bad: skopts_hdrs bundles internal enabled_hdrs not needed by //src/opts
skia_filegroup(
    name = "skopts_hdrs",
    srcs = [
        "SkSLTraceHook.h",
        ":enabled_hdrs",
    ],
    visibility = ["//src/opts:__pkg__"],
)
```

Good:

```cpp
# Good: skopts_hdrs includes only the header required by //src/opts
skia_filegroup(
    name = "skopts_hdrs",
    srcs = [
        "SkSLTraceHook.h",
    ],
    visibility = ["//src/opts:__pkg__"],
)
```
