---
name: skia/avoid-redundant-compiler-warning-pragmas
description: When adding compiler diagnostic pragmas in source files, do not re-enable globally configured flags like -Wall or -Wextra; reserve pragmas for selectively disabled warnings
rubric: readability
severity: should_fix
---

**Rule.** Do not redundantly re-enable warning sets like `-Wall` or `-Wextra`
via `#pragma clang diagnostic` in individual source files, as they are already
globally enabled by Skia build configurations. Reserve per-file diagnostic
pragmas for selectively opting into warnings that are globally disabled (such as
`-Wconversion`) or for targeted local suppressions.

**Why.** Re-enabling globally active flags adds boilerplate without benefit,
whereas opting into specific disabled warnings like `-Wconversion` enables
incremental enforcement that carries over into embedder builds.

## Examples

Bad:

```cpp
#ifdef __clang__
#pragma clang diagnostic error "-Wall"
#pragma clang diagnostic error "-Wextra"
#pragma clang diagnostic error "-Wconversion"
#endif
```

Good:

```cpp
#ifdef __clang__
#pragma clang diagnostic error "-Wconversion"
#endif
```
