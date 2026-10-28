---
name: skia/bazel/forward-macro-target-attributes
description: When defining Bazel wrapper macros that instantiate underlying library targets, ensure all accepted configuration and dependency attributes like sdk_frameworks are forwarded to each underlying target
rubric: correctness
severity: should_fix
---

**Rule.** When defining Starlark/Bazel wrapper macros that generate underlying
library targets (such as `skia_objc_library_with_testutil`), forward all
accepted configuration and dependency attributes (such as `sdk_frameworks`,
`defines`, `copts`, and `deps`) to each generated target definition. Do not drop
accepted parameter attributes when constructing the underlying rule or macro
invocations.

**Why.** Omitting accepted configuration or dependency parameters when
generating underlying targets silently drops required dependencies—such as Apple
platform frameworks (e.g., Foundation, Metal)—preventing them from reaching the
compiler or linker and breaking hermetic builds.

## Examples

Bad:

```cpp
def skia_objc_library_with_testutil(name, sdk_frameworks = [], deps = [], **kwargs):
    skia_objc_library(
        name = name,
        deps = deps,
        # sdk_frameworks dropped
    )
    skia_objc_library(
        name = name + "_testutil",
        deps = deps,
        # sdk_frameworks dropped
    )
```

Good:

```cpp
def skia_objc_library_with_testutil(name, sdk_frameworks = [], deps = [], **kwargs):
    skia_objc_library(
        name = name,
        deps = deps,
        sdk_frameworks = sdk_frameworks,
    )
    skia_objc_library(
        name = name + "_testutil",
        deps = deps,
        sdk_frameworks = sdk_frameworks,
    )
```
