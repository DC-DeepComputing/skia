---
name: skia/gn/avoid-duplicate-sources-in-filegroups
description: When configuring GN targets, source lists, or Bazel cc_libraries, do not compile the same C++ source across multiple targets; depend on shared libraries instead
rubric: design
severity: blocking
---

**Rule.** Do not include the same .cpp or .cc source or test file in multiple
build targets, GN source lists, or Bazel library srcs that link into the same
binary (such as duplicating test files in sources_for_tests when already
compiled by a dependency). In GN, keep file lists disjoint and compose base
source lists with +. In Bazel, do not include .cpp files or filegroups in a
target's srcs if that file is already compiled by a dependency in deps or by
another target linking into the same binary; encapsulate shared C++ sources in a
dedicated skia_cc_library and consume them via deps.

**Why.** Compiling the same translation unit into multiple source sets, static
libraries, or filegroups causes duplicate symbol linker errors, redundant
compilation, and One Definition Rule (ODR) violations—especially when targets
link into the same binary or tests rely on static registration macros like
DEF_TEST. Enforcing single target ownership and depending on shared libraries
via deps ensures disjoint symbols, avoids redundant compilation, and preserves
clean module layering.

## Examples

Bad:

```cpp
# Bazel: Re-listing a source file in srcs already compiled by a dependency in deps
skia_cc_library(
    name = "fontmgr_fontconfig_fontations",
    srcs = ["SkFontMgr_fontconfig.cpp", "SkFontMgr_fontconfig_fontations.cpp"],
    deps = [":fontconfig_support"],  # already compiles SkFontMgr_fontconfig.cpp
)

# Bazel: Compiling shared .cpp filegroup into multiple cc_libraries causes ODR violations
skia_cc_library(
    name = "spirv",
    srcs = ["SkSLSPIRVCodeGenerator.cpp", "//src/utils:shader_utils_srcs"],
)

# GN: Duplicate test in sources_for_tests already built by a dependency
optional("fontmgr_fontconfig") {
  deps = [ ":typeface_freetype" ]
  sources_for_tests = [ "tests/FontScanner_FreeTypeTest.cpp" ]
}
```

Good:

```cpp
# Bazel: Consume shared source via dependency in deps without duplicating in srcs
skia_cc_library(
    name = "fontmgr_fontconfig_fontations",
    srcs = ["SkFontMgr_fontconfig_fontations.cpp"],
    deps = [":fontconfig_support"],
)

# Bazel: Encapsulate shared sources in a dedicated cc_library and depend via deps
skia_cc_library(
    name = "spirv",
    srcs = ["SkSLSPIRVCodeGenerator.cpp"],
    deps = ["//src/utils:shader_utils"],
)

# GN: Single target ownership per test translation unit
optional("fontmgr_fontconfig") {
  deps = [ ":typeface_freetype" ]
  sources_for_tests = [ "tests/FontMgrFontConfigTest.cpp" ]
}
```
