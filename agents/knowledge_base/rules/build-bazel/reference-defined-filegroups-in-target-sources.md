---
name: skia/build-bazel/reference-defined-filegroups-in-target-sources
description: When declaring library targets or composite filegroups in BUILD.bazel, reference defined filegroup labels in srcs or hdrs instead of duplicating individual file paths
rubric: design
severity: should_fix
---

**Rule.** When defining library targets (such as `skia_cc_library`) or composite
filegroups in `BUILD.bazel` files, reference defined `skia_filegroup` (or
`filegroup`) labels in `srcs` or `hdrs` rather than repeating individual file
paths. If a file or subset of files is already defined in another filegroup
within the package, reference that target label (e.g., `":skopts_hdrs"`) in the
broader filegroup or library instead of listing the files redundantly.

**Why.** Duplicating file paths across multiple filegroups or library targets
creates redundant source lists that violate DRY and drift out of sync when files
are renamed or removed. Referencing existing filegroup targets unifies file list
management across local compilation and package-level filegroup compositions.

## Examples

Bad:

```cpp
# Bad: repeating file paths in a library target when filegroups exist
skia_filegroup(
    name = "skbmprustcodec_hdrs",
    srcs = ["SkBmpRustCodec.h"],
)
skia_cc_library(
    name = "skbmprustcodec",
    srcs = ["SkBmpRustCodec.cpp"],
    hdrs = ["SkBmpRustCodec.h"],
)

# Bad: duplicating files across multiple filegroups in the same package
skia_filegroup(
    name = "skopts_hdrs",
    srcs = ["SkSLTraceHook.h"],
)
skia_filegroup(
    name = "private_hdrs",
    srcs = [
        "SkRPDebugTrace.h",
        "SkSLTraceHook.h",
    ],
)
```

Good:

```cpp
# Good: reference existing filegroup labels in library targets
skia_filegroup(
    name = "skbmprustcodec_hdrs",
    srcs = ["SkBmpRustCodec.h"],
)
skia_cc_library(
    name = "skbmprustcodec",
    srcs = [":skbmprustcodec_srcs"],
    hdrs = [":skbmprustcodec_hdrs"],
)

# Good: compose filegroups by referencing the subset filegroup label
skia_filegroup(
    name = "skopts_hdrs",
    srcs = ["SkSLTraceHook.h"],
)
skia_filegroup(
    name = "private_hdrs",
    srcs = [
        "SkRPDebugTrace.h",
        ":skopts_hdrs",
    ],
)
```
