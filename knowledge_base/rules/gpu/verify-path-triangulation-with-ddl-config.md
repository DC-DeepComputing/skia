---
name: skia/gpu/verify-path-triangulation-with-ddl-config
description: When modifying path rendering or triangulation algorithms in Ganesh (such as GrTriangulator), verify test coverage and GMs using dm with deferred display lists (--config glddl)
rubric: testing
severity: blocking
---

**Rule.** When modifying path rendering, contour tessellation, or triangulation
algorithms (such as in GrTriangulator), verify GMs and test coverage using dm
with deferred display lists enabled (dm --config glddl). Ensure that path
chopping and tiling performed during DDL recording do not expose numerical
precision bugs, cracks, or triangulation failures.

**Why.** Deferred display list (DDL) recording chops paths into tiles before
rendering, exercising geometric boundary and triangulation edge cases that
direct rendering modes do not trigger. Because the interactive Viewer cannot run
with DDLs enabled, running dm with --config glddl is necessary to catch
DDL-specific path chopping regressions locally.

## Examples

Bad:

```cpp
# Relying solely on default direct rendering configs or Viewer to test triangulation changes:
out/Debug/dm --config gl --match Triangulat
out/Debug/viewer --slide GM_path_triangulation
# Misses geometry edge cases exposed by DDL path chopping and tiling
```

Good:

```cpp
# Verify triangulation and path rendering changes under deferred display list tiling:
out/Debug/dm --config glddl --match Triangulat
# Verifies path chopping and tile-boundary geometry under DDL recording
```
