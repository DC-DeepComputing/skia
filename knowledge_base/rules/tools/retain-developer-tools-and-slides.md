---
name: skia/tools/retain-developer-tools-and-slides
description: When refactoring or migrating core APIs, do not delete viewer slides or developer tools as drive-by cleanup; land tool retirements in dedicated changes
rubric: change_scope
severity: should_fix
---

**Rule.** Do not delete developer utilities, viewer slides, or demo harnesses
(such as files under `tools/viewer/`) as drive-by cleanup during an API
refactoring or migration. Keep existing tools and demo slides intact, and land
any necessary tool retirements, migrations, or fixes in dedicated, standalone
changes.

**Why.** Viewer slides and developer tools serve as diagnostic harnesses,
interactive demos, and regression checkers. Removing them as collateral cleanup
during core refactorings removes useful developer utilities and conflates API
migration review with tooling decommissioning.

## Examples

Bad:

```cpp
// In BUILD.gn and tools/viewer/BUILD.bazel:
-    "tools/viewer/PathLerpSlide.cpp",
// Deleting tools/viewer/PathLerpSlide.cpp as collateral cleanup during SkPath refactoring
```

Good:

```cpp
// In BUILD.gn and tools/viewer/BUILD.bazel:
     "tools/viewer/PathLerpSlide.cpp",
// Keep viewer slides and developer tools intact during refactoring; retire tools in dedicated changes
```
