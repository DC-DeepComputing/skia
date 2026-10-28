---
name: skia/build-bazel/keep-harmless-includes-unconditional
description: When configuring Bazel cc_library includes, list header search paths unconditionally instead of wrapping harmless, collision-free directories in platform select() expressions
rubric: design
severity: should_fix
---

**Rule.** Do not use select() to conditionally add include directories in Bazel
cc_library (or skia_cc_library) includes attributes when the directories are
harmless and cause no header collisions or shadowing across other platforms.
List header search directories unconditionally in includes, reserving select()
strictly for directories containing conflicting header filenames that would
break compilation on other targets.

**Why.** Wrapping include search paths in platform-specific select() expressions
adds unnecessary configuration verbosity, complicates build inspection, and
increases maintenance overhead. When header filenames within a directory do not
conflict with or shadow headers used on other platforms or architectures, adding
the directory unconditionally is safe and keeps build targets concise.

## Examples

Bad:

```cpp
cc_library(
    name = "libjpeg_turbo",
    srcs = [...],
    includes = ["src"] + select({
        ":arm_or_arm64": [
            "simd/arm",
        ],
        "//conditions:default": [],
    }),
)
```

Good:

```cpp
cc_library(
    name = "libjpeg_turbo",
    srcs = [...],
    includes = [
        "simd/arm",
        "src",
    ],
)
```
