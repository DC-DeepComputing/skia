---
name: skia/build-gn/do-not-suppress-compiler-warnings-in-maintained-targets
description: When configuring compiler flags in GN or Bazel build targets, do not add -w or /w to disable warnings on maintained or first-party Skia modules
rubric: readability
severity: should_fix
---

**Rule.** Do not add blanket warning suppression flags like `-w` or `/w` (via
`cflags` or `copts`) to maintained Skia modules, tools, or first-party build
targets. Reserve warning suppression exclusively for unmaintained third-party
code; maintained targets must compile cleanly under standard warning levels.

**Why.** Blanket warning suppression (`-w` or `/w`) silences diagnostics that
catch undefined behavior, memory hazards, and cross-platform build breakage.
First-party Skia code and actively maintained modules (such as skcms) are
designed to compile warning-free with strict diagnostics enabled across all
supported toolchains.

## Examples

Bad:

```cpp
target("static_library", "skcms") {
  cflags = []
  if (!is_win || is_clang) {
    cflags += [
      "-w",
      "-std=c11",
    ]
  }
}
```

Good:

```cpp
static_library("skcms") {
  cflags = []
  if (!is_win || is_clang) {
    cflags += [ "-std=c11" ]
  }
}
```
