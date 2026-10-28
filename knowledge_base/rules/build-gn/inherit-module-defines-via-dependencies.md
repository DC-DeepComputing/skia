---
name: skia/build-gn/inherit-module-defines-via-dependencies
description: When configuring GN build targets that require upstream module flags, depend on the module via deps rather than manually redeclaring its defines
rubric: design
severity: should_fix
---

**Rule.** Do not manually declare preprocessor defines owned or exported by
other modules (such as SK_SHAPER_* or SK_UNICODE_*) in a GN target's defines
list. Instead, add explicit dependencies on those modules in deps (or re-export
via public_deps) so their public_configs propagate the required macro
definitions through the build graph.

**Why.** Manually hardcoding another module's configuration macros bypasses GN's
dependency graph, breaks architectural layering, and tightly couples consumers
to the provider's internal macro naming. Propagating defines via deps and
public_configs ensures compiler flags update automatically whenever module
implementations or configurations change.

## Examples

Bad:

```cpp
if (skia_use_icu && skia_use_harfbuzz) {
  defines += [
    "SK_SHAPER_HARFBUZZ_AVAILABLE",
    "SK_SHAPER_UNICODE_AVAILABLE",
    "SK_UNICODE_ICU_IMPLEMENTATION",
  ]
}
```

Good:

```cpp
if (skia_use_icu && skia_use_harfbuzz) {
  deps += [
    "../../modules/skshaper",
    "../../modules/skunicode",
  ]
}
```
