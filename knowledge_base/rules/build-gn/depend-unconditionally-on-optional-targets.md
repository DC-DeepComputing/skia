---
name: skia/build-gn/depend-unconditionally-on-optional-targets
description: When depending on targets defined with optional(...) in GN build files, list them unconditionally in deps instead of wrapping them in conditional if blocks
rubric: readability
severity: blocking
---

**Rule.** In GN build files (BUILD.gn), add targets defined with the
optional(...) template unconditionally to composite target dependencies (such as
skia_component("skia")'s public_deps or deps). Do not wrap them in conditional
'if' blocks; optional(...) targets automatically evaluate to empty source sets
when disabled.

**Why.** Skia's optional(...) GN template evaluates to an empty skia_source_set
with no sources, dependencies, or defines when invoker.enabled is false. Because
of this, composite targets can depend on optional feature and helper targets
unconditionally across all configurations without redundant conditional
branching.

## Examples

Bad:

```cpp
skia_component("skia") {
  public_deps = [ ":xps" ]
  if (skia_use_libpng_encode) {
    public_deps += [ ":xps_libpng_helpers" ]
  }
}
```

Good:

```cpp
skia_component("skia") {
  public_deps = [
    ":xps",
    ":xps_libpng_helpers",
    ":xps_rust_png_helpers",
  ]
}
```
