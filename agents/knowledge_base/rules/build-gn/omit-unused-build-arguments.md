---
name: skia/build-gn/omit-unused-build-arguments
description: When modifying GN build configurations in feature CLs, omit unused build arguments and leftover experimental flags from declare_args() blocks
rubric: design
severity: should_fix
---

**Rule.** Do not declare unused build arguments or leave temporary experimental
flags in GN declare_args() blocks within feature CLs. Keep build configuration
edits strictly scoped to options actively required and consumed by the feature,
cleaning up prototyping flags prior to landing.

**Why.** Declaring unused build arguments clutters project build configuration,
exposes misleading public configuration options via gn args, and creates dead
maintenance debt for embedders.

## Examples

Bad:

```cpp
# In gn/skia/BUILD.gn or gn/skia.gni for a feature change:
declare_args() {
  extra_cflags = []
  # Leftover prototyping flag not required or used by the feature
  skia_use_cpp20 = false
}
```

Good:

```cpp
# Keep build argument declarations scoped strictly to active feature requirements:
declare_args() {
  extra_cflags = []
}
```
