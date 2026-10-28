---
name: skia/api/avoid-hardcoding-build-flags-in-headers
description: When introducing client-configurable build flags or tracing macros, document them in SkUserConfig.h or build files rather than hardcoding or documenting them in SkTypes.h
rubric: design
severity: should_fix
---

**Rule.** Do not hardcode active `#define` directives for feature flags, build
variants, or tracing macros (such as `SK_TRACE_*` or staging flags) in public or
core headers like `include/core/SkTypes.h`. When introducing client-configurable
build flags, optional compile-time features, or tuning options, document them in
`include/config/SkUserConfig.h` (as commented-out `//#define` directives with
explanatory comments) or configure them via build arguments in `BUILD.gn`/Bazel
rather than placing or documenting them in arbitrary core headers. Ensure any
temporary test `#define`s used during local development are removed before
submission.

**Why.** Skia uses `include/config/SkUserConfig.h` as the centralized catalog
where downstream embedders and client builds discover and configure optional
compile-time defines and limits. Documenting or defining build flags in
arbitrary core headers fractures configuration discoverability, while hardcoding
active `#define`s in public headers unconditionally enables flags across all
consumers and bypasses target build configurations.

## Examples

Bad:

```cpp
// include/core/SkTypes.h
// Hardcoding active opt-in trace macro
#define SK_TRACE_GRAPHITE_PIPELINE_USE

// Documenting client-configurable build flag in a core header
// define SK_USE_DRAWING_MIPMAP_DOWNSAMPLER to use smaller mipmap builder
```

Good:

```cpp
// include/config/SkUserConfig.h
// Document client-facing compile-time options centrally as commented-out defines
// To use smaller but slower mipmap builder
//#define SK_USE_DRAWING_MIPMAP_DOWNSAMPLER

// BUILD.gn
// Configure opt-in tracing in build files rather than public headers
if (!skia_disable_tracing && skia_enable_pipeline_label_tracing) {
    defines += [ "SK_TRACE_GRAPHITE_PIPELINE_USE" ]
}
```
