---
name: skia/document-filegroup-dependencies
description: When declaring dependencies or SDK frameworks on aggregate Bazel targets on behalf of filegroup sources, add comments identifying which source files require them
rubric: readability
severity: nit
---

**Rule.** When declaring library dependencies or platform SDK frameworks (such
as `sdk_frameworks` or `deps`) on an aggregate target on behalf of source files
imported via filegroups, add an explanatory comment specifying which source file
requires the dependency. Explicitly note that the source file is included as a
filegroup to explain why the dependency is injected at the target level.

**Why.** Bazel `filegroup` rules group source files without tracking or
propagating their individual compilation and linking dependencies. When an
aggregate target injects SDK frameworks or dependencies on behalf of grouped
source files, documenting which source file needs the dependency prevents
confusion and avoids accidental removal of seemingly unused dependencies.

## Examples

Bad:

```cpp
skia_objc_library(
    name = "ganesh_gl_utils_frameworks",
    sdk_frameworks = ["OpenGL"],
)
```

Good:

```cpp
# OpenGL needed for CreatePlatformGLTestContext_mac.cpp (which is included as a filegroup).
skia_objc_library(
    name = "ganesh_gl_utils_frameworks",
    sdk_frameworks = ["OpenGL"],
)
```
