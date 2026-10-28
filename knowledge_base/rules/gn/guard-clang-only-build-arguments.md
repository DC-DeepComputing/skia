---
name: skia/gn/guard-clang-only-build-arguments
description: When declaring default values for build arguments in GN files, guard Clang-specific features with is_clang instead of setting them unconditionally to true
rubric: correctness
severity: blocking
---

**Rule.** When declaring default values for build arguments in GN files (such as
gn/skia.gni), do not unconditionally set arguments for features, dependencies,
or tools only supported or tested on Clang to true. Guard their defaults with
compiler checks such as is_clang (e.g., skia_use_feature = is_clang or
is_skia_standalone && is_clang) so non-Clang compilers (GCC, MSVC) do not
attempt to build unsupported configurations.

**Why.** Skia supports compiling with non-Clang compilers including GCC and
MSVC, but certain external dependencies or features (such as PartitionAlloc)
lack CI coverage or support on non-Clang toolchains. Enabling them
unconditionally causes build failures on non-Clang CQ bots and downstream
embedders.

## Examples

Bad:

```cpp
declare_args() {
  skia_use_partition_alloc = true
}
```

Good:

```cpp
declare_args() {
  skia_use_partition_alloc = is_clang
}
```
