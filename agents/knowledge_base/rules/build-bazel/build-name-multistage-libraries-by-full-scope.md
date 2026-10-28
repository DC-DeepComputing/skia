---
name: skia/build-bazel/build-name-multistage-libraries-by-full-scope
description: When defining build targets or libraries that perform rendering or execution alongside parsing, name them after full capabilities rather than parsing alone
rubric: readability
severity: nit
---

**Rule.** Do not name a build target or library after only an initial pipeline
stage (such as `_parser` or `_decode`) when the library also implements
downstream rendering, layout, or execution. Name the target to reflect its full
scope or primary capability (such as `_renderer`, `_player`, or the unqualified
format or module name).

**Why.** Naming a library after only an initial stage obscures its true scope
and transitive dependencies. Clients looking for parsing may unintentionally
link an entire rendering or execution pipeline and its heavy dependencies, while
clients looking for rendering may fail to discover the component.

## Examples

Bad:

```cpp
# modules/svg/BUILD.bazel
skia_cc_library(
    name = "svg_parser",
    srcs = [
        "//modules/svg/src:private_hdrs",
        "//modules/svg/src:srcs",
    ],
    hdrs = ["//modules/svg/include:public_hdrs"],
    deps = [
        "//:core",
        "//:pathops",
        "//modules/skshaper:skshaper_core",
    ],
)
```

Good:

```cpp
# modules/svg/BUILD.bazel
skia_cc_library(
    name = "svg_renderer",
    srcs = [
        "//modules/svg/src:private_hdrs",
        "//modules/svg/src:srcs",
    ],
    hdrs = ["//modules/svg/include:public_hdrs"],
    deps = [
        "//:core",
        "//:pathops",
        "//modules/skshaper:skshaper_core",
    ],
)
```
