---
name: skia/delete-obsolete-files-instead-of-export-filegroups
description: When retiring obsolete files, coordinate downstream reference cleanup and delete unused files instead of creating synthetic export filegroups
rubric: readability
severity: should_fix
---

**Rule.** Do not create synthetic build targets or filegroups to preserve
obsolete or empty placeholder files solely to maintain legacy downstream file
lists. Coordinate the removal of downstream references in embedders first, and
delete the unused files and workaround build definitions completely.

**Why.** Preserving empty placeholder files and introducing synthetic build
targets to satisfy legacy downstream configurations accumulates dead code and
maintenance debt. Cleaning up downstream references allows obsolete files and
workaround targets to be eliminated entirely.

## Examples

Bad:

```cpp
# Not used for Bazel builds. This is for exporting to utils.gni:skia_utils_sources.
filegroup(
    name = "empty_files",
    srcs = [
        "SkThreadUtils_pthread.cpp",
        "SkThreadUtils_win.cpp",
    ],
)
```

Good:

```cpp
# Coordinate downstream removal in Chromium/PDFium first,
# then delete the obsolete files and targets completely.
```
