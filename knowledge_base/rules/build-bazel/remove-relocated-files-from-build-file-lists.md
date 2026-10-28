---
name: skia/build-bazel/remove-relocated-files-from-build-file-lists
description: When adding, relocating, renaming, or deleting files, update Bazel and GN build manifests with committed, exact-matching file paths and prune stale entries
rubric: correctness
severity: blocking
---

**Rule.** When adding, relocating, renaming, or deleting source and header
files, update the corresponding build manifests in Bazel (`BUILD.bazel` `srcs`,
`hdrs`, or filegroups) and GN (`BUILD.gn` or `gn/*.gni`). Ensure that newly
declared file paths exist in the repository or are included in the changelist,
match the filesystem path on disk exactly, and include companion headers. Prune
stale, relocated, or renamed entries from existing parent or target lists.

**Why.** In Bazel, undeclared headers fail sandboxed compilation, while stale
entries cause dangling-reference errors. In GN, referencing uncommitted files or
using mismatched filenames breaks Ninja builds with missing-target errors and
leaves newly added sources or tests uncompiled.

## Examples

Bad:

```cpp
// modules/canvaskit/BUILD.gn: referencing file not added to git/CL
sources += [ "fonts/no_builtin_fonts.cpp" ]

// gn/gm.gni: adding a GM with mismatched filename
gm_sources = [
  ...
  "$_gm/squarecappedhairlines.cpp", // file created was gm/closedcappedhairlines.cpp
]

// BUILD.bazel: missing companion header or retaining relocated file
GRAPHITE_FILES = [
  "SpecialImage_Graphite.h",
  "StripGenerator.h", // moved to sparse_strips/ but left in parent list
]
```

Good:

```cpp
// modules/canvaskit/BUILD.gn: file added to git/CL with matching path
sources += [ "fonts/no_builtin_fonts.cpp" ] // fonts/no_builtin_fonts.cpp committed

// gn/gm.gni: exact matching filename
gm_sources = [
  ...
  "$_gm/closedcappedhairlines.cpp",
]

// BUILD.bazel: pruning relocated file; companion header included
GRAPHITE_FILES = [
  "SpecialImage_Graphite.h",
]
// src/gpu/graphite/sparse_strips/BUILD.bazel
SPARSE_STRIPS_FILES = [
  "StripGenerator.cpp",
  "StripGenerator.h",
]
```
