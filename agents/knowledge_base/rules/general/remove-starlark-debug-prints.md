---
name: skia/remove-starlark-debug-prints
description: When authoring or modifying Bazel macros, rules, and build files (*.bzl, BUILD.bazel), remove temporary debug print statements before submitting
rubric: readability
severity: nit
---

**Rule.** Do not leave temporary debug print statements in Bazel/Starlark
macros, rules, or build files (*.bzl, BUILD.bazel). Remove all ad-hoc print
calls used during local testing or troubleshooting before submitting code for
review.

**Why.** In Bazel and Starlark, print(...) statements execute during the loading
and analysis phases whenever the macro or build target is evaluated. Leftover
print statements pollute terminal logs, CI build logs, and query output with
noisy diagnostic clutter.

## Examples

Bad:

```cpp
def skia_objc_library(name, srcs = [], non_arc_srcs = [], **kwargs):
    if len(non_arc_srcs) > 0:
        print("non_arc_srcs:")
        print(non_arc_srcs)
        objc_library(
            name = name + "_non_arc",
            srcs = non_arc_srcs,
        )
```

Good:

```cpp
def skia_objc_library(name, srcs = [], non_arc_srcs = [], **kwargs):
    if len(non_arc_srcs) > 0:
        objc_library(
            name = name + "_non_arc",
            srcs = non_arc_srcs,
        )
```
