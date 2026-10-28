---
name: skia/build-gn/update-dependent-targets-on-subtarget-split
description: When sub-targets or helper libraries are split in GN or Bazel, declare dependencies directly in consuming targets rather than top-level aggregators
rubric: design
severity: blocking
---

**Rule.** When factoring or moving source files out of a base library target
into a separate sub-target or helper target in GN (BUILD.gn) or Bazel
(BUILD.bazel), update all internal dependent targets (such as module test suites
or benchmarks) to explicitly depend on the new sub-target in `deps`. Declare
dependencies directly in the target that consumes them (e.g.,
`modules/<mod>:tests`) rather than in top-level aggregator targets (e.g.,
`//BUILD.gn:tests`), keeping internal module dependencies encapsulated.

**Why.** Extracting utility or helper sources out of a base library removes
those symbols from the primary target, causing dependent targets to fail
compilation or linking unless explicitly updated. Declaring the new dependency
directly in the consuming target preserves modular encapsulation and prevents
leaking internal sub-module dependencies into outer aggregators.

## Examples

Bad:

```cpp
# In top-level //BUILD.gn (aggregating module tests without updating the module target)
test_lib("tests") {
  deps = [
    "modules/skparagraph:utils",
    "modules/skparagraph:tests",
  ]
}

# In modules/skparagraph/BUILD.gn (missing dependency after splitting :utils from :skparagraph)
skia_source_set("tests") {
  sources = skparagraph_tests
  deps = [
    ":skparagraph",
    "../..:skia",
  ]
}
```

Good:

```cpp
# In modules/skparagraph/BUILD.gn (dependency declared directly in the consuming target)
skia_source_set("tests") {
  sources = skparagraph_tests
  deps = [
    ":skparagraph",
    ":utils",
    "../..:skia",
    "../..:test",
  ]
}

# In top-level //BUILD.gn (aggregator depends only on the modular target)
test_lib("tests") {
  deps = [
    "modules/skparagraph:tests",
  ]
}
```
